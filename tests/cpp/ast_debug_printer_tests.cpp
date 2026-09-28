#include "frontend/debug/ast_debug_printer.h"
#include "frontend/parser/parser.h"
#include <cstdlib>
#include <iostream>
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

std::string print_program(const std::string &source) {
    Parser parser(source.data(), source.size());
    auto const program = parser.parse();
    std::ostringstream output;
    AstDebugPrinter const printer(output);
    printer.print(*program);
    return output.str();
}

void expect_contains(std::string_view test_name, const std::string &output,
                     const std::vector<std::string_view> &expected) {
    for (std::string_view const text : expected) {
        if (!output.contains(text)) {
            fail(test_name, std::string("missing '") + std::string(text) + "'");
        }
    }
}

void test_empty_program() {
    const std::string output = print_program("");
    if (output != "Program\n") {
        fail("empty program", "unexpected tree output");
    }
}

void test_program_tree() {
    const std::string source = R"(
struct Point { x: f32, y: f32, }
fn calculate(point: &mut Point, values: [i32; 2]) -> i32 {
    let mut total: i32 = values[0] + 1;
    let origin = Point { x: 0.0, y: 0.0 };
    let items = [1, 2];
    let zeros = [0; 2];
    if total > 0 {
        total += 1;
    } else {
        return -1;
    }
    for index in 0..=2 {
        print(origin.x);
        continue;
    }
    while total < 10 {
        break;
    }
    return i32(total);
}
)";
    const std::string output = print_program(source);
    expect_contains("program tree", output,
                    {"Program\n├── StructDecl name=Point",
                     "Field name=x",
                     "└── FunctionDecl name=calculate",
                     "Parameter name=point",
                     "ReferenceType mutable=true",
                     "ArrayType",
                     "VarDeclStmt name=total",
                     "IndexExpr",
                     "BinaryExpr operator=\"+\"",
                     "StructConstructionExpr name=Point",
                     "ArrayExpr",
                     "RepeatArrayExpr",
                     "IfStmt",
                     "AssignmentStmt operator=\"+=\"",
                     "UnaryExpr operator=\"-\"",
                     "ForStmt variable=index",
                     "RangeExpr inclusive=true",
                     "CallExpr",
                     "FieldAccessExpr field=x",
                     "ContinueStmt",
                     "WhileStmt",
                     "BreakStmt",
                     "CastExpr target=i32",
                     "│   ",
                     "└── "});
}
} // namespace

int main() {
    test_empty_program();
    test_program_tree();
    if (failures != 0) {
        std::cerr << failures << " ast debug printer test(s) failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "All AST debug printer tests passed\n";
    return EXIT_SUCCESS;
}
