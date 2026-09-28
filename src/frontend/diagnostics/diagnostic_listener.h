#pragma once

#include "../lexer/token.h"
#include <iosfwd>
#include <string>
#include <string_view>

enum class DiagnosticSeverity { Error, Warning, Note };

struct Diagnostic {
    DiagnosticSeverity severity;
    SourceLocation location;
    std::string message;
};

class DiagnosticListener {
  public:
    virtual ~DiagnosticListener() = default;
    virtual void report(const Diagnostic &diagnostic) = 0;
};

class TextDiagnosticListener final : public DiagnosticListener {
  public:
    TextDiagnosticListener(std::ostream &output, std::string_view file_name,
                           std::string_view source);

    void report(const Diagnostic &diagnostic) override;

  private:
    std::ostream &output_;
    std::string_view file_name_;
    std::string_view source_;
};
