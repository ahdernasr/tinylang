#include "tl/error.hpp"
#include <sstream>
#include <algorithm>

namespace tl {

std::string ErrorReporter::format_error(const Error& error) const {
    std::ostringstream oss;
    
    // Error type prefix
    switch (error.type) {
        case ErrorType::LEXICAL:
            oss << "[LEXICAL ERROR]";
            break;
        case ErrorType::SYNTAX:
            oss << "[SYNTAX ERROR]";
            break;
        case ErrorType::SEMANTIC:
            oss << "[SEMANTIC ERROR]";
            break;
        case ErrorType::RUNTIME:
            oss << "[RUNTIME ERROR]";
            break;
        case ErrorType::COMPILATION:
            oss << "[COMPILATION ERROR]";
            break;
    }
    
    // Location information
    if (error.span.line > 0) {
        oss << " at line " << error.span.line;
        if (error.span.column > 0) {
            oss << ", column " << error.span.column;
        }
    }
    
    oss << ": " << error.what();
    
    // Source line with caret
    if (!error.source_line.empty()) {
        oss << "\n" << error.source_line << "\n";
        
        // Add caret pointing to the error location
        std::string caret(error.span.column - 1, ' ');
        caret += "^";
        oss << caret;
    }
    
    return oss.str();
}

std::string ErrorReporter::get_line_at(std::size_t line_num) const {
    if (source_.empty() || line_num == 0) {
        return "";
    }
    
    std::size_t current_line = 1;
    std::size_t line_start = 0;
    
    for (std::size_t i = 0; i < source_.length(); i++) {
        if (source_[i] == '\n') {
            if (current_line == line_num) {
                // Found the line, extract it
                std::size_t line_end = i;
                return source_.substr(line_start, line_end - line_start);
            }
            current_line++;
            line_start = i + 1;
        }
    }
    
    // Handle the last line (no trailing newline)
    if (current_line == line_num && line_start < source_.length()) {
        return source_.substr(line_start);
    }
    
    return "";
}

} // namespace tl
