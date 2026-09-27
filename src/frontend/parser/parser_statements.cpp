#include "parser.h"

namespace {
bool is_assignment_operator(TokenKind kind) {
    return kind == TokenKind::OP_ASSIGN || kind == TokenKind::OP_PLUS_ASSIGN ||
           kind == TokenKind::OP_MINUS_ASSIGN ||
           kind == TokenKind::OP_MULTIPLY_ASSIGN ||
           kind == TokenKind::OP_DIVIDE_ASSIGN ||
           kind == TokenKind::OP_MODULO_ASSIGN;
}

/// checks if an expression is a valid target for an assignment.
/// for `values[index]`, it checks if the index expression is a valid assignment
/// target. for `object.field`, it checks if the object expression is a valid
/// assignment target.
bool is_assignment_target(const Expr &expression) {
    if (dynamic_cast<const Identifier *>(&expression)) {
        return true;
    }
    if (const auto *index = dynamic_cast<const IndexExpr *>(&expression)) {
        return is_assignment_target(index->collection());
    }
    if (const auto *field =
            dynamic_cast<const FieldAccessExpr *>(&expression)) {
        return is_assignment_target(field->object());
    }
    return false;
}
} // namespace

// this funct parses parses a statement that starts with an identifier and is
// followed by an assignment operator. for example) `value = 1;` or
// `values[index] += 2;` or `object.field -= 3;` what this does is that it
// checks if the statement starts with an identifier and is followed by an
// assignment operator. If it does, it parses the statement as an assignment
// statement.
// if not, it returns false and the parser will try to parse the statement as an
// expression statement instead.
bool Parser::starts_assignment_statement() {
    if (!check(TokenKind::IDENTIFIER)) {
        return false;
    }
    std::size_t offset = 1;
    while (true) {
        if (peek(offset).kind == TokenKind::PUNC_DOT) {
            if (peek(offset + 1).kind != TokenKind::IDENTIFIER) {
                return false;
            }
            offset += 2;
            continue;
        }
        if (peek(offset).kind == TokenKind::PUNC_LBRACKET) {
            std::size_t bracket_depth = 1;
            offset++;
            while (bracket_depth != 0) {
                TokenKind const kind = peek(offset).kind;
                if (kind == TokenKind::EOF_TOKEN) {
                    return false;
                }
                if (kind == TokenKind::PUNC_LBRACKET) {
                    bracket_depth++;
                } else if (kind == TokenKind::PUNC_RBRACKET) {
                    bracket_depth--;
                }
                offset++;
            }
            continue;
        }
        return is_assignment_operator(peek(offset).kind);
    }
}

/// chooses the parser for the statement at the current token.
/// for `let count = 1;`, it delegates to the variable declaration parser.
/// identifier-led targets are assignments; other starts may be expressions.
std::unique_ptr<Stmt> Parser::parse_statement() {
    switch (peek().kind) {
    case TokenKind::KW_LET:
        return parse_var_declaration();
    case TokenKind::KW_RETURN:
        return parse_return_statement();
    case TokenKind::KW_IF:
        return parse_if_statement();
    case TokenKind::KW_WHILE:
        return parse_while_statement();
    case TokenKind::KW_FOR:
        return parse_for_statement();
    case TokenKind::KW_BREAK:
        return parse_break_statement();
    case TokenKind::KW_CONTINUE:
        return parse_continue_statement();
    case TokenKind::PUNC_LBRACE:
        return parse_block_statement();
    default:
        if (starts_assignment_statement()) {
            return parse_assignment_statement();
        }
        return parse_expression_statement();
    }
}

/// parses a local variable declaration and its initializer.
/// for `let mut total: f32 = 0.0;`, it keeps mutability, type, name, and value.
/// the type annotation is optional, but the initializer and final semicolon are
/// required.
std::unique_ptr<Stmt> Parser::parse_var_declaration() {
    Token const var_token = peek();
    if (!match(TokenKind::KW_LET)) {
        ParserError::expected(var_token.location, "'let'",
                              "for variable declaration");
    }
    bool const is_mutable = match(TokenKind::KW_MUT);
    Token const name_token = peek();
    if (!match(TokenKind::IDENTIFIER)) {
        ParserError::expected(name_token.location, "identifier",
                              "for variable declaration");
    }
    std::string var_name = name_token.current_token_string();
    std::unique_ptr<Type> var_type;
    if (match(TokenKind::PUNC_COLON)) {
        var_type = parse_type();
        if (!var_type) {
            ParserError::expected(peek().location, "type",
                                  "after ':' in variable declaration");
        }
    }
    if (!match(TokenKind::OP_ASSIGN)) {
        ParserError::expected(peek().location, "'='",
                              "in variable declaration");
    }
    auto initializer = parse_expression();
    if (!initializer) {
        ParserError::expected(peek().location, "initializer expression",
                              "in variable declaration");
    }
    if (!match(TokenKind::PUNC_SEMICOLON)) {
        ParserError::expected(peek().location, "';'",
                              "after variable declaration");
    }
    return std::make_unique<VarDeclStmt>(
        var_token.location, is_mutable, std::move(var_name),
        std::move(var_type), std::move(initializer));
}

std::unique_ptr<Stmt> Parser::parse_assignment_statement() {
    Token const target_token = peek();
    auto lhs = parse_postfix();
    if (!lhs || !is_assignment_target(*lhs)) {
        ParserError::invalid(target_token.location, "assignment target");
    }
    AssignmentOperation op{};
    if (match(TokenKind::OP_ASSIGN)) {
        op = AssignmentOperation::Assign;
    } else if (match(TokenKind::OP_PLUS_ASSIGN)) {
        op = AssignmentOperation::AddAssign;
    } else if (match(TokenKind::OP_MINUS_ASSIGN)) {
        op = AssignmentOperation::SubtractAssign;
    } else if (match(TokenKind::OP_MULTIPLY_ASSIGN)) {
        op = AssignmentOperation::MultiplyAssign;
    } else if (match(TokenKind::OP_DIVIDE_ASSIGN)) {
        op = AssignmentOperation::DivideAssign;
    } else if (match(TokenKind::OP_MODULO_ASSIGN)) {
        op = AssignmentOperation::RemainderAssign;
    } else {
        ParserError::expected(peek().location, "assignment operator",
                              "in assignment statement");
    }
    auto rhs = parse_expression();
    if (!rhs) {
        ParserError::expected(peek().location, "expression",
                              "for right-hand side of assignment");
    }
    if (!match(TokenKind::PUNC_SEMICOLON)) {
        ParserError::expected(peek().location, "';'",
                              "after assignment statement");
    }
    return std::make_unique<AssignmentStatement>(
        target_token.location, std::move(lhs), std::move(rhs), op);
}

std::unique_ptr<Stmt> Parser::parse_return_statement() {
    Token const return_token = peek();
    if (!match(TokenKind::KW_RETURN)) {
        ParserError::expected(return_token.location, "'return'",
                              "for return statement");
    }
    if (check(TokenKind::PUNC_SEMICOLON)) {
        advance(); // consume ';'.
        return std::make_unique<RetStmt>(return_token.location, nullptr);
    }
    auto return_value = parse_expression();
    if (!return_value) {
        ParserError::expected(peek().location, "expression",
                              "for return value");
    }
    if (!match(TokenKind::PUNC_SEMICOLON)) {
        ParserError::expected(peek().location, "';'", "after return statement");
    }
    return std::make_unique<RetStmt>(return_token.location,
                                     std::move(return_value));
}

std::unique_ptr<Stmt> Parser::parse_expression_statement() {
    Token const expression_token = peek();
    auto expr = parse_expression();
    if (!expr) {
        ParserError::expected(peek().location, "expression",
                              "for expression statement");
    }
    if (!match(TokenKind::PUNC_SEMICOLON)) {
        ParserError::expected(peek().location, "';'",
                              "after expression statement");
    }
    return std::make_unique<ExprStmt>(expression_token.location,
                                      std::move(expr));
}

std::unique_ptr<Stmt> Parser::parse_block_statement() {
    Token const left_brace = peek();
    if (!match(TokenKind::PUNC_LBRACE)) {
        ParserError::expected(left_brace.location, "'{'",
                              "for block statement");
    }
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!check(TokenKind::PUNC_RBRACE)) {
        if (check(TokenKind::EOF_TOKEN)) {
            ParserError::expected(peek().location, "'}'",
                                  "after block statement");
        }
        auto statement = parse_statement();
        if (!statement) {
            ParserError::expected(peek().location, "statement",
                                  "in block statement");
        }
        statements.push_back(std::move(statement));
    }
    advance(); // consume '}'.
    return std::make_unique<BlockStmt>(left_brace.location,
                                       std::move(statements));
}

std::unique_ptr<Stmt> Parser::parse_if_statement() {
    Token const if_token = peek();
    if (!match(TokenKind::KW_IF)) {
        ParserError::expected(if_token.location, "'if'", "for if statement");
    }
    auto condition = parse_expression();
    if (!condition) {
        ParserError::expected(peek().location, "expression",
                              "for if statement condition");
    }

    std::unique_ptr<Stmt> then_branch = parse_block_statement();
    if (!then_branch) {
        ParserError::expected(peek().location, "block",
                              "for then branch of if statement");
    }

    std::unique_ptr<Stmt> else_branch;
    if (match(TokenKind::KW_ELSE)) {
        if (check(TokenKind::KW_IF)) {
            else_branch = parse_if_statement();
        } else {
            else_branch = parse_block_statement();
        }
    }

    return std::make_unique<IfStmt>(if_token.location, std::move(condition),
                                    std::move(then_branch),
                                    std::move(else_branch));
}

std::unique_ptr<Stmt> Parser::parse_while_statement() {
    Token const while_token = peek();
    if (!match(TokenKind::KW_WHILE)) {
        ParserError::expected(while_token.location, "'while'",
                              "for while statement");
    }
    auto condition = parse_expression();
    if (!condition) {
        ParserError::expected(peek().location, "expression",
                              "for while statement condition");
    }

    std::unique_ptr<Stmt> body = parse_block_statement();
    if (!body) {
        ParserError::expected(peek().location, "block",
                              "for body of while statement");
    }

    return std::make_unique<WhileStmt>(while_token.location,
                                       std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::parse_for_statement() {
    Token const for_token = peek();
    if (!match(TokenKind::KW_FOR)) {
        ParserError::expected(for_token.location, "'for'", "for for statement");
    }
    Token const loop_var_token = peek();
    if (!match(TokenKind::IDENTIFIER)) {
        ParserError::expected(loop_var_token.location, "identifier",
                              "for loop variable in for statement");
    }
    std::string loop_var_name = loop_var_token.current_token_string();
    if (!match(TokenKind::KW_IN)) {
        ParserError::expected(peek().location, "'in'", "for for statement");
    }
    auto iterable = parse_expression();
    if (!iterable) {
        ParserError::expected(peek().location, "expression",
                              "for iterable in for statement");
    }
    std::unique_ptr<Stmt> body = parse_block_statement();
    if (!body) {
        ParserError::expected(peek().location, "block",
                              "for body of for statement");
    }
    return std::make_unique<ForStmt>(for_token.location,
                                     std::move(loop_var_name),
                                     std::move(iterable), std::move(body));
}

std::unique_ptr<Stmt> Parser::parse_break_statement() {
    Token const break_token = peek();
    if (!match(TokenKind::KW_BREAK)) {
        ParserError::expected(break_token.location, "'break'",
                              "for break statement");
    }
    if (!match(TokenKind::PUNC_SEMICOLON)) {
        ParserError::expected(peek().location, "';'", "after break statement");
    }
    return std::make_unique<BreakStmt>(break_token.location);
}

std::unique_ptr<Stmt> Parser::parse_continue_statement() {
    Token const continue_token = peek();
    if (!match(TokenKind::KW_CONTINUE)) {
        ParserError::expected(continue_token.location, "'continue'",
                              "for continue statement");
    }
    if (!match(TokenKind::PUNC_SEMICOLON)) {
        ParserError::expected(peek().location, "';'",
                              "after continue statement");
    }
    return std::make_unique<ContinueStmt>(continue_token.location);
}
