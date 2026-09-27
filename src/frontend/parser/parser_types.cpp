#include "parser.h"

/// parses a primitive type or recursively nested array type.
/// for `[[i32; 2]; 3]`, it builds an outer array whose element is another
/// array. named and reference types are not handled here yet.
std::unique_ptr<Type> Parser::parse_type() {
    Token type_token = peek();
    if (type_token.kind == TokenKind::PUNC_LBRACKET) {
        return check_array_type();
    }
    PrimitiveTypeKind kind;
    switch (type_token.kind) {
    case TokenKind::KW_VOID:
        kind = PrimitiveTypeKind::Void;
        break;
    case TokenKind::KW_BOOL:
        kind = PrimitiveTypeKind::Bool;
        break;
    case TokenKind::KW_CHAR:
        kind = PrimitiveTypeKind::Char;
        break;
    case TokenKind::KW_I32:
        kind = PrimitiveTypeKind::i32;
        break;
    case TokenKind::KW_U32:
        kind = PrimitiveTypeKind::u32;
        break;
    case TokenKind::KW_USIZE:
        kind = PrimitiveTypeKind::usize;
        break;
    case TokenKind::KW_F32:
        kind = PrimitiveTypeKind::f32;
        break;
    case TokenKind::KW_F64:
        kind = PrimitiveTypeKind::f64;
        break;
    case TokenKind::KW_STRING:
        kind = PrimitiveTypeKind::String;
        break;
    default:
        return nullptr;
    }

    advance();
    return std::make_unique<PrimitiveType>(type_token.location, kind);
}

/// parses an array element type followed by one or more dimensions.
/// for `[f32; 2 + 2, 8]`, it keeps both dimension expressions in source order.
/// missing dimensions and trailing dimension commas are syntax errors.
std::unique_ptr<Type> Parser::check_array_type() {
    Token type_token = peek();
    if (type_token.kind != TokenKind::PUNC_LBRACKET) {
        return nullptr;
    }
    advance(); // consume '['.

    auto element_type = parse_type();
    if (!element_type) {
        ParserError::expected(peek().location, "type", "after '['");
    }
    if (!match(TokenKind::PUNC_SEMICOLON)) {
        ParserError::expected(peek().location, "';'",
                              "after array element type");
    }
    if (check(TokenKind::PUNC_RBRACKET)) {
        ParserError::expected(peek().location, "array dimension");
    }
    std::vector<std::unique_ptr<Expr>> dimensions;
    while (true) {
        auto dimension = parse_expression();
        dimensions.push_back(std::move(dimension));
        if (!match(TokenKind::PUNC_COMMA)) {
            break;
        }
        if (check(TokenKind::PUNC_RBRACKET)) {
            ParserError::invalid(peek().location,
                                 "trailing comma in array type");
        }
    }
    if (!match(TokenKind::PUNC_RBRACKET)) {
        ParserError::expected(peek().location, "']'", "after array type");
    }
    return std::make_unique<ArrayType>(
        type_token.location, std::move(element_type), std::move(dimensions));
}
