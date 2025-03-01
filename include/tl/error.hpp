#pragma once

#include "span.hpp"
#include <string>
#include <vector>
#include <stdexcept>

namespace tl {

enum class ErrorType {
    LEXICAL,
    SYNTAX,
    SEMANTIC,
    RUNTIME,
    COMPILATION
};

class Error : public std::runtime_error {
public:
    ErrorType type;
    Span span;
    std::string source_line;

    Error(ErrorType type, const std::string& message, const Span& span = Span())
        : std::runtime_error(message), type(type), span(span) {}

    Error(ErrorType type, const std::string& message, const Span& span, const std::string& source_line)
        : std::runtime_error(message), type(type), span(span), source_line(source_line) {}
};

class ErrorReporter {
private:
    std::vector<Error> errors_;
    std::string source_;

public:
    void set_source(const std::string& source) { source_ = source; }
    
    void report(ErrorType type, const std::string& message, const Span& span = Span()) {
        std::string source_line;
        if (!source_.empty() && span.line > 0) {
            source_line = get_line_at(span.line);
        }
        errors_.emplace_back(type, message, span, source_line);
    }

    bool has_errors() const { return !errors_.empty(); }
    const std::vector<Error>& errors() const { return errors_; }
    void clear() { errors_.clear(); }

    std::string format_error(const Error& error) const;

private:
    std::string get_line_at(std::size_t line_num) const;
};

} // namespace tl
