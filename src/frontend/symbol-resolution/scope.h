#pragma once
#include "../ast/ast.h"
#include "../lexer/token.h"
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
    const Node *declaration_node; // Pointer to the AST node where the symbol is
                                  // declared
    // Additional information can be added here, such as type, scope level, etc.
};

class ScopeStack {
  public:
    explicit ScopeStack(ScopeStack *parent = nullptr) : parent_scope(parent) {
    }

    void enter_scope() {
        scopes.emplace_back();
    }

    void exit_scope() {
        if (!scopes.empty()) {
            scopes.pop_back();
        }
    }

    void add_symbol(const Symbol &symbol) {
        if (!scopes.empty()) {
            scopes.back()[symbol.name] = symbol;
        }
    }

    const Symbol *lookup(const std::string &name) const {
        for (const ScopeStack *scope = this; scope != nullptr;
             scope = scope->parent_scope) {
            for (auto it = scope->scopes.rbegin(); it != scope->scopes.rend();
                 ++it) {
                auto found = it->find(name);
                if (found != it->end()) {
                    return &found->second;
                }
            }
        }
        return nullptr; // Not found
    }

    void DeclareSymbol(const std::string &name, SymbolKind kind,
                       SourceLocation location, const Node *declaration_node) {
        Symbol symbol{kind, name, location, declaration_node};
        add_symbol(symbol);
    }

  private:
    ScopeStack *parent_scope;
    std::vector<llvm::StringMap<Symbol>> scopes;
};