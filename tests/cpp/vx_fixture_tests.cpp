#include "frontend/debug/ast_debug_printer.h"
#include "frontend/debug/debug_output.h"
#include "frontend/parser/parser.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>

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
    std::size_t count = 0;
    for (const auto &entry : std::filesystem::directory_iterator(
             std::filesystem::path(VORTEX_SOURCE_DIR) / "tests/vx")) {
        if (!entry.is_regular_file() || entry.path().extension() != ".vx") {
            continue;
        }
        const std::string path = "tests/vx/" + entry.path().filename().string();
        const std::string source = read_fixture(path);
        // These fixtures assert parser behavior; resolution diagnostics are for
        // inspection.
        const auto out_dir = std::filesystem::path(VORTEX_SOURCE_DIR) /
                             "output" / std::filesystem::path(path).stem();
        (void)write_debug_output(path, path, source, out_dir);
        for (const auto &name : {std::filesystem::path(path).filename(),
                                 std::filesystem::path("ast.txt"),
                                 std::filesystem::path("symbols.txt")}) {
            if (!std::filesystem::is_regular_file(out_dir / name)) {
                fail(path, "missing debug output file");
            }
        }
        const std::string output_path =
            "output/" + entry.path().stem().string() + "/";
        if (read_fixture(output_path + entry.path().filename().string()) !=
            source) {
            fail(path, "output source differs from fixture");
        }
        const std::string ast = read_fixture(output_path + "ast.txt");
        const std::string symbols = read_fixture(output_path + "symbols.txt");
        if (entry.path().filename().string().starts_with("invalid_")) {
            if (!ast.contains("error:") || !symbols.contains("Skipped:")) {
                fail(path,
                     "missing parse diagnostic or skipped-pass explanation");
            }
            test_invalid_fixture(path);
        } else {
            if (!ast.contains("AST:") ||
                !symbols.contains("Resolving symbols")) {
                fail(path, "missing AST or symbol-resolution output");
            }
            test_valid_fixture(path);
        }
        ++count;
    }
    if (count == 0) {
        fail("fixtures", "no .vx fixtures found");
    }
    if (failures != 0) {
        std::cerr << failures << " fixture test(s) failed\n";
        return EXIT_FAILURE;
    }
    std::cout << count << " Vortex fixtures passed\n";
    return EXIT_SUCCESS;
}
