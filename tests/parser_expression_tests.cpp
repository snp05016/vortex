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

std::unique_ptr<Stmt> parse_statement(const std::string &source) {
    Parser parser(source.data(), source.size());
    return parser.parse_statement();
}

const Expr *initializer(const std::unique_ptr<Stmt> &statement) {
    const auto *variable = dynamic_cast<const VarDeclStmt *>(statement.get());
    return variable ? &variable->initializer() : nullptr;
}

void expect_parser_error(std::string_view test_name, std::string source) {
    try {
        auto statement = parse_statement(source);
        (void)statement;
        fail(test_name, "source was accepted");
    } catch (const ParserError &) {
    }
}

void test_binary_precedence() {
    auto statement = parse_statement("let value = 2 + 3 * 4;");
    const auto *add = dynamic_cast<const BinaryExpr *>(initializer(statement));
    if (!add || add->op() != BinOp::Add) {
        fail("binary precedence", "root is not addition");
        return;
    }
    const auto *multiply = dynamic_cast<const BinaryExpr *>(&add->right());
    if (!multiply || multiply->op() != BinOp::Multiply) {
        fail("binary precedence", "multiplication did not bind first");
    }
    statement = parse_statement("let value = a << b + c * d;");
    const auto *shift =
        dynamic_cast<const BinaryExpr *>(initializer(statement));
    if (!shift || shift->op() != BinOp::LeftShift) {
        fail("shift precedence", "root is not left shift");
        return;
    }
    const auto *shift_right = dynamic_cast<const BinaryExpr *>(&shift->right());
    if (!shift_right || shift_right->op() != BinOp::Add) {
        fail("shift precedence", "addition did not bind before shift");
    }
    statement = parse_statement("let value = a | b ^ c & d;");
    const auto *bitwise_or =
        dynamic_cast<const BinaryExpr *>(initializer(statement));
    if (!bitwise_or || bitwise_or->op() != BinOp::BitwiseOr) {
        fail("bitwise precedence", "root is not bitwise OR");
        return;
    }
    const auto *bitwise_xor =
        dynamic_cast<const BinaryExpr *>(&bitwise_or->right());
    if (!bitwise_xor || bitwise_xor->op() != BinOp::BitwiseXor) {
        fail("bitwise precedence", "XOR did not bind before OR");
        return;
    }
    const auto *bitwise_and =
        dynamic_cast<const BinaryExpr *>(&bitwise_xor->right());
    if (!bitwise_and || bitwise_and->op() != BinOp::BitwiseAnd) {
        fail("bitwise precedence", "AND did not bind before XOR");
    }
}

void test_binary_operators_and_associativity() {
    struct BinaryCase {
        std::string source;
        BinOp expected;
    };

    const std::vector<BinaryCase> cases = {
        {"let value = a + b;", BinOp::Add},
        {"let value = a - b;", BinOp::Subtract},
        {"let value = a * b;", BinOp::Multiply},
        {"let value = a / b;", BinOp::Divide},
        {"let value = a % b;", BinOp::Modulo},
        {"let value = a == b;", BinOp::Equal},
        {"let value = a != b;", BinOp::NotEqual},
        {"let value = a < b;", BinOp::Less},
        {"let value = a > b;", BinOp::Greater},
        {"let value = a <= b;", BinOp::LessEqual},
        {"let value = a >= b;", BinOp::GreaterEqual},
        {"let value = a && b;", BinOp::LogicalAnd},
        {"let value = a || b;", BinOp::LogicalOr},
        {"let value = a & b;", BinOp::BitwiseAnd},
        {"let value = a | b;", BinOp::BitwiseOr},
        {"let value = a ^ b;", BinOp::BitwiseXor},
        {"let value = a << b;", BinOp::LeftShift},
        {"let value = a >> b;", BinOp::RightShift},
    };
    for (const BinaryCase &test : cases) {
        auto statement = parse_statement(test.source);
        const auto *binary =
            dynamic_cast<const BinaryExpr *>(initializer(statement));
        if (!binary || binary->op() != test.expected) {
            fail("binary operator mapping", test.source);
        }
    }
    auto statement = parse_statement("let value = a - b - c;");
    const auto *outer =
        dynamic_cast<const BinaryExpr *>(initializer(statement));
    const auto *inner =
        outer ? dynamic_cast<const BinaryExpr *>(&outer->left()) : nullptr;
    if (!outer || outer->op() != BinOp::Subtract || !inner ||
        inner->op() != BinOp::Subtract) {
        fail("binary associativity", "subtraction is not left associative");
    }
}

void test_postfix_chaining() {
    auto statement =
        parse_statement("let value = factory().items[row, column].value;");
    const auto *value =
        dynamic_cast<const FieldAccessExpr *>(initializer(statement));
    if (!value || value->field_name() != "value") {
        fail("postfix chaining", "outer field access is missing");
        return;
    }
    const auto *index = dynamic_cast<const IndexExpr *>(&value->object());
    if (!index || index->indices().size() != 2) {
        fail("postfix chaining", "two-dimensional index is missing");
        return;
    }
    const auto *items =
        dynamic_cast<const FieldAccessExpr *>(&index->collection());
    if (!items || items->field_name() != "items") {
        fail("postfix chaining", "items field access is missing");
        return;
    }
    const auto *call = dynamic_cast<const CallCastExpr *>(&items->object());
    if (!call || !call->arguments().empty()) {
        fail("postfix chaining", "zero-argument call is missing");
    }
    statement = parse_statement("let value = add(1, 2 * 3);");
    call = dynamic_cast<const CallCastExpr *>(initializer(statement));
    if (!call || call->arguments().size() != 2) {
        fail("call arguments", "call arguments were not preserved");
    } else {
        const auto *multiply =
            dynamic_cast<const BinaryExpr *>(call->arguments()[1].get());
        if (!multiply || multiply->op() != BinOp::Multiply) {
            fail("call arguments", "argument expression was not parsed");
        }
    }
}

void test_ranges() {
    auto statement = parse_statement("let values = 0..=10;");
    const auto *range = dynamic_cast<const RangeExpr *>(initializer(statement));
    if (!range || !range->is_inclusive()) {
        fail("inclusive range", "inclusive range node is missing");
    }
    expect_parser_error("chained range", "let values = 0..10..20;");
}

void test_array_and_struct_edges() {
    auto statement = parse_statement("let values = [1, 2,];");
    const auto *array = dynamic_cast<const ArrayExpr *>(initializer(statement));
    if (!array || array->elements().size() != 2) {
        fail("array trailing comma", "array elements were not preserved");
    }
    statement = parse_statement("let point = Point { x: 1, y: 2, };");
    const auto *structure =
        dynamic_cast<const StructConstructionExpr *>(initializer(statement));
    if (!structure || structure->fields().size() != 2) {
        fail("struct trailing comma", "struct fields were not preserved");
    }
    expect_parser_error("empty struct expression", "let point = Point {};");
    expect_parser_error("repeat trailing comma", "let values = [0; 4,];");
}

void test_non_chaining_and_trailing_commas() {
    expect_parser_error("chained comparison", "let value = a < b < c;");
    expect_parser_error("mixed comparison", "let value = a == b < c;");
    expect_parser_error("call trailing comma", "let value = add(1,);");
    expect_parser_error("index trailing comma", "let value = values[0,];");
    expect_parser_error("unsupported escape", R"(let value = "bad\q";)");
}
} // namespace

int main() {
    test_binary_precedence();
    test_binary_operators_and_associativity();
    test_postfix_chaining();
    test_ranges();
    test_array_and_struct_edges();
    test_non_chaining_and_trailing_commas();
    if (failures != 0) {
        std::cerr << failures << " expression parser test(s) failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "All expression parser tests passed\n";
    return EXIT_SUCCESS;
}
