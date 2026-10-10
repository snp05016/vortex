#include "frontend/debug/ast_debug_printer.h"
#include "frontend/debug/symbol_table_debug_printer.h"
#include "frontend/diagnostics/diagnostic_listener.h"
#include "frontend/parser/parser.h"
#include "frontend/symbol-resolution/scope.h"
#include <algorithm>
#include <array>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>

namespace {

// pass 1: source text -> AST. Returns null after reporting a syntax error.
std::unique_ptr<Program> parse_pass(std::string_view file_name,
                                    std::string_view source) {
    try {
        Parser parser(source.data(), source.size());
        return parser.parse();
    } catch (const ParserError &error) {
        TextDiagnosticListener listener(std::cerr, file_name, source);
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

// runs every pass in order, stopping at the first one that fails. Each pass
// writes its own file into `out_dir`: ast.txt and symbols.txt.
bool run_passes(std::string_view title, std::string_view file_name,
                std::string_view source, const std::filesystem::path &out_dir) {
    const auto program = parse_pass(file_name, source);
    if (!program) {
        return false;
    }

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
    print_ast_pass(ast_file, title, source, *program);
    std::cout << "wrote " << ast_path.string() << '\n';

    const auto symbols_path = out_dir / "symbols.txt";
    std::ofstream symbols_file;
    if (!open_output(symbols_file, symbols_path)) {
        return false;
    }
    const bool resolved =
        symbol_resolution_pass(symbols_file, file_name, source, *program);
    std::cout << "wrote " << symbols_path.string()
              << (resolved ? "" : " (name resolution reported errors)") << '\n';
    return resolved;
}

int run_examples() {
    struct Example {
        std::string_view name;
        std::string_view source;
    };

    constexpr std::array examples = {
        Example{"function and expression", "fn main() {\n"
                                           "    let value: i32 = 2 + 3 * 4;\n"
                                           "    return;\n"
                                           "}\n"},
        Example{"struct and references", "struct Point { x: f32, y: f32, }\n"
                                         "fn read_x(point: &Point) -> f32 {\n"
                                         "    return point.x;\n"
                                         "}\n"},
        Example{"control flow", "fn count() {\n"
                                "    let mut total = 0;\n"
                                "    for index in 0..=3 {\n"
                                "        if index == 2 { continue; }\n"
                                "        total += index;\n"
                                "    }\n"
                                "    while total < 10 { total += 1; }\n"
                                "}\n"},
        Example{"arrays and construction",
                "struct Point { x: f32, y: f32 }\n"
                "fn values() {\n"
                "    let origin = Point { x: 0.0, y: 0.0 };\n"
                "    let numbers: [i32; 3] = [1, 2, 3];\n"
                "    let zeros = [0; 2, 2];\n"
                "    print(origin.x, numbers[0]);\n"
                "}\n"},
    };
    bool all_valid = true;
    for (const Example &example : examples) {
        const std::string file_name =
            "<example:" + std::string(example.name) + ">";
        // "control flow" -> examples.out/control_flow
        std::string slug(example.name);
        std::ranges::replace(slug, ' ', '_');
        all_valid = run_passes(example.name, file_name, example.source,
                               std::filesystem::path("examples.out") / slug) &&
                    all_valid;
    }
    return all_valid ? 0 : 1;
}

int run_file(const std::string &path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        std::cerr << "Could not open: " << path << '\n';
        return 1;
    }
    const std::string source{std::istreambuf_iterator<char>(input),
                             std::istreambuf_iterator<char>()};
    // sample.vx -> sample.out/, next to the source file
    const std::filesystem::path source_path(path);
    const auto out_dir =
        source_path.parent_path() / (source_path.stem().string() + ".out");
    return run_passes("AST for " + path, path, source, out_dir) ? 0 : 1;
}

int run(int argc, char *const *argv) {
    if (argc == 1) {
        return run_examples();
    }
    if (argc == 2) {
        return run_file(argv[1]);
    }
    std::cerr << "Usage: vortex_debug [source-file]\n";
    return 1;
}

} // namespace

int main(int argc, char *const *argv) {
    try {
        return run(argc, argv);
    } catch (const std::exception &error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
