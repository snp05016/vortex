#include "frontend/diagnostics/diagnostic_listener.h"
#include "frontend/parser/parser.h"
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

namespace {
int failures = 0;

void fail(std::string_view test_name, std::string_view message) {
    std::cerr << "[FAIL] " << test_name << ": " << message << '\n';
    failures++;
}

void expect_contains(std::string_view test_name, const std::string &output,
                     std::string_view expected) {
    if (!output.contains(expected)) {
        fail(test_name, std::string("missing '") + std::string(expected) + "'");
    }
}

void test_parser_error_rendering() {
    const std::string source = "fn main() {\n    let value = ;\n}\n";
    try {
        Parser parser(source.data(), source.size());
        auto const program = parser.parse();
        (void)program;
        fail("parser diagnostic", "invalid source was accepted");
    } catch (const ParserError &error) {
        std::ostringstream output;
        TextDiagnosticListener listener(output, "broken.vx", source);
        listener.report(
            {DiagnosticSeverity::Error, error.location(), error.what()});
        const std::string rendered = output.str();
        expect_contains("file line column", rendered, "broken.vx:2:17: error:");
        expect_contains("message", rendered,
                        "unexpected token in primary expression");
        expect_contains("source line", rendered, "2 |     let value = ;");
        expect_contains("caret", rendered, "  |                 ^");
    }
}

void test_span_and_severities() {
    const std::string source = "alpha beta\n";
    std::ostringstream output;
    TextDiagnosticListener listener(output, "example.vx", source);
    listener.report(
        {DiagnosticSeverity::Warning, SourceLocation{6, 4}, "example warning"});
    listener.report(
        {DiagnosticSeverity::Note, SourceLocation{0, 5}, "example note"});
    const std::string rendered = output.str();
    expect_contains("warning severity", rendered,
                    "example.vx:1:7: warning: example warning");
    expect_contains("warning span", rendered, "|       ^~~~");
    expect_contains("note severity", rendered,
                    "example.vx:1:1: note: example note");
    expect_contains("note span", rendered, "| ^~~~~");
}

void test_end_of_file_location() {
    const std::string source = "fn main() {}";
    std::ostringstream output;
    TextDiagnosticListener listener(output, "eof.vx", source);
    listener.report({DiagnosticSeverity::Error,
                     SourceLocation{source.size(), 0}, "expected declaration"});
    const std::string rendered = output.str();
    expect_contains("eof location", rendered,
                    "eof.vx:1:13: error: expected declaration");
    expect_contains("eof caret", rendered, "|             ^");
}
} // namespace

int main() {
    test_parser_error_rendering();
    test_span_and_severities();
    test_end_of_file_location();
    if (failures != 0) {
        std::cerr << failures << " diagnostic listener test(s) failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "All diagnostic listener tests passed\n";
    return EXIT_SUCCESS;
}
