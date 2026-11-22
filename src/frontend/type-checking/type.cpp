#include "frontend/type-checking/type.h"
#include "frontend/ast/ast.h"
#include "frontend/diagnostics/diagnostic_listener.h"
#include "frontend/symbol-resolution/scope.h"
#include <limits>
#include <optional>

namespace {

bool is_void(const ResolvedType &type) {
    return type.kind() == TypeKind::PRIMITIVE &&
           type.getPrimitiveTypeKind() == PrimitiveTypeKind::Void;
}

bool is_integer_literal(const Expr &expr) {
    const auto *literal = dynamic_cast<const Literal *>(&expr);
    return literal && literal->kind() == LiteralKind::Integer;
}

bool is_dimension_expression(const Expr &expr) {
    if (const auto *group = dynamic_cast<const Grp *>(&expr)) {
        return is_dimension_expression(group->expr());
    }
    if (const auto *unary = dynamic_cast<const Unary *>(&expr)) {
        return unary->op() == UnaryOp::Negate &&
               is_integer_literal(unary->operand());
    }
    if (const auto *binary = dynamic_cast<const BinaryExpr *>(&expr)) {
        switch (binary->op()) {
        case BinOp::Add:
        case BinOp::Subtract:
        case BinOp::Multiply:
        case BinOp::Divide:
        case BinOp::Modulo:
            return is_dimension_expression(binary->left()) &&
                   is_dimension_expression(binary->right());
        default:
            return false;
        }
    }
    return is_integer_literal(expr);
}

std::optional<std::uint64_t> evaluate_dimension(const Expr &expr) {
    if (const auto *group = dynamic_cast<const Grp *>(&expr)) {
        return evaluate_dimension(group->expr());
    }
    if (const auto *literal = dynamic_cast<const Literal *>(&expr)) {
        return std::get<std::uint64_t>(literal->value());
    }
    if (const auto *unary = dynamic_cast<const Unary *>(&expr)) {
        // Only a negated zero fits usize. The expression form was checked
        // first.
        if (evaluate_dimension(unary->operand()) == 0) {
            return 0;
        }
        return std::nullopt;
    }
    const auto &binary = dynamic_cast<const BinaryExpr &>(expr);

    const auto left = evaluate_dimension(binary.left());
    const auto right = evaluate_dimension(binary.right());
    if (!left || !right) {
        return std::nullopt;
    }

    constexpr auto max = std::numeric_limits<std::uint64_t>::max();
    switch (binary.op()) {
    case BinOp::Add:
        if (*right <= max - *left) {
            return *left + *right;
        }
        break;
    case BinOp::Subtract:
        if (*left >= *right) {
            return *left - *right;
        }
        break;
    case BinOp::Multiply:
        if (*right == 0 || *left <= max / *right) {
            return *left * *right;
        }
        break;
    case BinOp::Divide:
        if (*right != 0) {
            return *left / *right;
        }
        break;
    case BinOp::Modulo:
        if (*right != 0) {
            return *left % *right;
        }
        break;
    default:
        break;
    }
    return std::nullopt;
}

} // namespace

void TypeChecker::check(const Program &program) {
    error_count_ = 0;
    nested_types_.clear();
    walk(program);
}

void TypeChecker::visit(const Decl &decl) {
    type_error(decl, "unknown declaration kind");
}

void TypeChecker::visit(const FunctionDecl &function) {
    for (const auto &param : function.parameters()) {
        walk(param);
    }
    check_type(function.return_type());
    walk(function.body());
}

void TypeChecker::visit(const ParamDecl &param) {
    if (is_void(check_type(param.param_type()))) {
        type_error(param, "parameter type cannot be void");
    }
}

void TypeChecker::visit(const StructFieldDecl &field) {
    if (is_void(check_type(field.field_type()))) {
        type_error(field.field_type(), "struct field type cannot be void");
    }
}

void TypeChecker::visit(const Stmt &stmt) {
    visit_unhandled_statement(stmt);
}

void TypeChecker::visit_unhandled_statement(const Stmt &stmt) {
    type_error(stmt, "statement type checking is not implemented");
}

ResolvedType TypeChecker::check_type(const Type &type) {
    return dispatch_type(
        type, [this](const auto &node) { return resolve_type(node); });
}

ResolvedType TypeChecker::resolve_type(const Type &type) {
    return type_error(type, "unknown type kind");
}

ResolvedType TypeChecker::resolve_type(const PrimitiveType &type) {
    return ResolvedType(type.primitive_type());
}

ResolvedType TypeChecker::resolve_type(const StructType &type) {
    const Symbol *symbol = resolver_.get_symbol_for_node(&type);
    const auto *declaration =
        symbol ? dynamic_cast<const StructDecl *>(symbol->declaration_node)
               : nullptr;
    if (!declaration) {
        return type_error(type, "unresolved struct type");
    }
    return ResolvedType(StructTypeData{declaration});
}

ResolvedType TypeChecker::resolve_type(const ArrayType &array) {
    ResolvedType element = check_type(array.element_type());
    if (element.kind() == TypeKind::ERROR) {
        return element;
    }
    if (is_void(element)) {
        return type_error(array.element_type(),
                          "array element type cannot be void");
    }

    llvm::SmallVector<std::uint64_t, 2> dimensions;
    std::uint64_t element_count = 1;
    for (const auto &dimension : array.dimensions()) {
        if (!is_dimension_expression(*dimension)) {
            return type_error(
                *dimension,
                "array dimension must be an integer constant expression");
        }
        const auto extent = evaluate_dimension(*dimension);
        if (!extent) {
            return type_error(
                *dimension, "array dimension arithmetic is invalid for usize");
        }
        if (*extent == 0) {
            return type_error(*dimension,
                              "array dimension must be greater than zero");
        }
        if (element_count >
            std::numeric_limits<std::uint64_t>::max() / *extent) {
            return type_error(*dimension, "array element count exceeds usize");
        }
        element_count *= *extent;
        dimensions.push_back(*extent);
    }
    nested_types_.push_back(std::move(element));
    return ResolvedType(
        ArrayTypeData{&nested_types_.back(), std::move(dimensions)});
}

ResolvedType TypeChecker::resolve_type(const ReferenceType &reference) {
    ResolvedType referenced = check_type(reference.referenced_type());
    if (referenced.kind() == TypeKind::ERROR) {
        return referenced;
    }
    if (is_void(referenced)) {
        return type_error(reference.referenced_type(),
                          "referenced type cannot be void");
    }
    nested_types_.push_back(std::move(referenced));
    return ResolvedType(
        ReferenceTypeData{reference.is_mutable(), &nested_types_.back()});
}

ResolvedType TypeChecker::type_error(const Node &node, const char *message) {
    ++error_count_;
    if (listener_) {
        listener_->report(
            {DiagnosticSeverity::Error, node.location(), message});
    }
    return ResolvedType(ErrorTypeData{});
}
