#pragma once

#include "token.hpp"
#include "error.hpp"
#include <string>

namespace tl {

class Lexer {
private:
    std::string source_;
    std::size_t start_;
    std::size_t current_;
    std::size_t line_;
    std::size_t column_;
    ErrorReporter& error_reporter_;

    static const std::unordered_map<std::string, TokenType> keywords_;

public:
    Lexer(const std::string& source, ErrorReporter& error_reporter);
    
    Token scan_token();
    std::vector<Token> scan_all();
    
private:
    bool is_at_end() const;
    char advance();
    char peek() const;
    char peek_next() const;
    bool match(char expected);
    
    void skip_whitespace();
    void skip_line_comment();
    void skip_block_comment();
    
    Token make_token(TokenType type);
    Token make_token(TokenType type, double number);
    Token make_token(TokenType type, const std::string& string);
    Token error_token(const std::string& message);
    
    Token scan_string();
    Token scan_number();
    Token scan_identifier();
    
    bool is_digit(char c) const;
    bool is_alpha(char c) const;
    bool is_alphanumeric(char c) const;
    
    Span current_span() const;
};

} // namespace tl
