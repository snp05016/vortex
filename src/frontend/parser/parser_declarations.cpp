#include "parser.h"
#include <stdexcept>
#include <utility>
#include <vector>
/// chooses the correct parser for a top-level declaration.
/// for `fn main() {}`, it delegates to the function declaration parser.
/// only functions and structs are valid top-level declarations in v0.1.
std::unique_ptr<Decl> Parser::parse_declaration() {
    Token tok = peek();
    switch (tok.kind) {
    case TokenKind::KW_FN:
        return parse_function_declaration();
    case TokenKind::KW_STRUCT:
        return parse_struct_declaration();
    default:
        ParserError::expected(tok.location, "declaration");
    }
}

/// parses a function name, parameters, return type, and body.
/// for `fn add(left: i32, right: i32) -> i32 {}`, it keeps the signature in order.
/// an omitted return type becomes `void`, while trailing parameter commas are rejected.
std::unique_ptr<Decl> Parser::parse_function_declaration() {
    Token function_token = peek();
    if (function_token.kind != TokenKind::KW_FN) {
        ParserError::expected(function_token.location,
                                "function declaration");
    }
 
    advance(); // consume "fn".
    Token name_token = peek();
    if (name_token.kind != TokenKind::IDENTIFIER) {
        ParserError::expected(name_token.location, "function name",
                                "after 'fn'");
    }
    std::string function_name = name_token.current_token_string();
    advance(); // consume the function name.
    if (!match(TokenKind::PUNC_LPAREN)) {
        ParserError::expected(peek().location, "'('",
                                "after function name");
    }
    std::vector<ParamDecl> parameters;
    if (!check(TokenKind::PUNC_RPAREN)) {
        while (true) {
            Token parameter_token = peek();
            if (parameter_token.kind != TokenKind::IDENTIFIER) {
                ParserError::expected(parameter_token.location,
                                        "parameter name",
                                        "in function declaration");
            }
            std::string parameter_name = parameter_token.current_token_string();
            advance(); // consume the parameter name.
            if (!match(TokenKind::PUNC_COLON)) {
                ParserError::expected(peek().location, "':'",
                                        "after parameter name");
            }
            auto parameter_type = parse_type();
            if (!parameter_type) {
                ParserError::expected(peek().location, "type",
                                        "after ':' in parameter declaration");
            }
            parameters.emplace_back(parameter_token.location, parameter_name,
                                    std::move(parameter_type));
            if (!match(TokenKind::PUNC_COMMA)) {
                break;
            }
            if (check(TokenKind::PUNC_RPAREN)) {
                ParserError::invalid(peek().location,
                                       "trailing comma in parameter list");
            }
        }
    }
    if (!match(TokenKind::PUNC_RPAREN)) {
        ParserError::expected(peek().location, "')'",
                                "after parameter list");
    }
    std::unique_ptr<Type> return_type;
    if (match(TokenKind::PUNC_ARROW)) {
        return_type = parse_type();
        if (!return_type) {
            ParserError::expected(peek().location, "return type",
                                    "after '->'");
        }
    } else {
        return_type = std::make_unique<PrimitiveType>(function_token.location, // if no return type is specified, defaulting to void
                                                      PrimitiveTypeKind::Void);
    }
    Token left_brace = peek();
    if (!match(TokenKind::PUNC_LBRACE)) {
        ParserError::expected(left_brace.location, "'{'",
                                "before function body");
    }
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!check(TokenKind::PUNC_RBRACE)) {
        if (check(TokenKind::EOF_TOKEN)) {
            ParserError::expected(peek().location, "'}'",
                                    "after function body");
        }
        auto statement = parse_statement();
        if (!statement) {
            ParserError::expected(peek().location, "statement",
                                    "in function body");
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

/// parses a struct name and its ordered field declarations.
/// for `struct point { x: f32, y: f32 }`, it records both fields in source order.
/// structs need at least one field, and a final field comma is allowed.
std::unique_ptr<Decl> Parser::parse_struct_declaration() {
    Token struct_token = peek();
    auto struct_token_location = struct_token.location;
    if (struct_token.kind != TokenKind::KW_STRUCT) {
        ParserError::expected(struct_token.location, "struct declaration");
    }
    advance(); // consume "struct".
    Token name_token = peek();
    if (name_token.kind != TokenKind::IDENTIFIER) {
        ParserError::expected(name_token.location, "struct name",
                                "after 'struct'");
    }
    std::string struct_name = name_token.current_token_string();
    advance(); // consume the struct name.
    if (!match(TokenKind::PUNC_LBRACE)) {
        ParserError::expected(peek().location, "'{'", "after struct name");
    }
    std::vector<StructFieldDecl> fields;
    while (!check(TokenKind::PUNC_RBRACE)) {
        if (check(TokenKind::EOF_TOKEN)) {
            ParserError::expected(peek().location, "'}'",
                                    "after struct fields");
        }
        Token curr_struct_field = peek();
        if (curr_struct_field.kind != TokenKind::IDENTIFIER) {
            ParserError::expected(curr_struct_field.location,
                                    "struct field name");
        }
        std::string field_name = curr_struct_field.current_token_string();
        advance();
        if (!match(TokenKind::PUNC_COLON)) {
            ParserError::expected(peek().location, "':'",
                                    "after struct field name");
        }
        auto field_type = parse_type();
        if (!field_type) {
            ParserError::expected(peek().location, "type",
                                    "after struct field name");
        }
        fields.emplace_back(curr_struct_field.location, std::move(field_name),
                            std::move(field_type));
        if (!match(TokenKind::PUNC_COMMA) &&
            !check(TokenKind::PUNC_RBRACE)) {
            ParserError::expected(peek().location, "',' or '}'",
                                    "after struct field");
        }
    }
    advance(); // consume "}".
    if (fields.empty()) {
        ParserError::invalid(struct_token_location,
                               "struct with zero fields");
    }
    return std::make_unique<StructDecl>(struct_token_location,
                                        std::move(struct_name),
                                        std::move(fields));
}
