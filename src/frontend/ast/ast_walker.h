#pragma once

#include "ast.h"

// walk() dispatches a base AST node to its concrete visit() overload.
// A pass overrides visit() where it needs to do work of its own.
class AstWalker {
  protected:
    ~AstWalker() = default;

    void walk(const Program &program);
    void walk(const Decl &base);
    void walk(const Stmt &base);
    void walk(const Expr &base);
    void walk(const Type &type);

    // Written type resolution also uses this dispatch to return a value.
    template <typename Visitor>
    static decltype(auto) dispatch_type(const Type &type, Visitor visitor) {
        if (const auto *node = dynamic_cast<const PrimitiveType *>(&type)) {
            return visitor(*node);
        }
        if (const auto *node = dynamic_cast<const StructType *>(&type)) {
            return visitor(*node);
        }
        if (const auto *node = dynamic_cast<const ArrayType *>(&type)) {
            return visitor(*node);
        }
        if (const auto *node = dynamic_cast<const ReferenceType *>(&type)) {
            return visitor(*node);
        }
        return visitor(type);
    }

    virtual void visit(const Decl & /*unused*/ /*unused*/ /*unused*/) {
    }

    virtual void visit(const FunctionDecl &node);
    virtual void visit(const StructDecl &node);
    virtual void visit(const ParamDecl &node);
    virtual void visit(const StructFieldDecl &node);

    // The default statement handlers call this before walking their children.
    // Blocks just group statements, so they only walk their children.
    virtual void
    visit_unhandled_statement(const Stmt & /*unused*/ /*unused*/ /*unused*/) {
    }

    virtual void visit(const Stmt & /*unused*/ /*unused*/ /*unused*/) {
    }

    virtual void visit(const BlockStmt &node);
    virtual void visit(const VarDeclStmt &node);
    virtual void visit(const AssignmentStatement &node);
    virtual void visit(const RetStmt &node);
    virtual void visit(const ExprStmt &node);
    virtual void visit(const IfStmt &node);
    virtual void visit(const WhileStmt &node);
    virtual void visit(const ForStmt &node);
    virtual void visit(const BreakStmt &node);
    virtual void visit(const ContinueStmt &node);

    virtual void visit(const Expr & /*unused*/ /*unused*/ /*unused*/) {
    }

    virtual void visit(const Literal & /*unused*/ /*unused*/ /*unused*/) {
    }

    virtual void visit(const Identifier & /*unused*/ /*unused*/ /*unused*/) {
    }

    virtual void visit(const Grp &node);
    virtual void visit(const Unary &node);
    virtual void visit(const BinaryExpr &node);
    virtual void visit(const RangeExpr &node);
    virtual void visit(const CallCastExpr &node);
    virtual void visit(const CastExpr &node);
    virtual void visit(const IndexExpr &node);
    virtual void visit(const FieldAccessExpr &node);
    virtual void visit(const ArrayExpr &node);
    virtual void visit(const RepeatArrayExpr &node);
    virtual void visit(const StructConstructionExpr &node);

    virtual void visit(const Type & /*unused*/ /*unused*/ /*unused*/) {
    }

    virtual void visit(const PrimitiveType & /*unused*/ /*unused*/ /*unused*/) {
    }

    virtual void visit(const StructType & /*unused*/ /*unused*/ /*unused*/) {
    }

    virtual void visit(const ArrayType &node);
    virtual void visit(const ReferenceType &node);
};
