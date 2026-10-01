#pragma once
#include "../ast/ast.h"
#include "../lexer/token.h"
#include <cassert>
#include <llvm/ADT/StringMap.h>
#include <string>
#include <vector>

enum class SymbolKind {
    Local,
    Function,
    Parameter,
    LoopVariable,
    Builtin,
};

struct Symbol {
    SymbolKind kind;
    std::string name;
    SourceLocation location;
    const Node *declaration_node;
};

class ScopeStack {
  public:
    ScopeStack() {
        scopes_.emplace_back();
    } // global scope always exists

    void enter_scope() {
        scopes_.emplace_back();
    }

    void exit_scope() {
        assert(scopes_.size() > 1 && "attempted to exit global scope");
        scopes_.pop_back();
    }

    // overloading so that i dont ahve to create a symbol object everytime i
    // want to declare a symbol
    bool declare(const Symbol &sym) {
        return scopes_.back().insert({sym.name, sym}).second;
    }

    bool declare(const std::string &name, SymbolKind kind, SourceLocation loc,
                 const Node *decl) {
        return declare(Symbol{kind, name, loc, decl});
    }

    // looks up a symbol by name, starting from the innermost scope
    const Symbol *lookup(const std::string &name) const {
        for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end())
                return &found->second;
        }
        return nullptr;
    }

    // current scope only looku`p, does not check outer scopes
    const Symbol *lookup_current(const std::string &name) const {
        auto found = scopes_.back().find(name);
        return found == scopes_.back().end() ? nullptr : &found->second;
    }

  private:
    std::vector<llvm::StringMap<Symbol>> scopes_;
};

// resolves symbols in the ast using a stack of scopes.
class ScopeResolver {
  public:
    void resolve(const Program &program);

  private:
    void resolve_decl(const Decl &decl);
    void resolve_stmt(const Stmt &stmt);
    void resolve_expr(const Expr &expr);
    void resolve_type(const Type &type);

    ScopeStack scope_stack_;
};