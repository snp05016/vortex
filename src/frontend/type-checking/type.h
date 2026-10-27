#pragma once
#include "../ast/ast.h"
#include <cstdint>
#include <llvm/ADT/SmallVector.h>
#include <variant>

enum class TypeKind : std::uint8_t {
    PRIMITIVE,
    STRUCT,
    ARRAY,
    REFERENCE,
    ERROR
};

class ResolvedType;

struct StructTypeData {
    const StructDecl *struct_declaration_;
};

struct ArrayTypeData {
    const ResolvedType *element;
    std::variant<std::uint64_t, llvm::SmallVector<std::uint64_t, 2>> dimensions;
};

struct ReferenceTypeData {
    bool Is_mutable;
    const ResolvedType *referenced;
};

struct ErrorTypeData {};

// Data representation of a type. Nested type pointers are non-owning.
class ResolvedType {
  public:
    explicit ResolvedType(PrimitiveTypeKind primTypeKind)
        : data_(primTypeKind) {
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
