#pragma once

#include "span.hpp"
#include <string>
#include <variant>

namespace tl {

enum class TokenType {
    // Literals
    NUMBER,
    STRING,
    IDENTIFIER,
    
    // Keywords
    LET,
    VAR,
    FN,
    IF,
    ELSE,
    WHILE,
    FOR,
    BREAK,
    CONTINUE,
    RETURN,
    TRUE,
    FALSE,
    NIL,
    PRINT,
    
    // Operators
    PLUS,           // +
    MINUS,          // -
    STAR,           // *
    SLASH,          // /
    PERCENT,        // %
    BANG,           // !
    EQUAL,          // =
    EQUAL_EQUAL,    // ==
    BANG_EQUAL,     // !=
    LESS,           // <
    LESS_EQUAL,     // <=
    GREATER,        // >
    GREATER_EQUAL,  // >=
    AND,            // &&
    OR,             // ||
    
    // Punctuation
    LEFT_PAREN,     // (
    RIGHT_PAREN,    // )
    LEFT_BRACE,     // {
    RIGHT_BRACE,    // }
    SEMICOLON,      // ;
    COMMA,          // ,
    
    // Special
    EOF_TOKEN,
    ERROR
};

struct Token {
    TokenType type;
    std::string lexeme;
    Span span;
    
    // For literals
    std::variant<std::monostate, double, std::string> literal;

    Token() : type(TokenType::EOF_TOKEN) {}
    Token(TokenType type, const std::string& lexeme, const Span& span)
        : type(type), lexeme(lexeme), span(span) {}
    
    Token(TokenType type, const std::string& lexeme, const Span& span, double number)
        : type(type), lexeme(lexeme), span(span), literal(number) {}
    
    Token(TokenType type, const std::string& lexeme, const Span& span, const std::string& string)
        : type(type), lexeme(lexeme), span(span), literal(string) {}

    bool is_literal() const {
        return type == TokenType::NUMBER || type == TokenType::STRING ||
               type == TokenType::TRUE || type == TokenType::FALSE || type == TokenType::NIL;
    }

    bool is_operator() const {
        return type >= TokenType::PLUS && type <= TokenType::OR;
    }

    bool is_keyword() const {
        return type >= TokenType::LET && type <= TokenType::PRINT;
    }
};

} // namespace tl
