#include "frontend/parser/parser.h"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace {
int failures = 0;

void fail(std::string_view test_name, std::string_view message) {
    std::cerr << "[FAIL] " << test_name << ": " << message << '\n';
    failures++;
}

std::unique_ptr<Decl> parse_declaration(const std::string &source) {
    Parser parser(source.data(), source.size());
    return parser.parse_declaration();
}

void expect_parser_error(std::string_view test_name,
                         const std::string &source) {
    try {
        auto const declaration = parse_declaration(source);
        (void)declaration;
        fail(test_name, "source was accepted");
    } catch (const ParserError &) {
    }
}

const PrimitiveType *as_primitive(const Type &type) {
    return dynamic_cast<const PrimitiveType *>(&type);
}

void test_empty_function() {
    auto const declaration = parse_declaration("  fn main() {}");
    const auto *function =
        dynamic_cast<const FunctionDecl *>(declaration.get());
    const auto *return_type =
        function ? as_primitive(function->return_type()) : nullptr;

    if (!function || function->function_name() != "main" ||
        function->location().start != 2 || !function->parameters().empty() ||
        !function->body().statements().empty() || !return_type ||
        return_type->primitive_type() != PrimitiveTypeKind::Void) {
        fail("empty function",
             "name, location, default return type, or body was incorrect");
    }
}

void test_function_signature_and_body() {
    const std::string source =
        "fn transform(point: &mut Point, values: [f32; 2 + 2, 8]) -> Point "
        "{ let copy = point; return copy; }";
    auto const declaration = parse_declaration(source);
    const auto *function =
        dynamic_cast<const FunctionDecl *>(declaration.get());
    if (!function || function->function_name() != "transform" ||
        function->parameters().size() != 2 ||
        function->body().statements().size() != 2) {
        fail("function signature", "function structure was not preserved");
        return;
    }

    const ParamDecl &point = function->parameters()[0];
    const auto *point_reference =
        dynamic_cast<const ReferenceType *>(&point.param_type());
    const auto *point_type = point_reference
                                 ? dynamic_cast<const StructType *>(
                                       &point_reference->referenced_type())
                                 : nullptr;
    if (point.param_name() != "point" || !point_reference ||
        !point_reference->is_mutable() || !point_type ||
        point_type->struct_name() != "Point") {
        fail("reference parameter", "&mut Point parameter was incorrect");
    }

    const ParamDecl &values = function->parameters()[1];
    const auto *array = dynamic_cast<const ArrayType *>(&values.param_type());
    const auto *element = array ? as_primitive(array->element_type()) : nullptr;
    if (values.param_name() != "values" || !array ||
        array->dimensions().size() != 2 || !element ||
        element->primitive_type() != PrimitiveTypeKind::f32) {
        fail("array parameter", "array parameter type was incorrect");
    }

    const auto *return_type =
        dynamic_cast<const StructType *>(&function->return_type());
    if (!return_type || return_type->struct_name() != "Point") {
        fail("named return type", "named return type was not preserved");
    }
    if (!dynamic_cast<const VarDeclStmt *>(
            function->body().statements()[0].get()) ||
        !dynamic_cast<const RetStmt *>(
            function->body().statements()[1].get())) {
        fail("function body", "body statements were not preserved in order");
    }
}

void test_semantic_errors_still_parse() {
    auto const declaration =
        parse_declaration("fn unusual(value: void) -> &i32 {}");
    const auto *function =
        dynamic_cast<const FunctionDecl *>(declaration.get());
    const auto *parameter_type =
        function && function->parameters().size() == 1
            ? as_primitive(function->parameters()[0].param_type())
            : nullptr;
    const auto *return_type =
        function ? dynamic_cast<const ReferenceType *>(&function->return_type())
                 : nullptr;
    if (!parameter_type ||
        parameter_type->primitive_type() != PrimitiveTypeKind::Void ||
        !return_type) {
        fail("semantic boundary",
             "syntactically valid semantic errors were rejected");
    }
}

void test_struct_fields() {
    auto const declaration =
        parse_declaration("  struct Point { x: f32, y: f32, }");
    const auto *structure = dynamic_cast<const StructDecl *>(declaration.get());
    if (!structure || structure->struct_name() != "Point" ||
        structure->location().start != 2 || structure->fields().size() != 2) {
        fail("struct fields",
             "struct name, location, or fields were incorrect");
        return;
    }

    const StructFieldDecl &x = structure->fields()[0];
    const StructFieldDecl &y = structure->fields()[1];
    const auto *x_type = as_primitive(x.field_type());
    const auto *y_type = as_primitive(y.field_type());
    if (x.field_name() != "x" || y.field_name() != "y" || !x_type || !y_type ||
        x_type->primitive_type() != PrimitiveTypeKind::f32 ||
        y_type->primitive_type() != PrimitiveTypeKind::f32 ||
        x.location().start >= y.location().start) {
        fail("ordered struct fields", "field names, types, or order were lost");
    }
}

void test_struct_type_forms() {
    const std::string source =
        "struct Holder { point: Point, matrix: [f32; 2, 2], borrowed: &Point }";
    auto const declaration = parse_declaration(source);
    const auto *structure = dynamic_cast<const StructDecl *>(declaration.get());
    if (!structure || structure->fields().size() != 3) {
        fail("struct type forms", "three fields were not preserved");
        return;
    }

    const auto *point =
        dynamic_cast<const StructType *>(&structure->fields()[0].field_type());
    const auto *matrix =
        dynamic_cast<const ArrayType *>(&structure->fields()[1].field_type());
    const auto *borrowed = dynamic_cast<const ReferenceType *>(
        &structure->fields()[2].field_type());
    if (!point || point->struct_name() != "Point" || !matrix ||
        matrix->dimensions().size() != 2 || !borrowed) {
        fail("struct type forms",
             "named, array, or reference field type was incorrect");
    }
}

void test_declaration_dispatch() {
    auto const function = parse_declaration("fn run() {}");
    auto const structure = parse_declaration("struct Item { value: i32 }");
    if (!dynamic_cast<const FunctionDecl *>(function.get()) ||
        !dynamic_cast<const StructDecl *>(structure.get())) {
        fail("declaration dispatch",
             "top-level declaration kind was incorrect");
    }
    expect_parser_error("invalid top-level declaration", "let value = 1;");
}

void test_invalid_functions() {
    struct InvalidCase {
        std::string_view name;
        std::string source;
    };

    const std::vector<InvalidCase> cases = {
        {"missing function name", "fn () {}"},
        {"missing parameter list", "fn run {}"},
        {"untyped parameter", "fn run(value) {}"},
        {"missing parameter type", "fn run(value:) {}"},
        {"defaulted parameter", "fn run(value: i32 = 1) {}"},
        {"trailing parameter comma", "fn run(value: i32,) {}"},
        {"missing parameter comma", "fn run(left: i32 right: i32) {}"},
        {"missing closing parenthesis", "fn run(value: i32 {}"},
        {"missing return type", "fn run() -> {}"},
        {"missing function body", "fn run()"},
        {"unterminated function body", "fn run() { return;"},
    };
    for (const InvalidCase &test : cases) {
        expect_parser_error(test.name, test.source);
    }
}

void test_invalid_structs() {
    struct InvalidCase {
        std::string_view name;
        std::string source;
    };

    const std::vector<InvalidCase> cases = {
        {"missing struct name", "struct { value: i32 }"},
        {"missing struct brace", "struct Item value: i32 }"},
        {"empty struct", "struct Item {}"},
        {"untyped field", "struct Item { value }"},
        {"missing field type", "struct Item { value: }"},
        {"defaulted field", "struct Item { value: i32 = 1 }"},
        {"missing field comma", "struct Pair { left: i32 right: i32 }"},
        {"field semicolon", "struct Item { value: i32; }"},
        {"unterminated struct", "struct Item { value: i32"},
    };
    for (const InvalidCase &test : cases) {
        expect_parser_error(test.name, test.source);
    }
}
} // namespace

int main() {
    test_empty_function();
    test_function_signature_and_body();
    test_semantic_errors_still_parse();
    test_struct_fields();
    test_struct_type_forms();
    test_declaration_dispatch();
    test_invalid_functions();
    test_invalid_structs();
    if (failures != 0) {
        std::cerr << failures << " declaration parser test(s) failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "All declaration parser tests passed\n";
    return EXIT_SUCCESS;
}
