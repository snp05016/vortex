#include "parser.h"

#include <iostream>

namespace {

/// maps a binary operator token to the matching ast operation.
/// for `left + right`, it turns `+` into the addition operation.
/// it returns no value when the token is not a binary operator.
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

/// tells whether an ast operation is equality or comparison.
/// for `left <= right`, it recognizes the less-than-or-equal operation.
/// this distinction prevents unparenthesized comparison chains.
bool is_equality_or_comparison(BinOp op) {
    return op == BinOp::Equal || op == BinOp::NotEqual || op == BinOp::Less ||
           op == BinOp::Greater || op == BinOp::LessEqual ||
           op == BinOp::GreaterEqual;
}

/// checks whether an expression directly contains equality or comparison.
/// for `left < right`, it returns true, while `(left < right)` is a group.
/// grouped expressions stay distinguishable so explicit parentheses are respected.
bool is_unparenthesized_equality_or_comparison(const Expr &expression) {
    const auto *binary = dynamic_cast<const BinaryExpr *>(&expression);
    return binary && is_equality_or_comparison(binary->op());
}

} // namespace

/// creates a parser over source text supplied by the caller.
/// for `let count = 1;`, parsing begins at the `let` token.
/// the source buffer must stay alive for as long as the parser uses it.
Parser::Parser(const char *source, std::size_t length) : lexer(source, length) {
}

/// reads and prints every token until the source reaches its end.
/// for `let count = 1;`, it prints the declaration tokens in order.
/// this is currently a debugging entry point and does not build a program ast.
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

/// parses one complete expression, including an optional range.
/// for `start..=end`, it builds an inclusive range expression.
/// ordinary operators are parsed first because ranges have the lowest precedence.
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
        ParserError::invalid(peek().location, "chained range expression");
    }
    return std::make_unique<RangeExpr>(
        range_operator.location, std::move(start), std::move(end),
        range_operator.kind == TokenKind::PUNC_INCL_RANGE);
}

/// parses binary operators at or above the requested precedence.
/// for `2 + 3 * 4`, multiplication becomes the right child of addition.
/// equality and comparison chains are rejected unless parentheses separate them.
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
            ParserError::invalid(operator_token.location,
                                   "chained comparison or equality expression");
        }
        advance();
        auto right = parse_binary_expression(precedence->second + 1);
        if (is_equality_or_comparison(*op) &&
            is_unparenthesized_equality_or_comparison(*right)) {
            ParserError::invalid(operator_token.location,
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
