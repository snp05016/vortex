#include "frontend/debug/ast_debug_printer.h"
#include "frontend/parser/parser.h"
#include <array>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>

namespace {

void print_ast(std::string_view title, std::string_view source) {
    Parser parser(source.data(), source.size());
    auto const program = parser.parse();
    std::cout << "=== " << title << " ===\n\n";
    std::cout << "Source:\n" << source << "\nAST:\n";
    AstDebugPrinter const printer(std::cout);
    printer.print(*program);
    std::cout << '\n';
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
    for (const Example &example : examples) {
        print_ast(example.name, example.source);
    }
    return 0;
}

int run_file(const std::string &path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        std::cerr << "Could not open: " << path << '\n';
        return 1;
    }
    const std::string source{std::istreambuf_iterator<char>(input),
                             std::istreambuf_iterator<char>()};
    print_ast("AST for " + path, source);
    return 0;
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
