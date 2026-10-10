#pragma once

#include "../symbol-resolution/scope.h"
#include <iosfwd>

// Prints scopes as bordered tables, for debugging.
class SymbolTableDebugPrinter {
  public:
    explicit SymbolTableDebugPrinter(std::ostream &output);

    // Prints one scope: a title line, then a table of its symbols sorted by
    // name (StringMap iteration order is arbitrary). Depth 0 is the global
    // scope.
    void print_scope(std::size_t depth,
                     const llvm::StringMap<const Symbol *> &scope) const;

    // Prints every scope still on the stack, outermost first.
    void print(const ScopeStack &scope_stack) const;

  private:
    std::ostream &output_;
};
