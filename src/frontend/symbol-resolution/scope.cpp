#include "scope.h"

void ScopeResolver::error(const Node *node, const std::string &message) {
    error_count_++;
    if (listener_) {
        listener_->report(
            {DiagnosticSeverity::Error, node->location(), message});
    }
}

void ScopeResolver::note(SourceLocation loc, const std::string &message) {
    if (listener_) {
        listener_->report({DiagnosticSeverity::Note, loc, message});
    }
}

void ScopeResolver::resolve(const Program &program) {
    for (const auto &declaration : program.declarations()) {
        resolve_decl(*declaration);
    }
}

void ScopeResolver::resolve_decl(const Decl &decl) {
    if (const auto *function_decl = dynamic_cast<const FunctionDecl *>(&decl)) {
        if (!scope_stack_.declare(function_decl->function_name(),
                                  SymbolKind::Function,
                                  function_decl->location(), function_decl)) {
            error(&decl, "redeclaration of function '" +
                             function_decl->function_name() + "'");
        }
        scope_stack_.enter_scope();
        // checks for parameter name clashes with top level decls
        for (const auto &param : function_decl->parameters()) {
            if (const Symbol *clash =
                    scope_stack_.check_illegal_shadow(param.param_name())) {
                error(&param, "parameter '" + param.param_name() +
                                  "' shadows a top-level declaration");
                note(clash->location, "previously declared here");
            }
            if (!scope_stack_.declare(param.param_name(), SymbolKind::Parameter,
                                      param.location(), &param, true)) {
                error(&param, "redeclaration of parameter '" +
                                  param.param_name() + "'");
            }
        }
        for (const auto &param : function_decl->parameters()) {
            resolve_type(param.param_type());
        }
        resolve_type(function_decl->return_type());
        for (const auto &statement : function_decl->body().statements()) {
            resolve_stmt(*statement);
        }
        scope_stack_.exit_scope();
    } else if (const auto *struct_decl =
                   dynamic_cast<const StructDecl *>(&decl)) {
        if (!scope_stack_.declare(struct_decl->struct_name(),
                                  SymbolKind::Struct, struct_decl->location(),
                                  struct_decl)) {
            error(&decl, "redeclaration of struct '" +
                             struct_decl->struct_name() + "'");
        }
        scope_stack_.enter_scope();
        for (const auto &field : struct_decl->fields()) {
            if (!scope_stack_.declare(field.field_name(), SymbolKind::Field,
                                      field.location(), &field)) {
                error(&field,
                      "redeclaration of field '" + field.field_name() + "'");
            }
        }
        for (const auto &field : struct_decl->fields()) {
            resolve_type(field.field_type());
        }
        scope_stack_.exit_scope();
    } else {
        error(&decl, "unknown declaration kind");
    }
}

void ScopeResolver::resolve_stmt(const Stmt &stmt) {
    if (const auto *block = dynamic_cast<const BlockStmt *>(&stmt)) {
        resolve_block(*block);
    } else if (const auto *if_stmt = dynamic_cast<const IfStmt *>(&stmt)) {
        resolve_if_stmt(*if_stmt);
    } else if (const auto *while_stmt =
                   dynamic_cast<const WhileStmt *>(&stmt)) {
        resolve_while_stmt(*while_stmt);
    } else if (const auto *for_stmt = dynamic_cast<const ForStmt *>(&stmt)) {
        resolve_for_stmt(*for_stmt);
    } else if (const auto *var_decl =
                   dynamic_cast<const VarDeclStmt *>(&stmt)) {
        resolve_var_decl(*var_decl);
    } else if (const auto *assignment =
                   dynamic_cast<const AssignmentStatement *>(&stmt)) {
        resolve_assignment_stmt(*assignment);
    } else if (const auto *ret_stmt = dynamic_cast<const RetStmt *>(&stmt)) {
        resolve_ret_stmt(*ret_stmt);
    } else if (const auto *expr_stmt = dynamic_cast<const ExprStmt *>(&stmt)) {
        resolve_expr_stmt(*expr_stmt);
    } else if (const auto *break_stmt =
                   dynamic_cast<const BreakStmt *>(&stmt)) {
        resolve_break_stmt(*break_stmt);
    } else if (const auto *continue_stmt =
                   dynamic_cast<const ContinueStmt *>(&stmt)) {
        resolve_continue_stmt(*continue_stmt);
    } else {
        error(&stmt, "unknown statement kind");
    }
}

/*
 * resolve_expr walks an expression tree and hands every name it finds to
 * resolve_identifier. Most node kinds just recurse into their children:
 *
 *   let y = (x + 1) * f(x);   // visits x, x, and f
 *
 * Literals have no names, so they're skipped. Field names (`p.x`) aren't
 * looked up here, since they depend on p's struct type and get checked later.
 */
void ScopeResolver::resolve_expr(const Expr &expr) {
    if (const auto *binary_expr = dynamic_cast<const BinaryExpr *>(&expr)) {
        // `a + b`: both sides can contain names
        resolve_expr(binary_expr->left());
        resolve_expr(binary_expr->right());
    } else if (const auto *unary_expr = dynamic_cast<const Unary *>(&expr)) {
        // `-x`, `!x`, `&x`: only the operand can contain names
        resolve_expr(unary_expr->operand());
    } else if (dynamic_cast<const Literal *>(&expr) != nullptr) {
        // nothing to resolve for literals
    } else if (const auto *identifier =
                   dynamic_cast<const Identifier *>(&expr)) {
        // a bare name used as a value, e.g. the `x` in `x + 1`
        resolve_identifier(*identifier, /*is_callee=*/false);
    } else if (const auto *group = dynamic_cast<const Grp *>(&expr)) {
        // `(x + 1)`: parentheses add no scope, just look inside
        resolve_expr(group->expr());
    } else if (const auto *range = dynamic_cast<const RangeExpr *>(&expr)) {
        // `0..n`: both ends can contain names
        resolve_expr(range->start());
        resolve_expr(range->end());
    } else if (const auto *call = dynamic_cast<const CallCastExpr *>(&expr)) {
        // `f(x)`: the callee may be a function or builtin, unlike a plain
        // identifier, so it takes the is_callee path
        if (const auto *callee =
                dynamic_cast<const Identifier *>(&call->function())) {
            resolve_identifier(*callee, /*is_callee=*/true);
        } else {
            // something like `make()(x)`, where the callee is itself an
            // expression
            resolve_expr(call->function());
        }
        for (const auto &argument : call->arguments()) {
            resolve_expr(*argument);
        }
    } else if (const auto *cast = dynamic_cast<const CastExpr *>(&expr)) {
        // `i32(x)`: the target type is a keyword, only the operand needs
        // resolving
        resolve_expr(cast->operand());
    } else if (const auto *index = dynamic_cast<const IndexExpr *>(&expr)) {
        // `arr[i][j]`: resolve the collection and every index
        resolve_expr(index->collection());
        for (const auto &idx : index->indices()) {
            resolve_expr(*idx);
        }
    } else if (const auto *field_access =
                   dynamic_cast<const FieldAccessExpr *>(&expr)) {
        // `p.x`: the field name is checked against the struct type later
        resolve_expr(field_access->object());
    } else if (const auto *array = dynamic_cast<const ArrayExpr *>(&expr)) {
        // `[a, b, c]`: each element is its own expression
        for (const auto &element : array->elements()) {
            resolve_expr(*element);
        }
    } else if (const auto *repeat =
                   dynamic_cast<const RepeatArrayExpr *>(&expr)) {
        // `[0; n]`: the repeated value and the size expressions
        resolve_expr(repeat->value());
        for (const auto &dimension : repeat->dimensions()) {
            resolve_expr(*dimension);
        }
    } else if (const auto *construction =
                   dynamic_cast<const StructConstructionExpr *>(&expr)) {
        // `Point { x: 1, y: 2 }`: the struct name must refer to a struct
        const Symbol *symbol = scope_stack_.lookup(construction->struct_name());
        if (!symbol) {
            error(&expr,
                  "unknown struct type '" + construction->struct_name() + "'");
        } else if (symbol->kind != SymbolKind::Struct) {
            error(&expr,
                  "'" + construction->struct_name() + "' is not a struct type");
        }
        // field names are checked later, but the values can contain names
        for (const auto &field : construction->fields()) {
            resolve_expr(*field.second);
        }
    } else {
        error(&expr, "unknown expression kind");
    }
}

/*
 * resolve_identifier does the actual scope lookup, and it cares about context.
 * A plain name has to be a local, parameter or loop variable:
 *
 *   let total = 5;
 *   print(total);   // ok, total is a local
 *   print(Point);   // error: 'Point' is not a variable
 *
 * A callee is different, it may also be a function or builtin:
 *
 *   print(total);   // ok, print is a builtin used as a callee
 *   total(1);       // error: 'total' is not callable
 *
 * A name that isn't in scope at all reports "unknown name".
 */
void ScopeResolver::resolve_identifier(const Identifier &identifier,
                                       bool is_callee) {
    // lookup searches from the innermost scope outwards, so the nearest
    // declaration wins (this is what makes shadowing work)
    const Symbol *symbol = scope_stack_.lookup(identifier.name());
    if (!symbol) {
        error(&identifier, "unknown name '" + identifier.name() + "'");
        return;
    }
    const SymbolKind kind = symbol->kind;
    // things you can use as a value
    const bool is_value = kind == SymbolKind::Local ||
                          kind == SymbolKind::Parameter ||
                          kind == SymbolKind::LoopVariable;
    // things you can call
    const bool is_function =
        kind == SymbolKind::Function || kind == SymbolKind::Builtin;
    if (is_value || (is_callee && is_function)) {
        // remember which symbol this name refers to, for later passes
        insert_variable(&identifier, symbol);
    } else {
        error(&identifier,
              "'" + identifier.name() +
                  (is_callee ? "' is not callable" : "' is not a variable"));
    }
}

void ScopeResolver::resolve_expr_stmt(const ExprStmt &expr_stmt) {
    resolve_expr(expr_stmt.expr());
}

void ScopeResolver::resolve_assignment_stmt(
    const AssignmentStatement &assignment_stmt) {
    resolve_expr(assignment_stmt.lhs());
    resolve_expr(assignment_stmt.rhs());
}

void ScopeResolver::resolve_type(const Type &type) {
    if (const auto *array_type = dynamic_cast<const ArrayType *>(&type)) {
        resolve_type(array_type->element_type());
        for (const auto &dimension : array_type->dimensions()) {
            resolve_expr(*dimension);
        }
    } else if (const auto *reference_type =
                   dynamic_cast<const ReferenceType *>(&type)) {
        resolve_type(reference_type->referenced_type());
    } else if (dynamic_cast<const PrimitiveType *>(&type) != nullptr) {
        // nothing to resolve for primitive types
    } else if (const auto *struct_type =
                   dynamic_cast<const StructType *>(&type)) {
        const Symbol *symbol = scope_stack_.lookup(struct_type->struct_name());
        if (!symbol) {
            error(&type,
                  "unknown struct type '" + struct_type->struct_name() + "'");
        } else if (symbol->kind != SymbolKind::Struct) {
            error(&type,
                  "'" + struct_type->struct_name() + "' is not a struct type");
        } else {
            insert_type(struct_type, symbol);
        }
    } else {
        error(&type, "unknown type kind");
    }
}

void ScopeResolver::resolve_var_decl(const VarDeclStmt &var_decl) {
    // resolve the initializer first so `let x = x;` doesn't see the new x
    resolve_expr(var_decl.initializer());
    // illegal shadow: a local reusing a top-level function, struct or
    // builtin name, e.g.
    //   fn foo() { }
    //   fn bar() { let foo = 5; }   // error
    // shadowing an outer local in an inner block is legal.
    if (const Symbol *clash =
            scope_stack_.check_illegal_shadow(var_decl.var_name())) {
        error(&var_decl, "variable '" + var_decl.var_name() +
                             "' shadows a top-level declaration");
        note(clash->location, "previously declared here");
    }
    if (!scope_stack_.declare(var_decl.var_name(), SymbolKind::Local,
                              var_decl.location(), &var_decl,
                              var_decl.is_mutable())) {
        error(&var_decl,
              "redeclaration of variable '" + var_decl.var_name() + "'");
    }
}

void ScopeResolver::resolve_block(const BlockStmt &block) {
    scope_stack_.enter_scope();
    for (const auto &statement : block.statements()) {
        resolve_stmt(*statement);
    }
    scope_stack_.exit_scope();
}

void ScopeResolver::resolve_break_stmt(const BreakStmt & /*break_stmt*/) {
    // nothing to resolve for break statements
}

void ScopeResolver::resolve_continue_stmt(
    const ContinueStmt & /*continue_stmt*/) {
    // nothing to resolve for continue statements
}

void ScopeResolver::resolve_ret_stmt(const RetStmt &ret_stmt) {
    if (const Expr *value = ret_stmt.return_value()) {
        resolve_expr(*value);
    }
}

void ScopeResolver::resolve_if_stmt(const IfStmt &if_stmt) {
    resolve_expr(if_stmt.condition());
    resolve_stmt(if_stmt.then_branch());
    if (if_stmt.else_branch() != nullptr) {
        resolve_stmt(*if_stmt.else_branch());
    }
}

void ScopeResolver::resolve_while_stmt(const WhileStmt &while_stmt) {
    resolve_expr(while_stmt.condition());
    scope_stack_.enter_scope();
    resolve_stmt(while_stmt.body());
    scope_stack_.exit_scope();
}

void ScopeResolver::resolve_for_stmt(const ForStmt &for_stmt) {
    if (!scope_stack_.declare(for_stmt.var_name(), SymbolKind::LoopVariable,
                              for_stmt.location(), &for_stmt)) {
        error(&for_stmt,
              "redeclaration of loop variable '" + for_stmt.var_name() + "'");
    }
    resolve_expr(for_stmt.iterable());
    scope_stack_.enter_scope();
    resolve_stmt(for_stmt.body());
    scope_stack_.exit_scope();
}
