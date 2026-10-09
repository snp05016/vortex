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

void ScopeResolver::resolve_decl(const Decl & /*unused*/) {
}

static void resolve_stmt(const Stmt &stmt) {
}

static void resolve_expr(const Expr &expr) {
}

static void resolve_type(const Type &type) {
}
