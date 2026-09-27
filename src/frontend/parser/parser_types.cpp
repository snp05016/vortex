#include "parser.h"

/// parses any type and delegates recursive forms to their own parser.
/// for `&mut [Point; 4]`, it preserves the reference, array, and named type.
std::unique_ptr<Type> Parser::parse_type() {
    Token const type_token = peek();
    if (type_token.kind == TokenKind::OP_BITWISE_AND) {
        return parse_reference_type(); // parsing the refernce type that is the
                                       // &mut [Point; 4] in the example
    }
    if (type_token.kind == TokenKind::PUNC_LBRACKET) {
        return check_array_type(); // parsing the array type that is the [Point;
                                   // 4] in the example
    }
    if (type_token.kind == TokenKind::IDENTIFIER) {
        return parse_named_types(); // parsing the named type that is the Point
                                    // in the example
    }

    PrimitiveTypeKind kind{}; // {} for default initialization to avoid
                              // uninitialized variable warning
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
    Token const type_token = peek();
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

/// parses `&type` or `&mut type`, including recursively nested types.
/// `mut` belongs directly after `&`, before the referenced type.
std::unique_ptr<Type> Parser::parse_reference_type() {
    Token const ref_token = peek();
    if (ref_token.kind != TokenKind::OP_BITWISE_AND) {
        return nullptr;
    }
    advance(); // consume '&'.
    bool const is_mutable = match(TokenKind::KW_MUT);
    auto referenced_type = parse_type();
    if (!referenced_type) {
        ParserError::expected(peek().location, "type",
                              "after reference marker");
    }
    return std::make_unique<ReferenceType>(
        ref_token.location, std::move(referenced_type), is_mutable);
}

/// parses an identifier as a named type and preserves its spelling.
/// for `Point`, name resolution later decides which struct it refers to.
std::unique_ptr<Type> Parser::parse_named_types() {
    Token const name_token = peek();
    if (name_token.kind != TokenKind::IDENTIFIER) {
        return nullptr;
    }
    advance();
    return std::make_unique<StructType>(name_token.location,
                                        name_token.current_token_string());
}