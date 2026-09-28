#include "ast_debug_printer.h"

#include <ostream>
#include <sstream>
#include <type_traits>
#include <utility>

namespace {
const char *primitive_name(PrimitiveTypeKind kind) {
    switch (kind) {
    case PrimitiveTypeKind::Void:
        return "void";
    case PrimitiveTypeKind::Bool:
        return "bool";
    case PrimitiveTypeKind::Char:
        return "char";
    case PrimitiveTypeKind::i32:
        return "i32";
    case PrimitiveTypeKind::u32:
        return "u32";
    case PrimitiveTypeKind::usize:
        return "usize";
    case PrimitiveTypeKind::f32:
        return "f32";
    case PrimitiveTypeKind::f64:
        return "f64";
    case PrimitiveTypeKind::String:
        return "String";
    }
    return "unknown";
}

const char *unary_name(UnaryOp op) {
    switch (op) {
    case UnaryOp::Negate:
        return "-";
    case UnaryOp::LogicalNot:
        return "!";
    case UnaryOp::BitwiseNot:
        return "~";
    case UnaryOp::Positive:
        return "+";
    case UnaryOp::Reference:
        return "&";
    case UnaryOp::MutReference:
        return "&mut";
    }
    return "?";
}

const char *binary_name(BinOp op) {
    switch (op) {
    case BinOp::Add:
        return "+";
    case BinOp::Subtract:
        return "-";
    case BinOp::Multiply:
        return "*";
    case BinOp::Divide:
        return "/";
    case BinOp::Modulo:
        return "%";
    case BinOp::Equal:
        return "==";
    case BinOp::NotEqual:
        return "!=";
    case BinOp::Less:
        return "<";
    case BinOp::Greater:
        return ">";
    case BinOp::LessEqual:
        return "<=";
    case BinOp::GreaterEqual:
        return ">=";
    case BinOp::LogicalAnd:
        return "&&";
    case BinOp::LogicalOr:
        return "||";
    case BinOp::BitwiseAnd:
        return "&";
    case BinOp::BitwiseOr:
        return "|";
    case BinOp::BitwiseXor:
        return "^";
    case BinOp::LeftShift:
        return "<<";
    case BinOp::RightShift:
        return ">>";
    }
    return "?";
}

const char *assignment_name(AssignmentOperation op) {
    switch (op) {
    case AssignmentOperation::Assign:
        return "=";
    case AssignmentOperation::AddAssign:
        return "+=";
    case AssignmentOperation::SubtractAssign:
        return "-=";
    case AssignmentOperation::MultiplyAssign:
        return "*=";
    case AssignmentOperation::DivideAssign:
        return "/=";
    case AssignmentOperation::RemainderAssign:
        return "%=";
    }
    return "?=";
}

std::string literal_value(const LiteralValue &value) {
    return std::visit(
        [](const auto &item) {
            std::ostringstream output;
            using Value = std::decay_t<decltype(item)>;
            if constexpr (std::is_same_v<Value, std::string>) {
                output << '"';
                for (char const character : item) {
                    switch (character) {
                    case '\n':
                        output << "\\n";
                        break;
                    case '\r':
                        output << "\\r";
                        break;
                    case '\t':
                        output << "\\t";
                        break;
                    case '\\':
                        output << "\\\\";
                        break;
                    case '"':
                        output << "\\\"";
                        break;
                    default:
                        output << character;
                        break;
                    }
                }
                output << '"';
            } else if constexpr (std::is_same_v<Value, char>) {
                output << "'";
                switch (item) {
                case '\n':
                    output << "\\n";
                    break;
                case '\r':
                    output << "\\r";
                    break;
                case '\t':
                    output << "\\t";
                    break;
                case '\\':
                    output << "\\\\";
                    break;
                case '\'':
                    output << "\\'";
                    break;
                default:
                    output << item;
                    break;
                }
                output << "'";
            } else if constexpr (std::is_same_v<Value, bool>) {
                output << (item ? "true" : "false");
            } else {
                output << item;
            }
            return output.str();
        },
        value);
}
} // namespace

AstDebugPrinter::AstDebugPrinter(std::ostream &output) : output_(output) {
}

void AstDebugPrinter::print(const Program &program) const {
    print_root(make_program(program));
}

AstDebugPrinter::TreeNode
AstDebugPrinter::make_program(const Program &program) {
    TreeNode result{"Program", {}};
    for (const auto &declaration : program.declarations()) {
        result.children.push_back(make_declaration(*declaration));
    }
    return result;
}

AstDebugPrinter::TreeNode
AstDebugPrinter::make_declaration(const Decl &declaration) {
    if (const auto *function =
            dynamic_cast<const FunctionDecl *>(&declaration)) {
        TreeNode parameters{"Parameters", {}};
        for (const ParamDecl &parameter : function->parameters()) {
            parameters.children.push_back(make_parameter(parameter));
        }
        TreeNode result{"FunctionDecl name=" + function->function_name() +
                            location_suffix(function->location()),
                        {}};
        result.children.push_back(std::move(parameters));
        result.children.push_back(
            branch("ReturnType", make_type(function->return_type())));
        result.children.push_back(
            branch("Body", make_statement(function->body())));
        return result;
    }
    if (const auto *structure =
            dynamic_cast<const StructDecl *>(&declaration)) {
        TreeNode result{"StructDecl name=" + structure->struct_name() +
                            location_suffix(structure->location()),
                        {}};
        for (const StructFieldDecl &field : structure->fields()) {
            result.children.push_back(make_field(field));
        }
        return result;
    }
    return {"UnknownDecl" + location_suffix(declaration.location()), {}};
}

AstDebugPrinter::TreeNode
AstDebugPrinter::make_statement(const Stmt &statement) {
    if (const auto *block = dynamic_cast<const BlockStmt *>(&statement)) {
        TreeNode result{"BlockStmt" + location_suffix(block->location()), {}};
        for (const auto &child : block->statements()) {
            result.children.push_back(make_statement(*child));
        }
        return result;
    }
    if (const auto *variable = dynamic_cast<const VarDeclStmt *>(&statement)) {
        TreeNode result{
            "VarDeclStmt name=" + variable->var_name() +
                " mutable=" + (variable->is_mutable() ? "true" : "false") +
                location_suffix(variable->location()),
            {}};
        if (variable->var_type()) {
            result.children.push_back(
                branch("Type", make_type(*variable->var_type())));
        }
        result.children.push_back(
            branch("Initializer", make_expression(variable->initializer())));
        return result;
    }
    if (const auto *assignment =
            dynamic_cast<const AssignmentStatement *>(&statement)) {
        TreeNode result{"AssignmentStmt operator=\"" +
                            std::string(assignment_name(assignment->op())) +
                            "\"" + location_suffix(assignment->location()),
                        {}};
        result.children.push_back(
            branch("Target", make_expression(assignment->lhs())));
        result.children.push_back(
            branch("Value", make_expression(assignment->rhs())));
        return result;
    }
    if (const auto *return_statement =
            dynamic_cast<const RetStmt *>(&statement)) {
        TreeNode result{
            "ReturnStmt" + location_suffix(return_statement->location()), {}};
        if (return_statement->return_value()) {
            result.children.push_back(
                make_expression(*return_statement->return_value()));
        }
        return result;
    }
    if (const auto *expression = dynamic_cast<const ExprStmt *>(&statement)) {
        return {"ExprStmt" + location_suffix(expression->location()),
                {make_expression(expression->expr())}};
    }
    if (const auto *conditional = dynamic_cast<const IfStmt *>(&statement)) {
        TreeNode result{"IfStmt" + location_suffix(conditional->location()),
                        {}};
        result.children.push_back(
            branch("Condition", make_expression(conditional->condition())));
        result.children.push_back(
            branch("Then", make_statement(conditional->then_branch())));
        if (conditional->else_branch()) {
            result.children.push_back(
                branch("Else", make_statement(*conditional->else_branch())));
        }
        return result;
    }
    if (const auto *loop = dynamic_cast<const WhileStmt *>(&statement)) {
        TreeNode result{"WhileStmt" + location_suffix(loop->location()), {}};
        result.children.push_back(
            branch("Condition", make_expression(loop->condition())));
        result.children.push_back(branch("Body", make_statement(loop->body())));
        return result;
    }
    if (const auto *loop = dynamic_cast<const ForStmt *>(&statement)) {
        TreeNode result{"ForStmt variable=" + loop->var_name() +
                            location_suffix(loop->location()),
                        {}};
        result.children.push_back(
            branch("Iterable", make_expression(loop->iterable())));
        result.children.push_back(branch("Body", make_statement(loop->body())));
        return result;
    }
    if (dynamic_cast<const BreakStmt *>(&statement)) {
        return {"BreakStmt" + location_suffix(statement.location()), {}};
    }
    if (dynamic_cast<const ContinueStmt *>(&statement)) {
        return {"ContinueStmt" + location_suffix(statement.location()), {}};
    }
    return {"UnknownStmt" + location_suffix(statement.location()), {}};
}

AstDebugPrinter::TreeNode
AstDebugPrinter::make_expression(const Expr &expression) {
    if (const auto *literal = dynamic_cast<const Literal *>(&expression)) {
        return {"Literal value=" + literal_value(literal->value()) +
                    location_suffix(literal->location()),
                {}};
    }
    if (const auto *identifier =
            dynamic_cast<const Identifier *>(&expression)) {
        return {"Identifier name=" + identifier->name() +
                    location_suffix(identifier->location()),
                {}};
    }
    if (const auto *group = dynamic_cast<const Grp *>(&expression)) {
        return {"GroupExpr" + location_suffix(group->location()),
                {make_expression(group->expr())}};
    }
    if (const auto *unary = dynamic_cast<const Unary *>(&expression)) {
        return {"UnaryExpr operator=\"" + std::string(unary_name(unary->op())) +
                    "\"" + location_suffix(unary->location()),
                {make_expression(unary->operand())}};
    }
    if (const auto *binary = dynamic_cast<const BinaryExpr *>(&expression)) {
        return {"BinaryExpr operator=\"" +
                    std::string(binary_name(binary->op())) + "\"" +
                    location_suffix(binary->location()),
                {make_expression(binary->left()),
                 make_expression(binary->right())}};
    }
    if (const auto *range = dynamic_cast<const RangeExpr *>(&expression)) {
        return {std::string("RangeExpr inclusive=") +
                    (range->is_inclusive() ? "true" : "false") +
                    location_suffix(range->location()),
                {branch("Start", make_expression(range->start())),
                 branch("End", make_expression(range->end()))}};
    }
    if (const auto *call = dynamic_cast<const CallCastExpr *>(&expression)) {
        TreeNode arguments{"Arguments", {}};
        for (const auto &argument : call->arguments()) {
            arguments.children.push_back(make_expression(*argument));
        }
        return {"CallExpr" + location_suffix(call->location()),
                {branch("Callee", make_expression(call->function())),
                 std::move(arguments)}};
    }
    if (const auto *cast = dynamic_cast<const CastExpr *>(&expression)) {
        return {"CastExpr target=" +
                    std::string(primitive_name(cast->target_type())) +
                    location_suffix(cast->location()),
                {make_expression(cast->operand())}};
    }
    if (const auto *index = dynamic_cast<const IndexExpr *>(&expression)) {
        TreeNode indices{"Indices", {}};
        for (const auto &item : index->indices()) {
            indices.children.push_back(make_expression(*item));
        }
        return {"IndexExpr" + location_suffix(index->location()),
                {branch("Collection", make_expression(index->collection())),
                 std::move(indices)}};
    }
    if (const auto *field =
            dynamic_cast<const FieldAccessExpr *>(&expression)) {
        return {"FieldAccessExpr field=" + field->field_name() +
                    location_suffix(field->location()),
                {make_expression(field->object())}};
    }
    if (const auto *array = dynamic_cast<const ArrayExpr *>(&expression)) {
        TreeNode result{"ArrayExpr" + location_suffix(array->location()), {}};
        for (const auto &element : array->elements()) {
            result.children.push_back(make_expression(*element));
        }
        return result;
    }
    if (const auto *array =
            dynamic_cast<const RepeatArrayExpr *>(&expression)) {
        TreeNode dimensions{"Dimensions", {}};
        for (const auto &dimension : array->dimensions()) {
            dimensions.children.push_back(make_expression(*dimension));
        }
        return {"RepeatArrayExpr" + location_suffix(array->location()),
                {branch("Value", make_expression(array->value())),
                 std::move(dimensions)}};
    }
    if (const auto *construction =
            dynamic_cast<const StructConstructionExpr *>(&expression)) {
        TreeNode result{
            "StructConstructionExpr name=" + construction->struct_name() +
                location_suffix(construction->location()),
            {}};
        for (const auto &[name, value] : construction->fields()) {
            result.children.push_back(
                branch("Field name=" + name, make_expression(*value)));
        }
        return result;
    }
    return {"UnknownExpr" + location_suffix(expression.location()), {}};
}

AstDebugPrinter::TreeNode AstDebugPrinter::make_type(const Type &type) {
    if (const auto *primitive = dynamic_cast<const PrimitiveType *>(&type)) {
        return {"PrimitiveType name=" +
                    std::string(primitive_name(primitive->primitive_type())) +
                    location_suffix(primitive->location()),
                {}};
    }
    if (const auto *array = dynamic_cast<const ArrayType *>(&type)) {
        TreeNode dimensions{"Dimensions", {}};
        for (const auto &dimension : array->dimensions()) {
            dimensions.children.push_back(make_expression(*dimension));
        }
        return {"ArrayType" + location_suffix(array->location()),
                {branch("ElementType", make_type(array->element_type())),
                 std::move(dimensions)}};
    }
    if (const auto *named = dynamic_cast<const StructType *>(&type)) {
        return {"NamedType name=" + named->struct_name() +
                    location_suffix(named->location()),
                {}};
    }
    if (const auto *reference = dynamic_cast<const ReferenceType *>(&type)) {
        return {std::string("ReferenceType mutable=") +
                    (reference->is_mutable() ? "true" : "false") +
                    location_suffix(reference->location()),
                {make_type(reference->referenced_type())}};
    }
    return {"UnknownType" + location_suffix(type.location()), {}};
}

AstDebugPrinter::TreeNode
AstDebugPrinter::make_parameter(const ParamDecl &parameter) {
    return {"Parameter name=" + parameter.param_name() +
                location_suffix(parameter.location()),
            {make_type(parameter.param_type())}};
}

AstDebugPrinter::TreeNode
AstDebugPrinter::make_field(const StructFieldDecl &field) {
    return {"Field name=" + field.field_name() +
                location_suffix(field.location()),
            {make_type(field.field_type())}};
}

AstDebugPrinter::TreeNode AstDebugPrinter::branch(std::string label,
                                                  TreeNode child) {
    return {std::move(label), {std::move(child)}};
}

std::string AstDebugPrinter::location_suffix(SourceLocation location) {
    return " @" + std::to_string(location.start) + ":" +
           std::to_string(location.length);
}

void AstDebugPrinter::print_root(const TreeNode &node) const {
    output_ << node.label << '\n';
    for (std::size_t index = 0; index < node.children.size(); ++index) {
        print_child(node.children[index], "",
                    index + 1 == node.children.size());
    }
}

void AstDebugPrinter::print_child(const TreeNode &node,
                                  const std::string &prefix,
                                  bool is_last) const {
    output_ << prefix << (is_last ? "└── " : "├── ") << node.label << '\n';
    const std::string child_prefix = prefix + (is_last ? "    " : "│   ");
    for (std::size_t index = 0; index < node.children.size(); ++index) {
        print_child(node.children[index], child_prefix,
                    index + 1 == node.children.size());
    }
}
