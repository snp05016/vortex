#pragma once
#include <memory>
#include "token.h"
// a constructor will create a new node with the location of the token, and the location of thee node will be 
// be the location of the token. The location of the node will be used to report errors and warnings in the source code.
// a constrctor in C++ is a special function that runs everytime a new object is created

// represents the base qualities for an ast node
class Node{
        public:
                virtual ~Node() = default; // virtual destructor to ensure proper cleanup of derived classes
                SourceLocation location() const { // getter for the location
                        return location_;
                }
        protected:
                // constructor to initialize the location of the node
                explicit Node(SourceLocation location): location_(location){} 
        private: 
                // member variable to store the location of the node
                SourceLocation location_;
};

struct Expr: public Node{
        public:
                virtual ~Expr() = default; // virtual destructor for proper cleanup of derived classes
        protected:
                // create a new expr node with the location
                explicit Expr(SourceLocation location): Node(location){}

};

struct Stmt: public Node{
        public:
                virtual ~Stmt() = default; // virtual destructor for proper cleanup of derived classes
        protected:
                // create a new stmt node with the location
                explicit Stmt(SourceLocation location): Node(location){}
};

struct Decl: public Node{
        public:
                virtual ~Decl() = default; // virtual destructor for proper cleanup of derived classes
        protected:
                // create a new decl node with the location
                explicit Decl(SourceLocation location): Node(location){}
};

struct Type: public Node{
        public:
                virtual ~Type() = default; // virtual destructor for proper cleanup of derived classes
        protected:
                // create a new type node with the location
                explicit Type(SourceLocation location): Node(location){}
};