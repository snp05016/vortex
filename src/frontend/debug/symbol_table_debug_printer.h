#pragma once

#include "../symbol-resolution/scope.h"
#include <iosfwd>

// Prints the symbols still in scope as a bordered table, for debugging.
class SymbolTableDebugPrinter {
  public:
    explicit SymbolTableDebugPrinter(std::ostream &output);

    // Scope 0 is the global scope. Rows are sorted by name within a scope
    // since StringMap iteration order is arbitrary.
    void print(const ScopeStack &scope_stack) const;

  private:
    std::ostream &output_;
};
