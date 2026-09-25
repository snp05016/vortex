#include "parser.h"
#include <stdexcept>
#include <utility>
#include <vector>
std::unique_ptr<Decl> Parser::parse_declaration() {
    Token tok = peek();
    switch (tok.kind) {
    case TokenKind::KW_FN:
        return parse_function_declaration();
    case TokenKind::KW_STRUCT:
        return parse_struct_declaration();
    default:
        throw std::runtime_error("Expected a declaration");
    }
}

// fn add(x:i32,y:i64) -> i32 { return x + y; }
std::unique_ptr<Decl> Parser::parse_function_declaration() {
    Token function_token = peek();
    if (function_token.kind != TokenKind::KW_FN) {
        throw std::runtime_error("Expected a function declaration");
    }

    advance(); // consume "fn".
    Token name_token = peek();
    if (name_token.kind != TokenKind::IDENTIFIER) {
        throw std::runtime_error("Expected function name after 'fn'");
    }
    std::string function_name = name_token.current_token_string();
    advance(); // consume the function name.
    if (!match(TokenKind::PUNC_LPAREN)) {
        throw std::runtime_error("Expected '(' after function name");
    }
    std::vector<ParamDecl> parameters;
    if (!check(TokenKind::PUNC_RPAREN)) {
        while (true) {
            Token parameter_token = peek();
            if (parameter_token.kind != TokenKind::IDENTIFIER) {
                throw std::runtime_error(
                    "Expected parameter name in function declaration");
            }
            std::string parameter_name = parameter_token.current_token_string();
            advance(); // consume the parameter name.
            if (!match(TokenKind::PUNC_COLON)) {
                throw std::runtime_error("Expected ':' after parameter name");
            }
            auto parameter_type = parse_type();
            if (!parameter_type) {
                throw std::runtime_error(
                    "Expected type after ':' in parameter declaration");
            }
            parameters.emplace_back(parameter_token.location, parameter_name,
                                    std::move(parameter_type));
            if (!match(TokenKind::PUNC_COMMA)) {
                break;
            }
            if (check(TokenKind::PUNC_RPAREN)) {
                throw std::runtime_error(
                    "Trailing comma is not allowed in a parameter list");
            }
        }
    }
    if (!match(TokenKind::PUNC_RPAREN)) {
        throw std::runtime_error("Expected ')' after parameter list");
    }
    std::unique_ptr<Type> return_type;
    if (match(TokenKind::PUNC_ARROW)) {
        return_type = parse_type();
        if (!return_type) {
            throw std::runtime_error("Expected return type after '->'");
        }
    } else {
        return_type = std::make_unique<PrimitiveType>(function_token.location, // if no return type is specified, defaulting to void
                                                      PrimitiveTypeKind::Void);
    }
    Token left_brace = peek();
    if (!match(TokenKind::PUNC_LBRACE)) {
        throw std::runtime_error("Expected '{' before function body");
    }
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!check(TokenKind::PUNC_RBRACE)) {
        if (check(TokenKind::EOF_TOKEN)) {
            throw std::runtime_error("Expected '}' after function body");
        }
        auto statement = parse_statement();
        if (!statement) {
            throw std::runtime_error("Expected statement in function body");
        }
        statements.push_back(std::move(statement));
    }
    advance(); // consume "}".
    auto body =
        std::make_unique<BlockStmt>(left_brace.location, std::move(statements));
    return std::make_unique<FunctionDecl>(
        function_token.location, std::move(function_name),
        std::move(parameters), std::move(return_type), std::move(body));
}
std::unique_ptr<Decl> Parser::parse_struct_declaration() {
    Token struct_token = peek();
    auto struct_token_location = struct_token.location;
    if (struct_token.kind != TokenKind::KW_STRUCT) {
        throw std::runtime_error("Expected a struct declaration");
    }
    advance(); // consume "struct".
    Token name_token = peek();
    if (name_token.kind != TokenKind::IDENTIFIER) {
        throw std::runtime_error("Expected struct name after 'struct'");
    }
    std::string struct_name = name_token.current_token_string();
    advance(); // consume the struct name.
    if (!match(TokenKind::PUNC_LBRACE)) {
        throw std::runtime_error("Expected '{' after struct name");
    }
    std::vector<StructFieldDecl> fields;
    while (!check(TokenKind::PUNC_RBRACE)) {
        if (check(TokenKind::EOF_TOKEN)) {
            throw std::runtime_error("expected } after struct definitions");
        }
        Token curr_struct_field = peek();
        if (curr_struct_field.kind != TokenKind::IDENTIFIER) {
            throw std::runtime_error("expected a struct field name");
        }
        std::string field_name = curr_struct_field.current_token_string();
        advance();
        if (!match(TokenKind::PUNC_COLON)) {
            throw std::runtime_error("expected ':' after struct field name");
        }
        auto field_type = parse_type();
        if (!field_type) {
            throw std::runtime_error("expected a type after struct field name");
        }
        fields.emplace_back(curr_struct_field.location, std::move(field_name),
                            std::move(field_type));
        if (!match(TokenKind::PUNC_COMMA) &&
            !check(TokenKind::PUNC_RBRACE)) {
            throw std::runtime_error("expected ',' or '}' after struct field");
        }
    }
    advance(); // consume "}".
    if (fields.empty()) {
        throw std::runtime_error("structs cannot have zero fields");
    }
    return std::make_unique<StructDecl>(struct_token_location,
                                        std::move(struct_name),
                                        std::move(fields));
}
