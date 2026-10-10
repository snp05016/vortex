#include "symbol_table_debug_printer.h"
#include <algorithm>
#include <iomanip>
#include <ostream>
#include <string>
#include <vector>

namespace {

const char *kind_name(SymbolKind kind) {
    switch (kind) {
    case SymbolKind::Local:
        return "Local";
    case SymbolKind::Function:
        return "Function";
    case SymbolKind::Parameter:
        return "Parameter";
    case SymbolKind::LoopVariable:
        return "LoopVariable";
    case SymbolKind::Builtin:
        return "Builtin";
    case SymbolKind::Struct:
        return "Struct";
    case SymbolKind::Field:
        return "Field";
    }
    return "Unknown";
}

} // namespace

SymbolTableDebugPrinter::SymbolTableDebugPrinter(std::ostream &output)
    : output_(output) {
}

void SymbolTableDebugPrinter::print_scope(
    std::size_t depth, const llvm::StringMap<const Symbol *> &scope) const {
    std::vector<const Symbol *> symbols;
    for (const auto &entry : scope) {
        symbols.push_back(entry.second);
    }
    std::ranges::sort(symbols, {}, &Symbol::name);

    output_ << "Scope " << depth << (depth == 0 ? " (global)" : "") << ": "
            << symbols.size() << (symbols.size() == 1 ? " symbol" : " symbols")
            << '\n';
    if (symbols.empty()) {
        output_ << '\n';
        return;
    }

    // column widths: at least as wide as the header, grown to fit the data
    std::size_t name_w = 4;
    std::size_t kind_w = 4;
    for (const Symbol *symbol : symbols) {
        name_w = std::max(name_w, symbol->name.size());
        kind_w = std::max(kind_w, std::string(kind_name(symbol->kind)).size());
    }
    constexpr std::size_t mut_w = 3;
    constexpr std::size_t offset_w = 6;
    constexpr std::size_t length_w = 6;

    const auto rule = [&] {
        output_ << '+';
        for (const std::size_t w :
             {name_w, kind_w, mut_w, offset_w, length_w}) {
            output_ << std::string(w + 2, '-') << '+';
        }
        output_ << '\n';
    };
    const auto line = [&](const std::string &name, const std::string &kind,
                          const std::string &mut, const std::string &offset,
                          const std::string &length) {
        output_ << "| " << std::left << std::setw(name_w) << name << " | "
                << std::setw(kind_w) << kind << " | " << std::setw(mut_w) << mut
                << " | " << std::right << std::setw(offset_w) << offset << " | "
                << std::setw(length_w) << length << " |\n";
    };

    rule();
    line("Name", "Kind", "Mut", "Offset", "Length");
    rule();
    for (const Symbol *symbol : symbols) {
        line(symbol->name, kind_name(symbol->kind),
             symbol->is_mutable ? "yes" : "no",
             std::to_string(symbol->location.start),
             std::to_string(symbol->location.length));
    }
    rule();
    output_ << '\n';
}

void SymbolTableDebugPrinter::print(const ScopeStack &scope_stack) const {
    const auto &scopes = scope_stack.scopes();
    for (std::size_t depth = 0; depth < scopes.size(); ++depth) {
        print_scope(depth, scopes[depth]);
    }
}
