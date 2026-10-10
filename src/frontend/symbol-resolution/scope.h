#pragma once
#include "../ast/ast.h"
#include "../diagnostics/diagnostic_listener.h"
#include "../lexer/token.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringMap.h"
#include <cassert>
#include <deque>
#include <functional>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>
enum class SymbolKind {
    Local,
    Function,
    Parameter,
    LoopVariable,
    Builtin,
    Struct,
    Field
};

struct Symbol {
    SymbolKind kind;
    std::string name; // identifier used to look up the symbol
    SourceLocation location;
    const Node *declaration_node;
    bool is_mutable{false};
};

// Called with a scope's depth and symbols just before it is popped. Lets a
// debug tool show what each scope held, since the scope is gone afterwards.
using ScopeExitHook =
    std::function<void(std::size_t, const llvm::StringMap<const Symbol *> &)>;

class ScopeStack {
  public:
    ScopeStack() {
        scopes_.emplace_back(); // global/program scope always exists initially
        declare(Symbol{SymbolKind::Builtin, "print", {0, 0}, nullptr});
    }

    void enter_scope() {
        scopes_.emplace_back();
    }

    void exit_scope() {
        assert(scopes_.size() > 1 && "attempted to exit global scope");
        if (scope_exit_hook_) {
            scope_exit_hook_(scopes_.size() - 1, scopes_.back());
        }
        scopes_.pop_back();
    }

    void set_scope_exit_hook(ScopeExitHook hook) {
        scope_exit_hook_ = std::move(hook);
    }

    // Declares a symbol in the current (innermost) scope.
    // Returns true if insertion succeeded, false if a symbol with the same name
    // already exists in the current scope.
    bool declare(const Symbol &sym) {
        auto [slot, inserted] = scopes_.back().try_emplace(sym.name, nullptr);
        if (!inserted) {
            return false;
        }
        symbol_pool_.emplace_back(sym);
        slot->second = &symbol_pool_.back();
        return true;
    }

    bool declare(const std::string &name, SymbolKind kind, SourceLocation loc,
                 const Node *decl, bool is_mutable = false) {
        return declare(Symbol{kind, name, loc, decl, is_mutable});
    }

    // Looks up a symbol by name, starting from the innermost scope and walking
    // outward.
    [[nodiscard]] const Symbol *lookup(const std::string &name) const {
        for (const auto &scope : std::views::reverse(scopes_)) {
            auto const found = scope.find(name);
            if (found != scope.end()) {
                return found->second;
            }
        }
        return nullptr;
    }

    // Looks up a symbol only in the current (innermost) scope without checking
    // outer scopes.
    [[nodiscard]] const Symbol *lookup_current(const std::string &name) const {
        auto const found = scopes_.back().find(name);
        return found != scopes_.back().end() ? found->second : nullptr;
    }

    // Checks whether declaring `name` in an inner scope would illegally shadow
    // a top-level declaration (function/struct) or built-in (`print`).
    // Returns the conflicting symbol if illegal, or nullptr if allowed.
    [[nodiscard]] const Symbol *
    check_illegal_shadow(const std::string &name) const {
        if (scopes_.size() <= 1) {
            return nullptr;
        }
        auto const found = scopes_.front().find(name);
        if (found != scopes_.front().end()) {
            SymbolKind const k = found->second->kind;
            if (k == SymbolKind::Function || k == SymbolKind::Struct ||
                k == SymbolKind::Builtin) {
                return found->second;
            }
        }
        return nullptr;
    }

    // Read-only view of the scopes, outermost (global) first. Used by the
    // debug printer.
    [[nodiscard]] const std::vector<llvm::StringMap<const Symbol *>> &
    scopes() const {
        return scopes_;
    }

  private:
    // StringMap stores pointers to Symbols that live stably in symbol_pool_
    std::vector<llvm::StringMap<const Symbol *>> scopes_;
    // Stable symbol storage so pointers remain valid after exiting scopes
    std::deque<Symbol> symbol_pool_;
    ScopeExitHook scope_exit_hook_;
};

// Resolves symbols in the AST using a stack of scopes.
class ScopeResolver {
  public:
    // creating a listener to report errors and warning during sym res
    explicit ScopeResolver(DiagnosticListener *listener) : listener_(listener) {
    }

    void resolve(const Program &program);

    [[nodiscard]] bool had_errors() const {
        return error_count_ > 0;
    }

    [[nodiscard("get symbol for node wasnt called")]] const Symbol *
    get_symbol_for_node(const Node *node) const {
        auto const found = node_to_symbol_map_.find(node);
        return found != node_to_symbol_map_.end() ? found->second : nullptr;
    }

    void insert_type(const Type *type, const Symbol *symbol) {
        node_to_symbol_map_.insert({type, symbol});
    }

    void insert_variable(const Expr *expr, const Symbol *symbol) {
        node_to_symbol_map_.insert({expr, symbol});
    }

    [[nodiscard]] const ScopeStack &scope_stack() const {
        return scope_stack_;
    }

    // debug aid: see ScopeExitHook
    void set_scope_exit_hook(ScopeExitHook hook) {
        scope_stack_.set_scope_exit_hook(std::move(hook));
    }

  private:
    void resolve_assignment_stmt(const AssignmentStatement &assignment_stmt);
    void resolve_ret_stmt(const RetStmt &ret_stmt);
    void resolve_expr_stmt(const ExprStmt &expr_stmt);
    void resolve_break_stmt(const BreakStmt &break_stmt);
    void resolve_continue_stmt(const ContinueStmt &continue_stmt);
    void resolve_if_stmt(const IfStmt &if_stmt);
    void resolve_while_stmt(const WhileStmt &while_stmt);
    void resolve_for_stmt(const ForStmt &for_stmt);
    void resolve_var_decl(const VarDeclStmt &var_decl);
    void resolve_stmt(const Stmt &stmt);

    void declare_top_level(const Decl &decl);
    void check_main();
    void resolve_decl(const Decl &decl);

    void resolve_expr(const Expr &expr);
    void resolve_identifier(const Identifier &identifier, bool is_callee);
    void resolve_type(const Type &type);
    void resolve_block(const BlockStmt &block);

    void error(const Node *node, const std::string &message);
    void error_at(SourceLocation loc, const std::string &message);
    void note(SourceLocation loc, const std::string &message);
    llvm::DenseMap<const Node *, const Symbol *> node_to_symbol_map_;
    ScopeStack scope_stack_;
    DiagnosticListener *listener_;
    std::size_t error_count_{0};
    // how many loops enclose the statement being resolved, for break/continue
    std::size_t loop_depth_{0};
};
