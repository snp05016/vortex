#pragma once
#include "../ast/ast_walker.h"
#include "frontend/diagnostics/diagnostic_listener.h"
#include <cstdint>
#include <deque>
#include <llvm/ADT/SmallVector.h>
#include <utility>
#include <variant>

enum class TypeKind : std::uint8_t {
    PRIMITIVE,
    STRUCT,
    ARRAY,
    REFERENCE,
    ERROR
};

class ResolvedType;
class ScopeResolver;

struct StructTypeData {
    const StructDecl *struct_declaration_;
};

struct ArrayTypeData {
    const ResolvedType *element;
    llvm::SmallVector<std::uint64_t, 2> dimensions;
};

// Data representation of a reference type.
struct ReferenceTypeData {
    // Indicates if the reference is mutable.
    bool Is_mutable;
    const ResolvedType *referenced;
};

struct ErrorTypeData {};

// Data representation of a type. Nested type pointers are non-owning.
class ResolvedType {
  public:
    explicit ResolvedType(ErrorTypeData error) : data_(error) {
    }

    explicit ResolvedType(PrimitiveTypeKind primTypeKind)
        : data_(primTypeKind) {
    }

    explicit ResolvedType(StructTypeData type) : data_(type) {
    }

    explicit ResolvedType(ArrayTypeData type) : data_(std::move(type)) {
    }

    explicit ResolvedType(ReferenceTypeData type) : data_(type) {
    }

    [[nodiscard]] TypeKind kind() const {
        if (std::holds_alternative<PrimitiveTypeKind>(data_)) {
            return TypeKind::PRIMITIVE;
        }
        if (std::holds_alternative<StructTypeData>(data_)) {
            return TypeKind::STRUCT;
        }
        if (std::holds_alternative<ArrayTypeData>(data_)) {
            return TypeKind::ARRAY;
        }
        if (std::holds_alternative<ReferenceTypeData>(data_)) {
            return TypeKind::REFERENCE;
        }
        return TypeKind::ERROR;
    }

    [[nodiscard]] PrimitiveTypeKind getPrimitiveTypeKind() const {
        return std::get<PrimitiveTypeKind>(data_);
    }

  private:
    std::variant<StructTypeData, PrimitiveTypeKind, ArrayTypeData,
                 ReferenceTypeData, ErrorTypeData>
        data_;
};

class TypeChecker : private AstWalker {
  public:
    TypeChecker(DiagnosticListener *listener, const ScopeResolver &resolver)
        : listener_(listener), resolver_(resolver) {
    }

    void check(const Program &program);

    [[nodiscard]] bool had_errors() const {
        return error_count_ > 0;
    }

  private:
    void visit(const Decl &decl) override;
    void visit(const FunctionDecl &function) override;
    void visit(const ParamDecl &param) override;
    void visit(const StructFieldDecl &field) override;

    // Add concrete visit() overloads for statement and expression rules here.
    void visit(const Stmt &stmt) override;
    void visit_unhandled_statement(const Stmt &stmt) override;

    ResolvedType check_type(const Type &type);
    ResolvedType resolve_type(const Type &type);
    ResolvedType resolve_type(const PrimitiveType &type);
    ResolvedType resolve_type(const StructType &type);
    ResolvedType resolve_type(const ArrayType &array);
    ResolvedType resolve_type(const ReferenceType &reference);
    ResolvedType type_error(const Node &node, const char *message);
    DiagnosticListener *listener_;
    const ScopeResolver &resolver_;
    // Inserting at the end keeps pointers to existing nested types valid.
    std::deque<ResolvedType> nested_types_;
    std::uint32_t error_count_ = 0;
};
