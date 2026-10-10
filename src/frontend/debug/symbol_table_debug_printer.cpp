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

struct Row {
    std::size_t depth;
    const Symbol *symbol;
};

} // namespace

SymbolTableDebugPrinter::SymbolTableDebugPrinter(std::ostream &output)
    : output_(output) {
}

void SymbolTableDebugPrinter::print(const ScopeStack &scope_stack) const {
    const auto &scopes = scope_stack.scopes();

    std::vector<Row> rows;
    for (std::size_t depth = 0; depth < scopes.size(); ++depth) {
        std::vector<const Symbol *> in_scope;
        for (const auto &entry : scopes[depth]) {
            in_scope.push_back(entry.second);
        }
        std::ranges::sort(in_scope, {}, &Symbol::name);
        for (const Symbol *symbol : in_scope) {
            rows.push_back({depth, symbol});
        }
    }

    // column widths: at least as wide as the header, grown to fit the data
    std::size_t name_w = 4;
    std::size_t kind_w = 4;
    for (const Row &row : rows) {
        name_w = std::max(name_w, row.symbol->name.size());
        kind_w =
            std::max(kind_w, std::string(kind_name(row.symbol->kind)).size());
    }
    constexpr std::size_t scope_w = 5;
    constexpr std::size_t mut_w = 3;
    constexpr std::size_t offset_w = 6;
    constexpr std::size_t length_w = 6;

    const auto rule = [&] {
        output_ << '+';
        for (const std::size_t w :
             {scope_w, name_w, kind_w, mut_w, offset_w, length_w}) {
            output_ << std::string(w + 2, '-') << '+';
        }
        output_ << '\n';
    };
    const auto line = [&](const std::string &scope, const std::string &name,
                          const std::string &kind, const std::string &mut,
                          const std::string &offset,
                          const std::string &length) {
        output_ << "| " << std::left << std::setw(scope_w) << scope << " | "
                << std::setw(name_w) << name << " | " << std::setw(kind_w)
                << kind << " | " << std::setw(mut_w) << mut << " | "
                << std::right << std::setw(offset_w) << offset << " | "
                << std::setw(length_w) << length << " |\n";
    };

    rule();
    line("Scope", "Name", "Kind", "Mut", "Offset", "Length");
    rule();
    for (const Row &row : rows) {
        const Symbol &s = *row.symbol;
        line(std::to_string(row.depth), s.name, kind_name(s.kind),
             s.is_mutable ? "yes" : "no", std::to_string(s.location.start),
             std::to_string(s.location.length));
    }
    rule();
}
