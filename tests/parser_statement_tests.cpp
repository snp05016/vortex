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

void expect_parser_error(std::string_view test_name,
                         const std::string &source) {
    try {
        auto const statement = parse_statement(source);
        (void)statement;
        fail(test_name, "source was accepted");
    } catch (const ParserError &) {
    }
}

void test_assignment_dispatch() {
    struct AssignmentCase {
        std::string source;
        AssignmentOperation expected;
    };

    const std::vector<AssignmentCase> cases = {
        {"count = 1;", AssignmentOperation::Assign},
        {"count += 1;", AssignmentOperation::AddAssign},
        {"count -= 1;", AssignmentOperation::SubtractAssign},
        {"count *= 2;", AssignmentOperation::MultiplyAssign},
        {"count /= 2;", AssignmentOperation::DivideAssign},
        {"count %= 2;", AssignmentOperation::RemainderAssign},
    };
    for (const AssignmentCase &test : cases) {
        auto const statement = parse_statement(test.source);
        const auto *assignment =
            dynamic_cast<const AssignmentStatement *>(statement.get());
        if (!assignment || assignment->op() != test.expected ||
            !dynamic_cast<const Identifier *>(&assignment->lhs())) {
            fail("assignment operator", test.source);
        }
    }

    auto statement = parse_statement("points[index].x = 0.0;");
    const auto *assignment =
        dynamic_cast<const AssignmentStatement *>(statement.get());
    const auto *field =
        assignment ? dynamic_cast<const FieldAccessExpr *>(&assignment->lhs())
                   : nullptr;
    const auto *index =
        field ? dynamic_cast<const IndexExpr *>(&field->object()) : nullptr;
    if (!field || field->field_name() != "x" || !index) {
        fail("assignment target chain",
             "indexed field target was not preserved");
    }

    statement = parse_statement("matrix[row, column] *= scale;");
    assignment = dynamic_cast<const AssignmentStatement *>(statement.get());
    index = assignment ? dynamic_cast<const IndexExpr *>(&assignment->lhs())
                       : nullptr;
    if (!index || index->indices().size() != 2 ||
        assignment->op() != AssignmentOperation::MultiplyAssign) {
        fail("multidimensional assignment target",
             "indices or compound operator were not preserved");
    }
}

void test_expression_dispatch() {
    auto const statement = parse_statement("  run();");
    const auto *expression = dynamic_cast<const ExprStmt *>(statement.get());
    if (!expression || expression->location().start != 2) {
        fail("expression dispatch",
             "call was not parsed as an expression statement");
    }
}

void test_return_statements() {
    auto statement = parse_statement("  return;");
    const auto *empty_return = dynamic_cast<const RetStmt *>(statement.get());
    if (!empty_return || empty_return->return_value() != nullptr ||
        empty_return->location().start != 2) {
        fail("empty return", "empty return or its location was incorrect");
    }

    statement = parse_statement("return value;");
    const auto *value_return = dynamic_cast<const RetStmt *>(statement.get());
    if (!value_return || value_return->return_value() == nullptr) {
        fail("value return", "return value was not preserved");
    }
}

void test_while_statement() {
    auto const statement = parse_statement("while ready { break; }");
    const auto *loop = dynamic_cast<const WhileStmt *>(statement.get());
    const auto *body =
        loop ? dynamic_cast<const BlockStmt *>(&loop->body()) : nullptr;
    if (!body || body->statements().size() != 1 ||
        !dynamic_cast<const BreakStmt *>(body->statements()[0].get())) {
        fail("while statement", "condition or body was not preserved");
    }
}

void test_for_statement() {
    auto const statement = parse_statement("for index in 0..10 { continue; }");
    const auto *loop = dynamic_cast<const ForStmt *>(statement.get());
    const auto *body =
        loop ? dynamic_cast<const BlockStmt *>(&loop->body()) : nullptr;
    if (!loop || loop->var_name() != "index" ||
        !dynamic_cast<const RangeExpr *>(&loop->iterable()) || !body ||
        body->statements().size() != 1 ||
        !dynamic_cast<const ContinueStmt *>(body->statements()[0].get())) {
        fail("for statement", "variable, iterable, or body was not preserved");
    }
}

void test_required_loop_blocks() {
    expect_parser_error("unbraced while", "while ready break;");
    expect_parser_error("unbraced for", "for index in 0..10 continue;");
}

void test_invalid_statements() {
    expect_parser_error("missing variable initializer", "let value;");
    expect_parser_error("misplaced mut", "let value mut = 1;");
    expect_parser_error("missing expression semicolon", "run()");
    expect_parser_error("binary assignment target", "(left + right) = 4;");
    expect_parser_error("call assignment target", "function() = 4;");
    expect_parser_error("missing return semicolon", "return value");
    expect_parser_error("unbraced if", "if ready run();");
    expect_parser_error("unbraced else", "if ready {} else run();");
    expect_parser_error("break with value", "break value;");
    expect_parser_error("continue with value", "continue 2;");
}

void test_if_without_else() {
    auto const statement = parse_statement("if ready {}");
    const auto *if_statement = dynamic_cast<const IfStmt *>(statement.get());
    if (!if_statement) {
        fail("if without else", "statement was not parsed as an if");
        return;
    }
    if (!dynamic_cast<const BlockStmt *>(&if_statement->then_branch())) {
        fail("if without else", "then branch was not parsed as a block");
    }
    if (if_statement->else_branch() != nullptr) {
        fail("if without else", "an absent else branch was not null");
    }
}

void test_if_else() {
    auto const statement = parse_statement("if ready {} else {}");
    const auto *if_statement = dynamic_cast<const IfStmt *>(statement.get());
    if (!if_statement ||
        !dynamic_cast<const BlockStmt *>(if_statement->else_branch())) {
        fail("if else", "else branch was not parsed as a block");
    }
}

void test_then_block_statements() {
    auto const statement =
        parse_statement("if ready { let value = 1; run(); }");
    const auto *if_statement = dynamic_cast<const IfStmt *>(statement.get());
    const auto *block =
        if_statement
            ? dynamic_cast<const BlockStmt *>(&if_statement->then_branch())
            : nullptr;
    if (!block || block->statements().size() != 2) {
        fail("then block statements",
             "the block did not preserve both statements");
    }
}

void test_else_if_chain() {
    auto const statement =
        parse_statement("if first {} else if second {} else {}");
    const auto *outer = dynamic_cast<const IfStmt *>(statement.get());
    const auto *nested =
        outer ? dynamic_cast<const IfStmt *>(outer->else_branch()) : nullptr;
    if (!nested) {
        fail("else if chain", "else-if branch was not parsed as a nested if");
        return;
    }
    if (!dynamic_cast<const BlockStmt *>(nested->else_branch())) {
        fail("else if chain", "final else branch was not parsed as a block");
    }
}

void test_parenthesized_condition() {
    auto const statement = parse_statement("if (ready) {}");
    const auto *if_statement = dynamic_cast<const IfStmt *>(statement.get());
    if (!if_statement ||
        !dynamic_cast<const Grp *>(&if_statement->condition())) {
        fail("parenthesized condition",
             "grouped condition was not preserved as an expression");
    }
}
} // namespace

int main() {
    test_assignment_dispatch();
    test_expression_dispatch();
    test_return_statements();
    test_if_without_else();
    test_if_else();
    test_then_block_statements();
    test_else_if_chain();
    test_parenthesized_condition();
    test_while_statement();
    test_for_statement();
    test_required_loop_blocks();
    test_invalid_statements();
    if (failures != 0) {
        std::cerr << failures << " statement parser test(s) failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "All statement parser tests passed\n";
    return EXIT_SUCCESS;
}
