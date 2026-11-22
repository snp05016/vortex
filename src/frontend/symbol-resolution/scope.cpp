#include "scope.h"

void ScopeResolver::error(const Node *node, const std::string &message) {
    error_at(node->location(), message);
}

void ScopeResolver::error_at(SourceLocation loc, const std::string &message) {
    error_count_++;
    if (listener_) {
        listener_->report({DiagnosticSeverity::Error, loc, message});
    }
}

void ScopeResolver::note(SourceLocation loc, const std::string &message) {
    if (listener_) {
        listener_->report({DiagnosticSeverity::Note, loc, message});
    }
}

void ScopeResolver::resolve(const Program &program) {
    // pass 1: declare every top-level function and struct, so declaration
    // order doesn't matter (a call may come before the function it names)
    for (const auto &declaration : program.declarations()) {
        declare_top_level(*declaration);
    }
    check_main();
    // pass 2: resolve types and bodies now that every top-level name exists
    walk(program);
}

void ScopeResolver::declare_top_level(const Decl &decl) {
    if (const auto *function_decl = dynamic_cast<const FunctionDecl *>(&decl)) {
        if (!scope_stack_.declare(function_decl->function_name(),
                                  SymbolKind::Function,
                                  function_decl->location(), function_decl)) {
            error(&decl, "redeclaration of function '" +
                             function_decl->function_name() + "'");
        }
    } else if (const auto *struct_decl =
                   dynamic_cast<const StructDecl *>(&decl);
               struct_decl &&
               (!scope_stack_.declare(struct_decl->struct_name(),
                                      SymbolKind::Struct,
                                      struct_decl->location(), struct_decl))) {
        error(&decl,
              "redeclaration of struct '" + struct_decl->struct_name() + "'");
    }
}

// exactly one `main`, taking no parameters and returning void. A second
// `main` was already reported as a redeclaration, so only the first is checked.
void ScopeResolver::check_main() {
    const Symbol *main_symbol = scope_stack_.lookup("main");
    if (main_symbol == nullptr || main_symbol->kind != SymbolKind::Function) {
        error_at({0, 1}, "program has no 'main' function");
        return;
    }
    const auto *main_fn =
        dynamic_cast<const FunctionDecl *>(main_symbol->declaration_node);
    if (!main_fn->parameters().empty()) {
        error(main_fn, "'main' must not take parameters");
    }
    const auto *return_type =
        dynamic_cast<const PrimitiveType *>(&main_fn->return_type());
    if (return_type == nullptr ||
        return_type->primitive_type() != PrimitiveTypeKind::Void) {
        error(main_fn, "'main' must return void");
    }
}

void ScopeResolver::visit(const Decl &decl) {
    error(&decl, "unknown declaration kind");
}

void ScopeResolver::visit(const FunctionDecl &function) {
    // Resolve the signature in the outer scope before declaring parameters.
    for (const auto &param : function.parameters()) {
        walk(param);
    }
    walk(function.return_type());

    // Parameters and the outermost statements share one scope.
    scope_stack_.enter_scope();
    for (const auto &param : function.parameters()) {
        if (const Symbol *clash =
                scope_stack_.check_illegal_shadow(param.param_name())) {
            error(&param, "parameter '" + param.param_name() +
                              "' shadows a top-level declaration");
            note(clash->location, "previously declared here");
        }
        if (!scope_stack_.declare(param.param_name(), SymbolKind::Parameter,
                                  param.location(), &param, true)) {
            error(&param,
                  "redeclaration of parameter '" + param.param_name() + "'");
        }
    }
    AstWalker::visit(function.body());
    scope_stack_.exit_scope();
}

void ScopeResolver::visit(const StructDecl &structure) {
    // Resolve field types in the outer scope, where field names are invisible.
    AstWalker::visit(structure);

    // This scope is only used to catch duplicate field names.
    scope_stack_.enter_scope();
    for (const auto &field : structure.fields()) {
        if (!scope_stack_.declare(field.field_name(), SymbolKind::Field,
                                  field.location(), &field)) {
            error(&field,
                  "redeclaration of field '" + field.field_name() + "'");
        }
    }
    scope_stack_.exit_scope();
}

void ScopeResolver::visit(const Stmt &stmt) {
    error(&stmt, "unknown statement kind");
}

void ScopeResolver::visit(const Expr &expr) {
    error(&expr, "unknown expression kind");
}

void ScopeResolver::visit(const Identifier &identifier) {
    resolve_identifier(identifier, /*is_callee=*/false);
}

void ScopeResolver::visit(const CallCastExpr &call) {
    if (const auto *callee =
            dynamic_cast<const Identifier *>(&call.function())) {
        resolve_identifier(*callee, /*is_callee=*/true);
        for (const auto &argument : call.arguments()) {
            walk(*argument);
        }
    } else {
        AstWalker::visit(call);
    }
}

void ScopeResolver::visit(const StructConstructionExpr &construction) {
    const Symbol *symbol = scope_stack_.lookup(construction.struct_name());
    if (!symbol) {
        error(&construction,
              "unknown struct type '" + construction.struct_name() + "'");
    } else if (symbol->kind != SymbolKind::Struct) {
        error(&construction,
              "'" + construction.struct_name() + "' is not a struct type");
    }
    AstWalker::visit(construction);
}

/* @llm written comment
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

void ScopeResolver::visit(const Type &type) {
    error(&type, "unknown type kind");
}

void ScopeResolver::visit(const StructType &type) {
    const Symbol *symbol = scope_stack_.lookup(type.struct_name());
    if (!symbol) {
        error(&type, "unknown struct type '" + type.struct_name() + "'");
    } else if (symbol->kind != SymbolKind::Struct) {
        error(&type, "'" + type.struct_name() + "' is not a struct type");
    } else {
        insert_type(&type, symbol);
    }
}

void ScopeResolver::visit(const VarDeclStmt &var_decl) {
    // Walk the annotation and initializer before declaring the local.
    AstWalker::visit(var_decl);
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

void ScopeResolver::visit(const BlockStmt &block) {
    scope_stack_.enter_scope();
    AstWalker::visit(block);
    scope_stack_.exit_scope();
}

void ScopeResolver::visit(const BreakStmt &break_stmt) {
    if (loop_depth_ == 0) {
        error(&break_stmt, "'break' outside of a loop");
    }
}

void ScopeResolver::visit(const ContinueStmt &continue_stmt) {
    if (loop_depth_ == 0) {
        error(&continue_stmt, "'continue' outside of a loop");
    }
}

void ScopeResolver::visit(const WhileStmt &while_stmt) {
    walk(while_stmt.condition());
    loop_depth_++;
    walk(while_stmt.body());
    loop_depth_--;
}

void ScopeResolver::visit(const ForStmt &for_stmt) {
    // the iterable is resolved outside the loop scope, so `for x in x` reads
    // the outer x, not the loop variable
    walk(for_stmt.iterable());
    scope_stack_.enter_scope();
    if (const Symbol *clash =
            scope_stack_.check_illegal_shadow(for_stmt.var_name())) {
        error(&for_stmt, "loop variable '" + for_stmt.var_name() +
                             "' shadows a top-level declaration");
        note(clash->location, "previously declared here");
    }
    if (!scope_stack_.declare(for_stmt.var_name(), SymbolKind::LoopVariable,
                              for_stmt.location(), &for_stmt)) {
        error(&for_stmt,
              "redeclaration of loop variable '" + for_stmt.var_name() + "'");
    }
    loop_depth_++;
    walk(for_stmt.body());
    loop_depth_--;
    scope_stack_.exit_scope();
}
