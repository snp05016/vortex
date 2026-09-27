#pragma once // this is use to prevent inclusion of the same header file
#include "../ast.h"
#include "../lexer.h"
#include "../token.h"
#include "errors/parser_error.h"
#include <cstddef>
#include <deque>
#include <memory>
#include <string>
#include <unordered_map>

class Parser {
  public:
    Parser(const char *source, std::size_t length);
    void parse();
    [[nodiscard("you prolly meant to use it")]]std::unique_ptr<Expr> parse_expression();
    [[nodiscard("you prolly meant to use it")]]std::unique_ptr<Expr> parse_binary_expression(
  int min_precedence);
    [[nodiscard("you prolly meant to use it")]]std::unique_ptr<Stmt> parse_statement();
    [[nodiscard("you prolly meant to use it")]]std::unique_ptr<Decl> parse_declaration();
    [[nodiscard("you prolly meant to use it")]]std::unique_ptr<Decl> parse_struct_declaration();
    [[nodiscard("you prolly meant to use it")]]std::unique_ptr<Decl> parse_function_declaration();
    [[nodiscard("you prolly meant to use it")]]std::unique_ptr<Type> parse_type();
    [[nodiscard("you prolly meant to use it")]]std::unique_ptr<Expr> parse_unary();
    [[nodiscard("you prolly meant to use it")]]std::unique_ptr<Expr> parse_postfix();
    [[nodiscard("you prolly meant to use it")]]std::unique_ptr<Expr> parse_primary();
    [[nodiscard("you prolly meant to use it")]]std::unique_ptr<Type> check_array_type();
    [[nodiscard("you prolly meant to use it")]]std::unique_ptr<Stmt> parse_var_declaration();
    [[nodiscard("you prolly meant to use it")]]std::unique_ptr<Stmt> parse_assignment_statement();
    [[nodiscard("you prolly meant to use it")]]std::unique_ptr<Stmt> parse_return_statement();
    [[nodiscard("you prolly meant to use it")]]std::unique_ptr<Stmt> parse_expression_statement();
    [[nodiscard("you prolly meant to use it")]]std::unique_ptr<Stmt> parse_block_statement();
    [[nodiscard("you prolly meant to use it")]]std::unique_ptr<Stmt> parse_if_statement();
    [[nodiscard("you prolly meant to use it")]]std::unique_ptr<Stmt> parse_while_statement();
    [[nodiscard("you prolly meant to use it")]]std::unique_ptr<Stmt> parse_for_statement();
    [[nodiscard("you prolly meant to use it")]]std::unique_ptr<Stmt> parse_break_statement();
    [[nodiscard("you prolly meant to use it")]]std::unique_ptr<Stmt> parse_continue_statement();
    private:
    inline Token peek(std::size_t distance = 0);
    bool starts_assignment_statement();
    Lexer lexer;
    std::deque<Token> lookahead_;

    /// checks the next token without consuming it.
    /// while parsing `let value = 1;`, it can check whether the next token is `let`.
    /// repeated checks see the same cached token until parsing advances.
    bool check(TokenKind kind) {
        Token tok = peek();
        return tok.kind == kind;
    }

    /// consumes the next token only when it has the requested kind.
    /// after `let mut value = 1;`, matching `mut` records optional mutability.
    /// a failed match leaves the token untouched so another rule can inspect it.
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

static std::unordered_map<TokenKind, int> operator_precedence = {
    {TokenKind::OP_LOGICAL_OR, 2},  {TokenKind::OP_LOGICAL_AND, 3},
    {TokenKind::OP_BITWISE_OR, 4},  {TokenKind::OP_BITWISE_XOR, 5},
    {TokenKind::OP_BITWISE_AND, 6}, {TokenKind::OP_EQUAL, 7},
    {TokenKind::OP_NOT_EQUAL, 7},   {TokenKind::OP_LESS, 8},
    {TokenKind::OP_GREATER, 8},     {TokenKind::OP_LESS_EQUAL, 8},
    {TokenKind::OP_GREATER_EQUAL, 8},
    {TokenKind::OP_LEFT_SHIFT, 9},  {TokenKind::OP_RIGHT_SHIFT, 9},
    {TokenKind::OP_PLUS, 10},       {TokenKind::OP_MINUS, 10},
    {TokenKind::OP_MULTIPLY, 11},   {TokenKind::OP_DIVIDE, 11},
    {TokenKind::OP_MODULO, 11},
};

/// returns the next token and caches it for stable lookahead.
/// for `value + 1`, repeated peeks return `value` until it is consumed.
/// token text still points into the caller-owned source buffer.
inline Token Parser::peek(std::size_t distance) {
    while (lookahead_.size() <= distance) {
        lookahead_.push_back(lexer.next_token());
    }
    return lookahead_[distance];
}
