#pragma once // this is use to prevent inclusion of the same header file
#include "../ast/ast.h"
#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include "errors/parser_error.h"
#include <cstddef>
#include <deque>
#include <memory>
#include <string>

class Parser {
  public:
    Parser(const char *source, std::size_t length);
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Program>
    parse();
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Expr>
    parse_expression();
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Expr>
    parse_binary_expression(int min_precedence);
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Stmt>
    parse_statement();
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Decl>
    parse_declaration();
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Decl>
    parse_struct_declaration();
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Decl>
    parse_function_declaration();
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Type>
    parse_type();
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Expr>
    parse_unary();
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Expr>
    parse_postfix();
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Expr>
    parse_primary();
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Type>
    check_array_type();
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Stmt>
    parse_var_declaration();
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Stmt>
    parse_assignment_statement();
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Stmt>
    parse_return_statement();
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Stmt>
    parse_expression_statement();
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Stmt>
    parse_block_statement();
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Stmt>
    parse_if_statement();
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Stmt>
    parse_while_statement();
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Stmt>
    parse_for_statement();
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Stmt>
    parse_break_statement();
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Stmt>
    parse_continue_statement();
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Type>
    parse_reference_type();
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Type>
    parse_type_or_reference();
    [[nodiscard("you prolly meant to use it")]] std::unique_ptr<Type>
    parse_named_types();

  private:
    inline Token peek(std::size_t distance = 0);
    bool starts_assignment_statement();
    Lexer lexer;
    std::deque<Token> lookahead_;

    /// checks the next token without consuming it.
    /// while parsing `let value = 1;`, it can check whether the next token is
    /// `let`. repeated checks see the same cached token until parsing advances.
    bool check(TokenKind kind) {
        Token const tok = peek();
        return tok.kind == kind;
    }

    /// consumes the next token only when it has the requested kind.
    /// after `let mut value = 1;`, matching `mut` records optional mutability.
    /// a failed match leaves the token untouched so another rule can inspect
    /// it.
    bool match(TokenKind kind) {
        if (check(kind)) {
            advance(); // consume the token.
            return true;
        }
        return false;
    }

    /// consumes the first cached token while preserving later lookahead.
    /// after reading `let` in `let value = 1;`, advancing exposes `value`.
    /// the lexer moves only when the parser asks for the next token.
    void advance() {
        if (lookahead_.empty()) {
            peek();
        }
        lookahead_.pop_front();
    }

    /// consumes a required token or raises a located parser error.
    /// in `let value = 1;`, it can require the final `;`.
    /// callers provide the human-readable spelling used in the diagnostic.
    void expect(TokenKind kind, std::string_view expected_value) {
        if (!match(kind)) {
            ParserError::expected(peek().location, expected_value);
        }
    }
};

/// returns the binding power of a binary operator, or -1 for non-operators.
constexpr int operator_precedence(TokenKind kind) {
    switch (kind) {
    case TokenKind::OP_LOGICAL_OR:
        return 2;
    case TokenKind::OP_LOGICAL_AND:
        return 3;
    case TokenKind::OP_BITWISE_OR:
        return 4;
    case TokenKind::OP_BITWISE_XOR:
        return 5;
    case TokenKind::OP_BITWISE_AND:
        return 6;
    case TokenKind::OP_EQUAL:
    case TokenKind::OP_NOT_EQUAL:
        return 7;
    case TokenKind::OP_LESS:
    case TokenKind::OP_GREATER:
    case TokenKind::OP_LESS_EQUAL:
    case TokenKind::OP_GREATER_EQUAL:
        return 8;
    case TokenKind::OP_LEFT_SHIFT:
    case TokenKind::OP_RIGHT_SHIFT:
        return 9;
    case TokenKind::OP_PLUS:
    case TokenKind::OP_MINUS:
        return 10;
    case TokenKind::OP_MULTIPLY:
    case TokenKind::OP_DIVIDE:
    case TokenKind::OP_MODULO:
        return 11;
    default:
        return -1;
    }
}

/// returns the next token and caches it for stable lookahead.
/// for `value + 1`, repeated peeks return `value` until it is consumed.
/// token text still points into the caller-owned source buffer.
inline Token Parser::peek(std::size_t distance) {
    while (lookahead_.size() <= distance) {
        lookahead_.push_back(lexer.next_token());
    }
    return lookahead_[distance];
}
