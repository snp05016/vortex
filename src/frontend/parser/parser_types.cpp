#include "parser.h"

// parses a type; type parsing is not implemented yet.
std::unique_ptr<Type> Parser::parse_type() {
    Token type_token = peek();
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
