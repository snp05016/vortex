#pragma once
#include "../../lexer/token.h"
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
enum class ParserErrorKind : std::uint8_t { // sum fuckass warning that llvm
                                            // linter was giving change the
                                            // default size of each var in the
                                            // enum to 1 byte to save memory
    Expected,
    Unexpected,
    Invalid,
};

class ParserError : public std::runtime_error {
  public:
    /// creates a parser error with its category, source location, and message.
    /// for a missing `;` after `let value = 1`, it keeps the expected location.
    /// callers can still read the message through the standard exception
    /// interface.
    ParserError(ParserErrorKind kind, SourceLocation location,
                const std::string &message)
        : std::runtime_error(message), kind_(kind), location_(location) {
    }

    /// returns the broad category assigned to this parser error.
    /// for an empty `point {}` expression, the category is invalid syntax.
    /// the category lets diagnostics react without comparing message text.
    [[nodiscard]] ParserErrorKind kind() const {
        return kind_;
    }

    /// returns the source span associated with this parser error.
    /// for `let = 1;`, it points where the variable name was expected.
    /// the span uses byte offsets from the original source buffer.
    [[nodiscard]] SourceLocation location() const {
        return location_;
    }

    /// raises an error for syntax that requires a missing token or construct.
    /// for `let value 1;`, it reports that `=` was expected.
    /// optional context is appended to keep the final message natural.
    [[noreturn]] static void expected(SourceLocation location,
                                      std::string_view expected_value,
                                      std::string_view context = {}) {
        std::string message = "expected ";
        message += expected_value;
        if (!context.empty()) {
            message += ' ';
            message += context;
        }
        throw ParserError(ParserErrorKind::Expected, location, message);
    }

    /// raises an error when the current token cannot begin the requested
    /// syntax. for `let value = );`, it reports an unexpected token in the
    /// expression. this function never returns, so parsing stops at the
    /// reported location.
    [[noreturn]] static void unexpected(SourceLocation location,
                                        std::string_view value) {
        std::string message = "unexpected ";
        message += value;
        throw ParserError(ParserErrorKind::Unexpected, location, message);
    }

    /// raises an error for syntax that has the right pieces in a forbidden
    /// form. for `left < middle < right`, it reports an invalid comparison
    /// chain. this function never returns and preserves the location of the
    /// invalid form.
    [[noreturn]] static void invalid(SourceLocation location,
                                     std::string_view value) {
        std::string message = "invalid ";
        message += value;
        throw ParserError(ParserErrorKind::Invalid, location, message);
    }

  private:
    ParserErrorKind kind_;
    SourceLocation location_;
};
