#pragma once
#include "../ast/ast.h"
#include "../diagnostics/diagnostic_listener.h"
#include "../lexer/token.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringMap.h"
#include <cassert>
#include <deque>
#include <ranges>
#include <string>
#include <vector>

enum class SymbolKind {
    Local,
    Function,
    Parameter,
    LoopVariable,
    Builtin,
    Struct
};

struct Symbol {
    SymbolKind kind;
    std::string name; // identifier used to look up the symbol
    SourceLocation location;
    const Node *declaration_node;
    bool is_mutable{false};
};

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
        scopes_.pop_back();
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

  private:
    // StringMap stores pointers to Symbols that live stably in symbol_pool_
    std::vector<llvm::StringMap<const Symbol *>> scopes_;
    // Stable symbol storage so pointers remain valid after exiting scopes
    std::deque<Symbol> symbol_pool_;
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

    [[nodiscard]] const Symbol *get_symbol_for_node(const Node *node) const {
        auto const found = node_to_symbol_map_.find(node);
        return found != node_to_symbol_map_.end() ? found->second : nullptr;
    }

  private:
    void resolve_decl(const Decl &decl);
    void resolve_stmt(const Stmt &stmt);
    void resolve_expr(const Expr &expr);
    void resolve_type(const Type &type);
    void error(const Node *node, const std::string &message);
    void note(SourceLocation loc, const std::string &message);
    llvm::DenseMap<const Node *, const Symbol *> node_to_symbol_map_;
    ScopeStack scope_stack_;
    DiagnosticListener *listener_;
    std::size_t error_count_{0};
};
