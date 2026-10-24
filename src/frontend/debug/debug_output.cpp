#include "debug_output.h"
#include "ast_debug_printer.h"
#include "frontend/diagnostics/diagnostic_listener.h"
#include "frontend/parser/parser.h"
#include "frontend/symbol-resolution/scope.h"
#include "symbol_table_debug_printer.h"
#include <fstream>
#include <iostream>
#include <memory>

namespace {

// pass 1: source text -> AST. Returns null after reporting a syntax error.
std::unique_ptr<Program> parse_pass(std::ostream &diagnostics,
                                    std::string_view file_name,
                                    std::string_view source) {
    try {
        Parser parser(source.data(), source.size());
        return parser.parse();
    } catch (const ParserError &error) {
        TextDiagnosticListener listener(diagnostics, file_name, source);
        listener.report(
            {DiagnosticSeverity::Error, error.location(), error.what()});
        return nullptr;
    }
}

// debug view of pass 1: the source and the AST it produced
void print_ast_pass(std::ostream &out, std::string_view title,
                    std::string_view source, const Program &program) {
    out << "=== " << title << " ===\n\n";
    out << "Source:\n" << source << "\nAST:\n";
    AstDebugPrinter const printer(out);
    printer.print(program);
    out << '\n';
}

// pass 2: name resolution. Writes each scope's symbol table to `out` as the
// scope closes (innermost first), with any diagnostics where they happen.
// Returns false if any error was reported.
bool symbol_resolution_pass(std::ostream &out, std::string_view file_name,
                            std::string_view source, const Program &program) {
    TextDiagnosticListener listener(out, file_name, source);
    ScopeResolver resolver(&listener);
    SymbolTableDebugPrinter const symbol_printer(out);
    resolver.set_scope_exit_hook(
        [&symbol_printer](std::size_t depth,
                          const llvm::StringMap<const Symbol *> &scope) {
            symbol_printer.print_scope(depth, scope);
        });
    out << "Resolving symbols...\n\n";
    resolver.resolve(program);
    // whatever is left (the global scope) never closes, so print it here
    symbol_printer.print(resolver.scope_stack());
    return !resolver.had_errors();
}

// opens `directory/name` for writing, or reports why it couldn't
bool open_output(std::ofstream &file, const std::filesystem::path &path) {
    file.open(path);
    if (!file) {
        std::cerr << "Could not write: " << path.string() << '\n';
        return false;
    }
    return true;
}

} // namespace

// runs every pass in order, stopping at the first one that fails. Each pass
// writes its own file into `out_dir`: ast.txt and symbols.txt.
bool write_debug_output(std::string_view title, std::string_view file_name,
                        std::string_view source,
                        const std::filesystem::path &out_dir) {
    std::error_code ec;
    std::filesystem::create_directories(out_dir, ec);
    if (ec) {
        std::cerr << "Could not create " << out_dir.string() << ": "
                  << ec.message() << '\n';
        return false;
    }

    const auto ast_path = out_dir / "ast.txt";
    std::ofstream ast_file;
    if (!open_output(ast_file, ast_path)) {
        return false;
    }
    std::ofstream source_file;
    const auto source_path =
        out_dir / std::filesystem::path(file_name).filename();
    if (!open_output(source_file, source_path)) {
        return false;
    }
    source_file << source;
    source_file.close();
    if (!source_file) {
        std::cerr << "Could not write: " << source_path.string() << '\n';
        return false;
    }

    const auto symbols_path = out_dir / "symbols.txt";
    std::ofstream symbols_file;
    if (!open_output(symbols_file, symbols_path)) {
        return false;
    }
    const auto program = parse_pass(ast_file, file_name, source);
    if (!program) {
        symbols_file << "Skipped: parsing failed; see ast.txt.\n";
        std::cerr << "Parsing failed; see " << ast_path.string() << '\n';
        return false;
    }
    print_ast_pass(ast_file, title, source, *program);
    const bool resolved =
        symbol_resolution_pass(symbols_file, file_name, source, *program);
    std::cout << "wrote " << symbols_path.string()
              << (resolved ? "" : " (name resolution reported errors)") << '\n';
    ast_file.close();
    symbols_file.close();
    if (!ast_file || !symbols_file) {
        std::cerr << "Could not finish writing debug output in "
                  << out_dir.string() << '\n';
        return false;
    }
    return resolved;
}
