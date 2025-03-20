#pragma once

#include <string>
#include <variant>

namespace tl {

enum class TokenType {
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    COMMA,
    DOT,
    MINUS,
    PLUS,
    SEMICOLON,
    SLASH,
    STAR,
    BANG,
    BANG_EQUAL,
    EQUAL,
    EQUAL_EQUAL,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL,
    IDENTIFIER,
    STRING,
    NUMBER,
    TRUE,
    FALSE,
    NIL,
    LET,
    PRINT,
    IF,
    ELSE,
    WHILE,
    AND,
    OR,
    END_OF_FILE
};

using Literal = std::variant<std::monostate, double, std::string>;

struct Token {
    TokenType type;
    std::string lexeme;
    Literal literal;
    int line;

    Token(TokenType type, std::string lexeme, Literal literal, int line)
        : type(type), lexeme(std::move(lexeme)), literal(std::move(literal)), line(line) {}
};

inline std::string token_type_to_string(TokenType type) {
    switch (type) {
        case TokenType::LEFT_PAREN: return "(";
        case TokenType::RIGHT_PAREN: return ")";
        case TokenType::LEFT_BRACE: return "{";
        case TokenType::RIGHT_BRACE: return "}";
        case TokenType::COMMA: return ",";
        case TokenType::DOT: return ".";
        case TokenType::MINUS: return "-";
        case TokenType::PLUS: return "+";
        case TokenType::SEMICOLON: return ";";
        case TokenType::SLASH: return "/";
        case TokenType::STAR: return "*";
        case TokenType::BANG: return "!";
        case TokenType::BANG_EQUAL: return "!=";
        case TokenType::EQUAL: return "=";
        case TokenType::EQUAL_EQUAL: return "==";
        case TokenType::GREATER: return ">";
        case TokenType::GREATER_EQUAL: return ">=";
        case TokenType::LESS: return "<";
        case TokenType::LESS_EQUAL: return "<=";
        case TokenType::IDENTIFIER: return "identifier";
        case TokenType::STRING: return "string";
        case TokenType::NUMBER: return "number";
        case TokenType::TRUE: return "true";
        case TokenType::FALSE: return "false";
        case TokenType::NIL: return "nil";
        case TokenType::LET: return "let";
        case TokenType::PRINT: return "print";
        case TokenType::IF: return "if";
        case TokenType::ELSE: return "else";
        case TokenType::WHILE: return "while";
        case TokenType::AND: return "and";
        case TokenType::OR: return "or";
        case TokenType::END_OF_FILE: return "EOF";
    }
    return "unknown";
}

} // namespace tl

