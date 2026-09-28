#include "diagnostic_listener.h"

#include <algorithm>
#include <ostream>
#include <string>

namespace {
const char *severity_name(DiagnosticSeverity severity) {
    switch (severity) {
    case DiagnosticSeverity::Error:
        return "error";
    case DiagnosticSeverity::Warning:
        return "warning";
    case DiagnosticSeverity::Note:
        return "note";
    }
    return "diagnostic";
}

std::string caret_padding(std::string_view line, std::size_t column_offset) {
    std::string padding;
    padding.reserve(column_offset);
    for (std::size_t index = 0; index < column_offset; ++index) {
        padding += line[index] == '\t' ? '\t' : ' ';
    }
    return padding;
}
} // namespace

TextDiagnosticListener::TextDiagnosticListener(std::ostream &output,
                                               std::string_view file_name,
                                               std::string_view source)
    : output_(output), file_name_(file_name), source_(source) {
}

void TextDiagnosticListener::report(const Diagnostic &diagnostic) {
    const std::size_t start =
        std::min(diagnostic.location.start, source_.size());
    const std::size_t preceding_newline =
        start == 0 ? std::string_view::npos : source_.rfind('\n', start - 1);
    const std::size_t line_start =
        preceding_newline == std::string_view::npos ? 0 : preceding_newline + 1;
    const std::size_t newline = source_.find('\n', start);
    const std::size_t line_end =
        newline == std::string_view::npos ? source_.size() : newline;
    const std::size_t line_number =
        1 + static_cast<std::size_t>(
                std::count(source_.begin(), source_.begin() + start, '\n'));
    const std::size_t column_number = start - line_start + 1;
    const std::string_view source_line =
        source_.substr(line_start, line_end - line_start);

    std::size_t marker_length = 1;
    if (start < line_end && diagnostic.location.length > 0) {
        marker_length = std::min(diagnostic.location.length, line_end - start);
    }

    const std::string line_label = std::to_string(line_number);
    const std::string gutter(line_label.size(), ' ');
    output_ << file_name_ << ':' << line_number << ':' << column_number << ": "
            << severity_name(diagnostic.severity) << ": " << diagnostic.message
            << '\n';
    output_ << gutter << " |\n";
    output_ << line_label << " | " << source_line << '\n';
    output_ << gutter << " | " << caret_padding(source_line, start - line_start)
            << '^';
    if (marker_length > 1) {
        output_ << std::string(marker_length - 1, '~');
    }
    output_ << '\n';
}
