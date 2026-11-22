#include "ast_walker.h"

void AstWalker::walk(const Program &program) {
    for (const auto &decl : program.declarations()) {
        walk(*decl);
    }
}

void AstWalker::walk(const Decl &base) {
    if (const auto *node = dynamic_cast<const FunctionDecl *>(&base)) {
        visit(*node);
    } else if (const auto *node = dynamic_cast<const StructDecl *>(&base)) {
        visit(*node);
    } else if (const auto *node = dynamic_cast<const ParamDecl *>(&base)) {
        visit(*node);
    } else if (const auto *node =
                   dynamic_cast<const StructFieldDecl *>(&base)) {
        visit(*node);
    } else {
        visit(base);
    }
}

void AstWalker::walk(const Stmt &base) {
    if (const auto *node = dynamic_cast<const BlockStmt *>(&base)) {
        visit(*node);
    } else if (const auto *node = dynamic_cast<const VarDeclStmt *>(&base)) {
        visit(*node);
    } else if (const auto *node =
                   dynamic_cast<const AssignmentStatement *>(&base)) {
        visit(*node);
    } else if (const auto *node = dynamic_cast<const RetStmt *>(&base)) {
        visit(*node);
    } else if (const auto *node = dynamic_cast<const ExprStmt *>(&base)) {
        visit(*node);
    } else if (const auto *node = dynamic_cast<const IfStmt *>(&base)) {
        visit(*node);
    } else if (const auto *node = dynamic_cast<const WhileStmt *>(&base)) {
        visit(*node);
    } else if (const auto *node = dynamic_cast<const ForStmt *>(&base)) {
        visit(*node);
    } else if (const auto *node = dynamic_cast<const BreakStmt *>(&base)) {
        visit(*node);
    } else if (const auto *node = dynamic_cast<const ContinueStmt *>(&base)) {
        visit(*node);
    } else {
        visit(base);
    }
}

void AstWalker::walk(const Expr &base) {
    if (const auto *node = dynamic_cast<const Literal *>(&base)) {
        visit(*node);
    } else if (const auto *node = dynamic_cast<const Identifier *>(&base)) {
        visit(*node);
    } else if (const auto *node = dynamic_cast<const Grp *>(&base)) {
        visit(*node);
    } else if (const auto *node = dynamic_cast<const Unary *>(&base)) {
        visit(*node);
    } else if (const auto *node = dynamic_cast<const BinaryExpr *>(&base)) {
        visit(*node);
    } else if (const auto *node = dynamic_cast<const RangeExpr *>(&base)) {
        visit(*node);
    } else if (const auto *node = dynamic_cast<const CallCastExpr *>(&base)) {
        visit(*node);
    } else if (const auto *node = dynamic_cast<const CastExpr *>(&base)) {
        visit(*node);
    } else if (const auto *node = dynamic_cast<const IndexExpr *>(&base)) {
        visit(*node);
    } else if (const auto *node =
                   dynamic_cast<const FieldAccessExpr *>(&base)) {
        visit(*node);
    } else if (const auto *node = dynamic_cast<const ArrayExpr *>(&base)) {
        visit(*node);
    } else if (const auto *node =
                   dynamic_cast<const RepeatArrayExpr *>(&base)) {
        visit(*node);
    } else if (const auto *node =
                   dynamic_cast<const StructConstructionExpr *>(&base)) {
        visit(*node);
    } else {
        visit(base);
    }
}

void AstWalker::walk(const Type &type) {
    dispatch_type(type, [this](const auto &node) { visit(node); });
}

void AstWalker::visit(const FunctionDecl &node) {
    for (const auto &param : node.parameters()) {
        walk(param);
    }
    walk(node.return_type());
    walk(node.body());
}

void AstWalker::visit(const StructDecl &node) {
    for (const auto &field : node.fields()) {
        walk(field);
    }
}

void AstWalker::visit(const ParamDecl &node) {
    walk(node.param_type());
}

void AstWalker::visit(const StructFieldDecl &node) {
    walk(node.field_type());
}

void AstWalker::visit(const BlockStmt &node) {
    for (const auto &stmt : node.statements()) {
        walk(*stmt);
    }
}

void AstWalker::visit(const VarDeclStmt &node) {
    visit_unhandled_statement(node);
    if (const Type *type = node.var_type()) {
        walk(*type);
    }
    walk(node.initializer());
}

void AstWalker::visit(const AssignmentStatement &node) {
    visit_unhandled_statement(node);
    walk(node.lhs());
    walk(node.rhs());
}

void AstWalker::visit(const RetStmt &node) {
    visit_unhandled_statement(node);
    if (const Expr *value = node.return_value()) {
        walk(*value);
    }
}

void AstWalker::visit(const ExprStmt &node) {
    visit_unhandled_statement(node);
    walk(node.expr());
}

void AstWalker::visit(const IfStmt &node) {
    visit_unhandled_statement(node);
    walk(node.condition());
    walk(node.then_branch());
    if (const Stmt *branch = node.else_branch()) {
        walk(*branch);
    }
}

void AstWalker::visit(const WhileStmt &node) {
    visit_unhandled_statement(node);
    walk(node.condition());
    walk(node.body());
}

void AstWalker::visit(const ForStmt &node) {
    visit_unhandled_statement(node);
    walk(node.iterable());
    walk(node.body());
}

void AstWalker::visit(const BreakStmt &node) {
    visit_unhandled_statement(node);
}

void AstWalker::visit(const ContinueStmt &node) {
    visit_unhandled_statement(node);
}

void AstWalker::visit(const Grp &node) {
    walk(node.expr());
}

void AstWalker::visit(const Unary &node) {
    walk(node.operand());
}

void AstWalker::visit(const BinaryExpr &node) {
    walk(node.left());
    walk(node.right());
}

void AstWalker::visit(const RangeExpr &node) {
    walk(node.start());
    walk(node.end());
}

void AstWalker::visit(const CallCastExpr &node) {
    walk(node.function());
    for (const auto &arg : node.arguments()) {
        walk(*arg);
    }
}

void AstWalker::visit(const CastExpr &node) {
    walk(node.operand());
}

void AstWalker::visit(const IndexExpr &node) {
    walk(node.collection());
    for (const auto &index : node.indices()) {
        walk(*index);
    }
}

void AstWalker::visit(const FieldAccessExpr &node) {
    walk(node.object());
}

void AstWalker::visit(const ArrayExpr &node) {
    for (const auto &element : node.elements()) {
        walk(*element);
    }
}

void AstWalker::visit(const RepeatArrayExpr &node) {
    walk(node.value());
    for (const auto &dimension : node.dimensions()) {
        walk(*dimension);
    }
}

void AstWalker::visit(const StructConstructionExpr &node) {
    for (const auto &field : node.fields()) {
        walk(*field.second);
    }
}

void AstWalker::visit(const ArrayType &node) {
    walk(node.element_type());
    for (const auto &dimension : node.dimensions()) {
        walk(*dimension);
    }
}

void AstWalker::visit(const ReferenceType &node) {
    walk(node.referenced_type());
}
