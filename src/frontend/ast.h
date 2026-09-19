#pragma once
#include "token.h"

// represents the base qualities for an ast node
class Node{
        private:
                SourceLocation location;

        protected://protected members can be accessed by derived classes
                virtual ~Node() = default;
};
struct Expr : public Node {
        virtual ~Expr() = default;
};
struct Statement : public Node {
        virtual ~Statement() = default;
};
enum class BinOp {
        ADD,
        SUBTRACT,
        MULTIPLY,
        DIVIDE,
        EQUALS,
        NOT_EQUALS,
        LESS_THAN,
        GREATER_THAN,
        LESS_THAN_OR_EQUAL,
        GREATER_THAN_OR_EQUAL,
        SHIFT_LEFT,
        SHIFT_RIGHT,
        LOGICAL_AND,
        LOGICAL_OR,
        BITWISE_AND,
        BITWISE_OR,
        BITWISE_XOR,
        MODULO,
        BITWISE_NOT,
};
enum class RelOp {
        EQUALS,
        NOT_EQUALS,
        LESS_THAN,
        GREATER_THAN,
        LESS_THAN_OR_EQUAL,
        GREATER_THAN_OR_EQUAL,
};
struct BinExpr : public Expr {
        Expr* left;
        Expr* right;
        BinOp op;
        // override to delete the l and r expr when the Bin expr is deleted, 
        // to avoid memory leaks0
        ~BinExpr() override {
                delete left;
                delete right;
        }
};
enum class UnOp {
        NEGATE,
        POSITIVE,
        NEGATIVE,
        LOGICAL_NOT,
        BITWISE_NOT, 
        REFERENCE_AND // &value 
};
struct UnExpr : public Expr { 
        // override to delete the operand when the UnExpr is deleted,
        // to avoid memory leaks
        Expr* operand;
        UnOp op;
        ~UnExpr() override {
                delete operand;
        }
};