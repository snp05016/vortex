#include "parser.h"
#include "parser_expressions.h"
// parses a declaration; declaration parsing is not implemented yet.
inline std::unique_ptr<Decl> Parser::parse_declaration() {
    Token tok = peek();
    if (tok.kind == TokenKind::KW_FN) {
        advance(); // consume "fn".
        Token hopeful_identifier = peek();
        if (hopeful_identifier.kind != TokenKind::IDENTIFIER) {
            throw std::runtime_error("Expected function name after 'fn'"); // syntax error 
        }
    
    }
}