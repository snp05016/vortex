#pragma once
#include "token.h"

// represents the base qualities for an ast node
class Node{
        private:
                SourceLocation location;

        protected://protected members can be accessed by derived classes
                virtual ~Node() = default;
};


struct Expression : public Node {
        virtual ~Expression() = default;
};

enum class BinaryOperator {
        ADD,
        SUBTRACT,
        MULTIPLY,
        DIVIDE,
        EQUALS,
        NOT_EQUALS,
        LESS_THAN,
        GREATER_THAN,
        LESS_THAN_OR_EQUAL,
        GREATER_THAN_OR_EQUAL
};
struct BinaryExpression : public Expression {
        Expression* left;
        Expression* right;
        BinaryOperator op;

        ~BinaryExpression() override {
                delete left;
                delete right;
        }
};
