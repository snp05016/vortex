#include "parser.h"

#include <iostream>

namespace {

std::optional<BinOp> binary_operator(TokenKind kind) {
    switch (kind) {
    case TokenKind::OP_PLUS:
        return BinOp::Add;
    case TokenKind::OP_MINUS:
        return BinOp::Subtract;
    case TokenKind::OP_MULTIPLY:
        return BinOp::Multiply;
    case TokenKind::OP_DIVIDE:
        return BinOp::Divide;
    case TokenKind::OP_MODULO:
        return BinOp::Modulo;
    case TokenKind::OP_EQUAL:
        return BinOp::Equal;
    case TokenKind::OP_NOT_EQUAL:
        return BinOp::NotEqual;
    case TokenKind::OP_LESS:
        return BinOp::Less;
    case TokenKind::OP_GREATER:
        return BinOp::Greater;
    case TokenKind::OP_LESS_EQUAL:
        return BinOp::LessEqual;
    case TokenKind::OP_GREATER_EQUAL:
        return BinOp::GreaterEqual;
    case TokenKind::OP_LOGICAL_AND:
        return BinOp::LogicalAnd;
    case TokenKind::OP_LOGICAL_OR:
        return BinOp::LogicalOr;
    case TokenKind::OP_BITWISE_AND:
        return BinOp::BitwiseAnd;
    case TokenKind::OP_BITWISE_OR:
        return BinOp::BitwiseOr;
    case TokenKind::OP_BITWISE_XOR:
        return BinOp::BitwiseXor;
    case TokenKind::OP_LEFT_SHIFT:
        return BinOp::LeftShift;
    case TokenKind::OP_RIGHT_SHIFT:
        return BinOp::RightShift;
    default:
        return std::nullopt;
    }
}

bool is_equality_or_comparison(BinOp op) {
    return op == BinOp::Equal || op == BinOp::NotEqual || op == BinOp::Less ||
           op == BinOp::Greater || op == BinOp::LessEqual ||
           op == BinOp::GreaterEqual;
}

bool is_unparenthesized_equality_or_comparison(const Expr &expression) {
    const auto *binary = dynamic_cast<const BinaryExpr *>(&expression);
    return binary && is_equality_or_comparison(binary->op());
}

} // namespace

// constructs a parser for a source buffer.
Parser::Parser(const char *source, std::size_t length) : lexer(source, length) {
}

// prints each token until the lexer reaches the end of the source.
void Parser::parse() {
    while (true) {
        Token token = lexer.next_token();
        if (token.kind == TokenKind::EOF_TOKEN) {
            break;
        }

        // statement and expression dispatch will be added here later.
        std::cout << "Token kind: " << token.current_token_string()
                  << ", Location: " << token.location.start << "-"
                  << (token.location.start + token.location.length)
                  << std::endl;
    }
}

// parses an expression through the binary-expression entry point.
std::unique_ptr<Expr> Parser::parse_expression() {
    auto start = parse_binary_expression(2);
    Token range_operator = peek();
    if (range_operator.kind != TokenKind::PUNC_RANGE &&
        range_operator.kind != TokenKind::PUNC_INCL_RANGE) {
        return start;
    }
    advance();
    auto end = parse_binary_expression(2);
    if (check(TokenKind::PUNC_RANGE) || check(TokenKind::PUNC_INCL_RANGE)) {
        parser_errors::invalid(peek().location, "chained range expression");
    }
    return std::make_unique<RangeExpr>(
        range_operator.location, std::move(start), std::move(end),
        range_operator.kind == TokenKind::PUNC_INCL_RANGE);
}

// parses the current expression with the requested precedence floor.
std::unique_ptr<Expr> Parser::parse_binary_expression(int min_precedence) {
    auto left = parse_unary();
    bool saw_equality_or_comparison = false;
    while (true) {
        Token operator_token = peek();
        auto precedence = operator_precedence.find(operator_token.kind);
        if (precedence == operator_precedence.end() ||
            precedence->second < min_precedence) {
            break;
        }
        auto op = binary_operator(operator_token.kind);
        if (!op) {
            break;
        }
        if (is_equality_or_comparison(*op) && saw_equality_or_comparison) {
            parser_errors::invalid(operator_token.location,
                                   "chained comparison or equality expression");
        }
        advance();
        auto right = parse_binary_expression(precedence->second + 1);
        if (is_equality_or_comparison(*op) &&
            is_unparenthesized_equality_or_comparison(*right)) {
            parser_errors::invalid(operator_token.location,
                                   "chained comparison or equality expression");
        }
        if (is_equality_or_comparison(*op)) {
            saw_equality_or_comparison = true;
        }
        left = std::make_unique<BinaryExpr>(operator_token.location, *op,
                                            std::move(left), std::move(right));
    }
    return left;
}
