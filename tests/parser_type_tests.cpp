#include "frontend/parser/parser.h"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

namespace {
int failures = 0;

void fail(std::string_view test_name, std::string_view message) {
    std::cerr << "[FAIL] " << test_name << ": " << message << '\n';
    failures++;
}

std::unique_ptr<Type> parse_type(const std::string &source) {
    Parser parser(source.data(), source.size());
    return parser.parse_type();
}

void expect_parser_error(std::string_view test_name, std::string source) {
    try {
        auto type = parse_type(source);
        (void)type;
        fail(test_name, "source was accepted");
    } catch (const ParserError &) {
    }
}

void test_array_type() {
    auto type = parse_type("[i32; 4]");
    const auto *array = dynamic_cast<const ArrayType *>(type.get());
    if (!array || array->dimensions().size() != 1) {
        fail("array type", "one-dimensional array was not preserved");
        return;
    }
    const auto *element =
        dynamic_cast<const PrimitiveType *>(&array->element_type());
    if (!element || element->primitive_type() != PrimitiveTypeKind::i32) {
        fail("array type", "element type is not i32");
    }
}

void test_expression_dimensions() {
    auto type = parse_type("[f32; 2 + 2, 8 / 2]");
    const auto *array = dynamic_cast<const ArrayType *>(type.get());
    if (!array || array->dimensions().size() != 2) {
        fail("expression dimensions", "two dimensions were not preserved");
        return;
    }
    const auto *first =
        dynamic_cast<const BinaryExpr *>(array->dimensions()[0].get());
    const auto *second =
        dynamic_cast<const BinaryExpr *>(array->dimensions()[1].get());
    if (!first || first->op() != BinOp::Add || !second ||
        second->op() != BinOp::Divide) {
        fail("expression dimensions", "dimension expressions are incorrect");
    }
}

void test_nested_array_type() {
    auto type = parse_type("[[i32; 2]; 3]");
    const auto *outer = dynamic_cast<const ArrayType *>(type.get());
    const auto *inner =
        outer ? dynamic_cast<const ArrayType *>(&outer->element_type())
              : nullptr;
    if (!outer || outer->dimensions().size() != 1 || !inner ||
        inner->dimensions().size() != 1) {
        fail("nested array type", "nested array structure is incorrect");
    }
}

void test_invalid_array_types() {
    expect_parser_error("missing dimension", "[i32;]");
    expect_parser_error("missing semicolon and dimension", "[i32]");
    expect_parser_error("trailing dimension comma", "[i32; 4,]");
}

void test_repeat_array_remains_expression() {
    const std::string source = "let values = [0; 2, 3];";
    Parser parser(source.data(), source.size());
    auto statement = parser.parse_statement();
    const auto *variable = dynamic_cast<const VarDeclStmt *>(statement.get());
    const auto *repeat =
        variable
            ? dynamic_cast<const RepeatArrayExpr *>(&variable->initializer())
            : nullptr;
    if (!repeat || repeat->dimensions().size() != 2) {
        fail("repeat array expression",
             "repeat array was not preserved as an expression");
    }
}
} // namespace

int main() {
    test_array_type();
    test_expression_dimensions();
    test_nested_array_type();
    test_invalid_array_types();
    test_repeat_array_remains_expression();
    if (failures != 0) {
        std::cerr << failures << " type parser test(s) failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "All type parser tests passed\n";
    return EXIT_SUCCESS;
}
