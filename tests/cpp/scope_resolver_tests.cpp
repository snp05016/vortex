#include "frontend/parser/parser.h"
#include "frontend/symbol-resolution/scope.h"
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace {
int failures = 0;

struct CollectingListener final : DiagnosticListener {
    std::vector<std::string> errors;

    void report(const Diagnostic &diagnostic) override {
        if (diagnostic.severity == DiagnosticSeverity::Error) {
            errors.push_back(diagnostic.message);
        }
    }
};

void fail(std::string_view name, const std::string &message) {
    std::cerr << "[FAIL] " << name << ": " << message << '\n';
    failures++;
}

std::vector<std::string> resolve_errors(const std::string &source) {
    Parser parser(source.data(), source.size());
    auto program = parser.parse();
    CollectingListener listener;
    ScopeResolver(listener).resolve(*program);
    return listener.errors;
}

void expect(std::string_view name, const std::string &source,
            std::size_t error_count) {
    const auto errors = resolve_errors(source);
    if (errors.size() != error_count) {
        fail(name, "expected " + std::to_string(error_count) + " errors, got " +
                       std::to_string(errors.size()) +
                       (errors.empty() ? "" : " (first: " + errors[0] + ")"));
    }
}

void test_accepted() {
    expect("empty main", "fn main() {}", 0);
    expect("forward call and recursion",
           "fn main() { f(1); }\nfn f(n: i32) { f(n); g(); }\nfn g() {}", 0);
    expect("struct declared later",
           "fn main() {}\nstruct A { b: B }\nstruct B { x: i32 }", 0);
    expect("inner block shadows",
           "fn main() { let t = 5; { let t = t + 1; print(t); } print(t); }",
           0);
    expect(
        "sequential loops share a name",
        "fn main() { for i in 0..3 { print(i); } for i in 0..3 { print(i); } }",
        0);
    expect("loop var shadows outer local",
           "fn main() { let i = 1; for i in 0..3 { print(i); } print(i); }", 0);
}

void test_rejected() {
    expect("unknown name", "fn main() { print(missing); }", 1);
    expect("use after block",
           "fn main() { { let inside = 1; } print(inside); }", 1);
    expect("duplicate local", "fn main() { let a = 1; let a = 2; }", 1);
    expect("local reuses parameter",
           "fn main() {}\nfn f(a: i32) { let a = 1; }", 1);
    expect("duplicate parameter", "fn main() {}\nfn f(a: i32, a: i32) {}", 1);
    expect("local named like function",
           "fn main() { let helper = 1; }\nfn helper() {}", 1);
    expect("local named print", "fn main() { let print = 1; }", 1);
    expect("duplicate field", "fn main() {}\nstruct S { v: i32, v: f32 }", 1);
    expect("struct and function share a name",
           "fn main() {}\nstruct P { x: i32 }\nfn P() {}", 1);
    expect("unknown type", "fn main() { let v: Missing = 1; }", 1);
    expect("no main", "fn helper() {}", 1);
    expect("main with parameter", "fn main(a: i32) {}", 1);
    expect("main returning i32", "fn main() -> i32 { return 1; }", 1);
    expect("two mains report once", "fn main() {}\nfn main() {}", 1);
    expect("initializer cannot see its own name", "fn main() { let x = x; }",
           1);
}

// `let t = t + 1;` in an inner block must link to the outer t, and the later
// use to the inner one.
void test_links() {
    const std::string source =
        "fn main() { let t = 5; { let t = t + 1; print(t); } }";
    Parser parser(source.data(), source.size());
    auto program = parser.parse();
    CollectingListener listener;
    ScopeResolver resolver(listener);
    resolver.resolve(*program);

    const auto &main_fn =
        static_cast<const FunctionDecl &>(*program->declarations()[0]);
    const auto &statements = main_fn.body().statements();
    const auto &inner =
        static_cast<const BlockStmt &>(*statements[1]).statements();
    const auto &inner_let = static_cast<const VarDeclStmt &>(*inner[0]);
    const auto &call = static_cast<const CallCastExpr &>(
        static_cast<const ExprStmt &>(*inner[1]).expr());
    const auto &init = static_cast<const BinaryExpr &>(inner_let.initializer());

    const Symbol *init_link = resolver.symbol_for(init.left());
    const Symbol *use_link = resolver.symbol_for(*call.arguments()[0]);
    if (init_link == nullptr ||
        init_link->declaration_node != statements[0].get()) {
        fail("links", "initializer did not link to the outer t");
    }
    if (use_link == nullptr || use_link->declaration_node != &inner_let) {
        fail("links", "use did not link to the inner t");
    }
}
} // namespace

int main() {
    test_accepted();
    test_rejected();
    test_links();
    if (failures != 0) {
        return 1;
    }
    std::cout << "scope resolver tests passed\n";
    return 0;
}
