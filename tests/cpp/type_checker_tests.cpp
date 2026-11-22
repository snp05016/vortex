#include "frontend/parser/parser.h"
#include "frontend/symbol-resolution/scope.h"
#include "frontend/type-checking/type.h"
#include <iostream>
#include <string>
#include <vector>

struct Listener : DiagnosticListener {
    std::vector<std::string> errors;

    void report(const Diagnostic &diagnostic) override {
        if (diagnostic.severity == DiagnosticSeverity::Error) {
            errors.push_back(diagnostic.message);
        }
    }
};

int main() {
    int failures = 0;
    const auto check = [&](const std::string &source,
                           const std::string &expected) {
        const std::string input = "fn main() {}\n" + source;
        Parser parser(input.data(), input.size());
        const auto program = parser.parse();
        if (!program) {
            std::cerr << "Could not parse: " << source << '\n';
            ++failures;
            return;
        }
        Listener listener;
        ScopeResolver resolver(&listener);
        resolver.resolve(*program);
        if (resolver.had_errors()) {
            std::cerr << "Could not resolve: " << source << '\n';
            ++failures;
            return;
        }
        TypeChecker checker(&listener, resolver);
        checker.check(*program);
        if ((expected.empty() &&
             (checker.had_errors() || !listener.errors.empty())) ||
            (!expected.empty() &&
             (!checker.had_errors() || listener.errors.size() != 1 ||
              listener.errors[0] != expected))) {
            std::cerr << "Unexpected result for: " << source << '\n';
            ++failures;
        }
    };
    check("fn f(value: i32) -> i32 {}", "");
    check("fn f(value: void) {}", "parameter type cannot be void");
    check("fn f() { let x = 1; }",
          "statement type checking is not implemented");
    check("fn f() { { let x = 1; } }",
          "statement type checking is not implemented");
    check("fn f(value: Point) {} struct Point { x: i32 }", "");
    check("fn f(value: &mut [[i32; (2 + 2), 8 / 2]; 3]) {}", "");
    check("fn f(value: [i32; 4 - 4]) {}",
          "array dimension must be greater than zero");
    check("fn f(value: [i32; 4 % 0]) {}",
          "array dimension arithmetic is invalid for usize");
    check("fn f(value: [i32; 18446744073709551615 + 1]) {}",
          "array dimension arithmetic is invalid for usize");
    check("fn f(value: [i32; true]) {}",
          "array dimension must be an integer constant expression");
    check("fn f(value: [void; 3]) {}", "array element type cannot be void");
    check("fn f(value: &void) {}", "referenced type cannot be void");
    check("struct Invalid { value: void }", "struct field type cannot be void");
    return failures ? 1 : 0;
}
