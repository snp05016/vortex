#pragma once
#include "../../token.h"
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
enum class ParserErrorKind {
    Expected,
    Unexpected,
    Invalid,
};

class ParserError : public std::runtime_error {
  public:
    ParserError(ParserErrorKind kind, SourceLocation location,
                std::string message)
        : std::runtime_error(std::move(message)), kind_(kind),
          location_(location) {
    }

    ParserErrorKind kind() const {
        return kind_;
    }

    SourceLocation location() const {
        return location_;
    }

  private:
    ParserErrorKind kind_;
    SourceLocation location_;
};

namespace parser_errors {
[[noreturn]] inline void expected(SourceLocation location,
                                  std::string_view expected_value,
                                  std::string_view context = {}) {
    std::string message = "expected ";
    message += expected_value;
    if (!context.empty()) {
        message += " ";
        message += context;
    }
    throw ParserError(ParserErrorKind::Expected, location, std::move(message));
}

[[noreturn]] inline void unexpected(SourceLocation location,
                                    std::string_view value) {
    std::string message = "unexpected ";
    message += value;
    throw ParserError(ParserErrorKind::Unexpected, location,
                      std::move(message));
}

[[noreturn]] inline void invalid(SourceLocation location,
                                 std::string_view value) {
    std::string message = "invalid ";
    message += value;
    throw ParserError(ParserErrorKind::Invalid, location, std::move(message));
}
} // namespace parser_errors
