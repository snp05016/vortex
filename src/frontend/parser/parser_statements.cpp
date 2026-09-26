#include "parser.h"

// parses a statement; statement parsing is not implemented yet.
std::unique_ptr<Stmt> Parser::parse_statement() {
    if (check(TokenKind::KW_LET)) {
        return parse_var_declaration();
    }
    return nullptr;
}

std::unique_ptr<Stmt> Parser::parse_var_declaration() {
    Token var_token = peek();
    if (!match(TokenKind::KW_LET)) {
        parser_errors::expected(var_token.location, "'let'",
                                "for variable declaration");
    }
    bool is_mutable = match(TokenKind::KW_MUT);
    Token name_token = peek();
    if (!match(TokenKind::IDENTIFIER)) {
        parser_errors::expected(name_token.location, "identifier",
                                "for variable declaration");
    }
    std::string var_name = name_token.current_token_string();
    std::unique_ptr<Type> var_type;
    if (match(TokenKind::PUNC_COLON)) {
        var_type = parse_type();
        if (!var_type) {
            parser_errors::expected(peek().location, "type",
                                    "after ':' in variable declaration");
        }
    }
    if (!match(TokenKind::OP_ASSIGN)) {
        parser_errors::expected(peek().location, "'='",
                                "in variable declaration");
    }
    auto initializer = parse_expression();
    if (!initializer) {
        parser_errors::expected(peek().location, "initializer expression",
                                "in variable declaration");
    }
    if (!match(TokenKind::PUNC_SEMICOLON)) {
        parser_errors::expected(peek().location, "';'",
                                "after variable declaration");
    }
    return std::make_unique<VarDeclStmt>(
        var_token.location, is_mutable, std::move(var_name),
        std::move(var_type), std::move(initializer));
}
