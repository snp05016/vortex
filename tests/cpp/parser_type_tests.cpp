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

void expect_parser_error(std::string_view test_name,
                         const std::string &source) {
    try {
        auto const type = parse_type(source);
        (void)type;
        fail(test_name, "source was accepted");
    } catch (const ParserError &) {
    }
}

void test_array_type() {
    auto const type = parse_type("[i32; 4]");
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
    auto const type = parse_type("[f32; 2 + 2, 8 / 2]");
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
    auto const type = parse_type("[[i32; 2]; 3]");
    const auto *outer = dynamic_cast<const ArrayType *>(type.get());
    const auto *inner =
        outer ? dynamic_cast<const ArrayType *>(&outer->element_type())
              : nullptr;
    if (!outer || outer->dimensions().size() != 1 || !inner ||
        inner->dimensions().size() != 1) {
        fail("nested array type", "nested array structure is incorrect");
    }
}

void test_named_type() {
    auto const type = parse_type("Point");
    const auto *named = dynamic_cast<const StructType *>(type.get());
    if (!named || named->struct_name() != "Point") {
        fail("named type", "identifier was not preserved as a named type");
    }
}

void test_reference_types() {
    auto const shared_type = parse_type("&Point");
    const auto *shared = dynamic_cast<const ReferenceType *>(shared_type.get());
    const auto *named =
        shared ? dynamic_cast<const StructType *>(&shared->referenced_type())
               : nullptr;
    if (!shared || shared->is_mutable() || !named ||
        named->struct_name() != "Point") {
        fail("shared reference", "&Point was not preserved correctly");
    }

    auto const mutable_type = parse_type("&mut [f32; 4 * 2, 8]");
    const auto *mutable_reference =
        dynamic_cast<const ReferenceType *>(mutable_type.get());
    const auto *array = mutable_reference
                            ? dynamic_cast<const ArrayType *>(
                                  &mutable_reference->referenced_type())
                            : nullptr;
    if (!mutable_reference || !mutable_reference->is_mutable() || !array ||
        array->dimensions().size() != 2) {
        fail("mutable reference",
             "&mut array type was not preserved correctly");
    }
}

void test_recursive_type_forms() {
    auto const array_type = parse_type("[Point; 4]");
    const auto *array = dynamic_cast<const ArrayType *>(array_type.get());
    const auto *element =
        array ? dynamic_cast<const StructType *>(&array->element_type())
              : nullptr;
    if (!array || !element || element->struct_name() != "Point") {
        fail("named array element", "array did not preserve its named element");
    }

    auto const nested_type = parse_type("& &i32");
    const auto *outer = dynamic_cast<const ReferenceType *>(nested_type.get());
    const auto *inner =
        outer ? dynamic_cast<const ReferenceType *>(&outer->referenced_type())
              : nullptr;
    if (!outer || !inner) {
        fail("nested reference", "recursive reference type was not parsed");
    }
}

void test_invalid_reference_types() {
    expect_parser_error("missing referenced type", "&");
    expect_parser_error("missing mutable referenced type", "&mut");
}

void test_invalid_array_types() {
    expect_parser_error("missing dimension", "[i32;]");
    expect_parser_error("missing semicolon and dimension", "[i32]");
    expect_parser_error("trailing dimension comma", "[i32; 4,]");
}

void test_repeat_array_remains_expression() {
    const std::string source = "let values = [0; 2, 3];";
    Parser parser(source.data(), source.size());
    auto const statement = parser.parse_statement();
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
    test_named_type();
    test_reference_types();
    test_recursive_type_forms();
    test_invalid_reference_types();
    test_invalid_array_types();
    test_repeat_array_remains_expression();
    if (failures != 0) {
        std::cerr << failures << " type parser test(s) failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "All type parser tests passed\n";
    return EXIT_SUCCESS;
}
