#pragma once

#include "../ast/ast.h"
#include <iosfwd>
#include <string>
#include <vector>

class AstDebugPrinter {
  public:
    explicit AstDebugPrinter(std::ostream &output);

    void print(const Program &program) const;

  private:
    struct TreeNode {
        std::string label;
        std::vector<TreeNode> children;
    };

    std::ostream &output_;

    static TreeNode make_program(const Program &program);
    static TreeNode make_declaration(const Decl &declaration);
    static TreeNode make_statement(const Stmt &statement);
    static TreeNode make_expression(const Expr &expression);
    static TreeNode make_type(const Type &type);
    static TreeNode make_parameter(const ParamDecl &parameter);
    static TreeNode make_field(const StructFieldDecl &field);
    static TreeNode branch(std::string label, TreeNode child);
    static std::string location_suffix(SourceLocation location);

    void print_root(const TreeNode &node) const;
    void print_child(const TreeNode &node, const std::string &prefix,
                     bool is_last) const;
};
