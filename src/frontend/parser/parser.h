#pragma once // this is use to prevent inclusion of the same header file
#include "../ast.h"
#include "../lexer.h"
#include "../token.h"
#include "errors/parser_error.h"
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

class Parser {
  public:
    Parser(const char *source,
           std::size_t length); // constructs a parser for a source buffer.
    void parse();               // prints the tokens in the source buffer.
    [[nodiscard("you prolly meant to use it")]]std::optional<Token> lookahead_;
    [[nodiscard("you prolly meant to use it")]]std::unique_ptr<Expr> parse_expression();
    [[nodiscard("you prolly meant to use it")]]std::unique_ptr<Expr> parse_binary_expression(
  int min_precedence); // parses an expressi)on at a precedence floor.
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
    private:
    inline Token peek(); // returns the next token without consuming it.
    Lexer lexer;

    // this function checks if the next token is of the specified kind without consuming it.
    // used for lookahead and conditional parsing.
    bool check(TokenKind kind) {
        Token tok = peek();
        return tok.kind == kind;
    }

    bool match(TokenKind kind) {
        if (check(kind)) {
            advance(); // consume the token.
            return true;
        }
        return false;
    }

    void advance() {
        lookahead_.reset();
    } // consumes the current token.

    void expect(TokenKind kind, std::string_view expected_value) {
        if (!match(kind)) {
            parser_errors::expected(peek().location, expected_value);
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

inline Token Parser::peek() {
    if (!lookahead_) {
        lookahead_ = lexer.next_token();
    }
    return *lookahead_;
}
