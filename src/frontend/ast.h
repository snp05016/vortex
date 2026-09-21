#pragma once
#include <memory>
#include <string>
#include <variant>
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
enum class LiteralKind {
        Integer,
        Float,
        String,
        Char,
        Boolean
};
// Use a string representation so this header remains compatible with the
// project's current language standard, which does not provide std::variant.
using LiteralValue = std::variant<int, double,std::string, char, bool>;
struct Literal: public Expr{
        public:
                virtual ~Literal() = default; // virtual destructor for proper cleanup of derived classes
        protected:
                // create a new literal node with the location
                explicit Literal(SourceLocation location): Expr(location){}
        private:
                // member variable to store the kind of literal
                LiteralKind kind_;
                // member variable to store the value of the literal
                LiteralValue value_;
};
struct Identifier: public Expr{
        public:
                virtual ~Identifier() = default; // virtual destructor for proper cleanup of derived classes
        protected:
                // create a new identifier node with the location
                explicit Identifier(SourceLocation location): Expr(location){}
        private:
                // member variable to store the name of the identifier
                std::string name_;
};
struct Grp: public Expr{
        public:
                virtual ~Grp() = default; // virtual destructor for proper cleanup of derived classes
        protected:
                // create a new group node with the location
                explicit Grp(SourceLocation location): Expr(location){}
        private:
                // member variable to store the expression inside the group
                std::unique_ptr<Expr> expr_;
};
enum class UnaryOp {
        Negate, // -x
        LogicalNot, // !x
        BitwiseNot, // ~x
        Positive, // +x
        Reference, // &x
};
struct Unary : public Expr{
        public:
                virtual ~Unary() = default;
        protected:
                // create a new unary node with the location
                explicit Unary(SourceLocation location): Expr(location){}
        private:
                // member variable to store the operator of the unary expression
                UnaryOp op_;
                // member variable to store the operand of the unary expression
                // unique_ptr is a smart pointer that owns and manages another object through a 
                // pointer and disposes of that object when the unique_ptr goes out of scope
                std::unique_ptr<Expr> operand_;
                
                // used unique_ptr because we wnat to ensure that the operand is properly cleaned 
                // up when the unary object is destroyed, dint use Expr* because we want to avoid 
                // memory leaks and dangling pointers, and unique_ptr provides automatic memory 
                // management and ownership semantics.
};
enum class BinOp {
        Add, // +
        Subtract, // -
        Multiply, // *
        Divide, // /
        Modulo, // %
        Equal, // ==
        NotEqual, // !=
        Less, // <
        Greater, // >
        LessEqual, // <=
        GreaterEqual, // >=
        LogicalAnd, // &&
        LogicalOr, // ||
        BitwiseAnd, // &
        BitwiseOr, // |
        BitwiseXor, // ^
        LeftShift, // <<
        RightShift, // >>
};
struct BinaryExpr : public Expr {
        public:
                virtual ~BinaryExpr() = default; // virtual destructor for proper cleanup of derived classes
        protected:
                // create a new binary expression node with the location
                explicit BinaryExpr(SourceLocation location) : Expr(location) {}
        private:
                // member variable to store the operator of the binary expression
                BinOp op_;
                // member variable to store the left operand of the binary expression
                std::unique_ptr<Expr> left_;
                // member variable to store the right operand of the binary expression
                std::unique_ptr<Expr> right_;
};
struct RangeExpr : public Expr {
        public:
                virtual ~RangeExpr() = default; // virtual destructor for proper cleanup of derived classes
        protected:
                // create a new range expression node with the location
                explicit RangeExpr(SourceLocation location) : Expr(location) {}
        private:
                // member variable to store the start of the range expression
                std::unique_ptr<Expr> start_;
                // member variable to store the end of the range expression
                std::unique_ptr<Expr> end_;
};
struct CallCastExpr : public Expr {
        public:
                virtual ~CallCastExpr() = default; // virtual destructor for proper cleanup of derived classes
        protected:
                // create a new call cast expression node with the location
                explicit CallCastExpr(SourceLocation location) : Expr(location) {}
        private:
                // member variable to store the function being called
                std::unique_ptr<Expr> function_;
                // member variable to store the arguments of the function call
                std::vector<std::unique_ptr<Expr>> arguments_;
};
struct IndexExpr: public Expr {
        
        public:
                virtual ~IndexExpr() = default; // virtual destructor for proper cleanup of derived classes
        protected:
                // create a new index expression node with the location
                explicit IndexExpr(SourceLocation location) : Expr(location) {}
        private:
                // member variable to store the collection being indexed
                std::unique_ptr<Expr> collection_;
                // member variable to store the index expression
                std::unique_ptr<Expr> index_;
};
struct FieldAccessExpr: public Expr {
        public:
                virtual ~FieldAccessExpr() = default; // virtual destructor for proper cleanup of derived classes
        protected:
                // create a new field access expression node with the location
                explicit FieldAccessExpr(SourceLocation location) : Expr(location) {}
        private:
                // member variable to store the object being accessed
                std::unique_ptr<Expr> object_;
                // member variable to store the field name being accessed, string cuz only the name of hte field is needed
                std::string field_name_; 
};
struct ArrayExpr: public Expr {
        public:
                virtual ~ArrayExpr() = default; // virtual destructor for proper cleanup of derived classes
        protected:
                // create a new array expression node with the location
                explicit ArrayExpr(SourceLocation location) : Expr(location) {}
        private:
                // member variable to store the elements of the array expression
                std::vector<std::unique_ptr<Expr>> elements_;
};
struct StructConstructionExpr: public Expr {
        public:
                virtual ~StructConstructionExpr() = default; // virtual destructor for proper cleanup of derived classes
        protected:
                // create a new struct construction expression node with the location
                explicit StructConstructionExpr(SourceLocation location) : Expr(location) {}
        private:
                // member variable to store the name of the struct being constructed
                std::string struct_name_;
                // member variable to store the fields of the struct construction expression,
                // in the form <name> { <string> : <expr> }
                std::vector<std::pair<std::string, std::unique_ptr<Expr>>> fields_;
};
struct VarDeclStmt: public Stmt{ 
        public:
                virtual ~VarDeclStmt() = default;
        protected:
                explicit VarDeclStmt(SourceLocation location) : Stmt(location){}
        private:
                std::string var_name_;
                std::unique_ptr<Expr> expr_;
};
enum class AssignmentOperation {
        Assign,          // =
        AddAssign,       // +=
        SubtractAssign,  // -=
        MultiplyAssign,  // *=
        DivideAssign,    // /=
        RemainderAssign  // %=
};
struct AssignmentStatement : public Stmt {
        public:
                virtual ~AssignmentStatement() = default;
        protected:
                explicit AssignmentStatement(SourceLocation location) : Stmt(location){}
        private:
                std::unique_ptr<Expr> lhs_;
                std::unique_ptr<Expr> rhs_;
                AssignmentOperation op_;
};

struct RetStmt : public Stmt {
        public:
                virtual ~RetStmt() = default;
        protected:
                explicit RetStmt(SourceLocation location) : Stmt(location){}
        private:
                std::unique_ptr<Expr> return_value_;
};

struct ExprStmt: public Stmt {
        public:
                virtual ~ExprStmt() = default;
        protected:
                explicit ExprStmt(SourceLocation location) : Stmt(location){}
        private:
                std::unique_ptr<Expr> expr_;
};
struct IfStmt: public Stmt {
        public:
                virtual ~IfStmt() = default;
        protected:
                explicit IfStmt(SourceLocation location) : Stmt(location){}
        private:
                std::unique_ptr<Expr> condition_;
                std::unique_ptr<Stmt> then_branch_;
                std::unique_ptr<Stmt> else_branch_;
};

struct WhileStmt: public Stmt {
        public:
                virtual ~WhileStmt() = default;
        protected:
                explicit WhileStmt(SourceLocation location) : Stmt(location){}
        private:
                std::unique_ptr<Expr> condition_;
                std::unique_ptr<Stmt> body_;
};
struct ForStmt: public Stmt {
        public:
                virtual ~ForStmt() = default;
        protected:
                explicit ForStmt(SourceLocation location) : Stmt(location){}
        private:
                std::unique_ptr<Stmt> init_;
                std::unique_ptr<Expr> condition_;
                std::unique_ptr<Stmt> increment_;
                std::unique_ptr<Stmt> body_;
};
struct BreakStmt: public Stmt {
        public:
                virtual ~BreakStmt() = default;
        protected:
                explicit BreakStmt(SourceLocation location) : Stmt(location){}
};
struct ContinueStmt: public Stmt {
        public:
                virtual ~ContinueStmt() = default;
        protected:
                explicit ContinueStmt(SourceLocation location) : Stmt(location){}
};
struct BlockStmt: public Stmt {
        public:
                virtual ~BlockStmt() = default;
        protected:
                explicit BlockStmt(SourceLocation location) : Stmt(location){}
        private:
                std::vector<std::unique_ptr<Stmt>> statements_;
};
struct FunctionDecl: public Decl {
        public:
                virtual ~FunctionDecl() = default;
        protected:
                explicit FunctionDecl(SourceLocation location) : Decl(location){}
        private:
                std::string function_name_;
                std::vector<std::pair<std::string, std::unique_ptr<Type>>> parameters_;
                std::unique_ptr<Type> return_type_;
                std::unique_ptr<BlockStmt> body_;
};
struct StructDecl: public Decl {
        public:
                virtual ~StructDecl() = default;
        protected:
                explicit StructDecl(SourceLocation location) : Decl(location){}
        private:
                std::string struct_name_;
                std::vector<std::pair<std::string, std::unique_ptr<Type>>> fields_;
};
struct VarDecl: public Decl {
        public:
                virtual ~VarDecl() = default;
        protected:
                explicit VarDecl(SourceLocation location) : Decl(location){}
        private:
                std::string var_name_;
                std::unique_ptr<Type> var_type_;
                std::unique_ptr<Expr> initializer_;
};