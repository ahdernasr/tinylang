#pragma once

#include "token.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace tl {

class Lexer {
public:
    explicit Lexer(std::string source);

    std::vector<Token> tokenize();

private:
    std::string source_;
    std::size_t start_;
    std::size_t current_;
    int line_;

    std::vector<Token> tokens_;

    bool is_at_end() const;
    char advance();
    char peek() const;
    char peek_next() const;
    bool match(char expected);

    void scan_token();
    void string();
    void number();
    void identifier();
    void add_token(TokenType type, Literal literal = {});

    bool is_digit(char c) const;
    bool is_alpha(char c) const;
    bool is_alphanumeric(char c) const;

    static const std::unordered_map<std::string, TokenType> keywords_;
};

} // namespace tl

