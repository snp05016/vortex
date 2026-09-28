#include "frontend/debug/ast_debug_printer.h"
#include "frontend/parser/parser.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {
int failures = 0;

void fail(std::string_view test_name, std::string_view message) {
    std::cerr << "[FAIL] " << test_name << ": " << message << '\n';
    failures++;
}

std::string read_fixture(std::string_view relative_path) {
    const std::string path =
        std::string(VORTEX_SOURCE_DIR) + "/" + std::string(relative_path);
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        fail(relative_path, "could not open fixture");
        return {};
    }
    return {std::istreambuf_iterator<char>(input),
            std::istreambuf_iterator<char>()};
}

void test_valid_fixture(std::string_view path) {
    const std::string source = read_fixture(path);
    try {
        Parser parser(source.data(), source.size());
        auto const program = parser.parse();
        std::ostringstream output;
        AstDebugPrinter const printer(output);
        printer.print(*program);
        if (output.str().contains("Unknown")) {
            fail(path, "AST tree contains an unknown node");
        }
    } catch (const std::exception &error) {
        fail(path, error.what());
    }
}

void test_invalid_fixture(std::string_view path) {
    const std::string source = read_fixture(path);
    try {
        Parser parser(source.data(), source.size());
        auto const program = parser.parse();
        (void)program;
        fail(path, "invalid fixture was accepted");
    } catch (const ParserError &) {
    } catch (const std::exception &error) {
        fail(path, std::string("unexpected exception: ") + error.what());
    }
}
} // namespace

int main() {
    const std::vector<std::string_view> valid = {
        "tests/vx/valid/comment_only.vx",
        "tests/vx/valid/declarations_and_types.vx",
        "tests/vx/valid/expression_edges.vx",
        "tests/vx/valid/control_flow_edges.vx",
        "tests/vx/valid/literal_edges.vx",
        "tests/vx/valid/postfix_and_assignments.vx",
        "tests/vx/valid/syntax_semantic_boundary.vx",
    };
    const std::vector<std::string_view> invalid = {
        "tests/vx/invalid/empty_struct.vx",
        "tests/vx/invalid/trailing_parameter_comma.vx",
        "tests/vx/invalid/top_level_statement.vx",
        "tests/vx/invalid/missing_array_dimension.vx",
        "tests/vx/invalid/chained_comparison.vx",
        "tests/vx/invalid/invalid_assignment_target.vx",
        "tests/vx/invalid/trailing_call_comma.vx",
        "tests/vx/invalid/unbraced_else.vx",
        "tests/vx/invalid/trailing_repeat_dimension.vx",
        "tests/vx/invalid/unterminated_function.vx",
        "tests/vx/invalid/legacy_struct_syntax.vx",
    };

    for (std::string_view const path : valid) {
        test_valid_fixture(path);
    }
    for (std::string_view const path : invalid) {
        test_invalid_fixture(path);
    }
    if (failures != 0) {
        std::cerr << failures << " fixture test(s) failed\n";
        return EXIT_FAILURE;
    }
    std::cout << valid.size() << " valid and " << invalid.size()
              << " invalid Vortex fixtures passed\n";
    return EXIT_SUCCESS;
}
