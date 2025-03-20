#include "tl/lexer.hpp"

#include <cctype>
#include <stdexcept>

namespace tl {

const std::unordered_map<std::string, TokenType> Lexer::keywords_ = {
    {"let", TokenType::LET},
    {"print", TokenType::PRINT},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"nil", TokenType::NIL},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"and", TokenType::AND},
    {"or", TokenType::OR}
};

Lexer::Lexer(std::string source)
    : source_(std::move(source)), start_(0), current_(0), line_(1) {}

std::vector<Token> Lexer::tokenize() {
    while (!is_at_end()) {
        start_ = current_;
        scan_token();
    }

    tokens_.emplace_back(TokenType::END_OF_FILE, "", Literal{}, line_);
    return tokens_;
}

bool Lexer::is_at_end() const {
    return current_ >= source_.size();
}

char Lexer::advance() {
    return source_[current_++];
}

char Lexer::peek() const {
    if (is_at_end()) return '\0';
    return source_[current_];
}

char Lexer::peek_next() const {
    if (current_ + 1 >= source_.size()) return '\0';
    return source_[current_ + 1];
}

bool Lexer::match(char expected) {
    if (is_at_end()) return false;
    if (source_[current_] != expected) return false;
    current_++;
    return true;
}

void Lexer::scan_token() {
    char c = advance();
    switch (c) {
        case '(': add_token(TokenType::LEFT_PAREN); break;
        case ')': add_token(TokenType::RIGHT_PAREN); break;
        case '{': add_token(TokenType::LEFT_BRACE); break;
        case '}': add_token(TokenType::RIGHT_BRACE); break;
        case ',': add_token(TokenType::COMMA); break;
        case '.': add_token(TokenType::DOT); break;
        case '-': add_token(TokenType::MINUS); break;
        case '+': add_token(TokenType::PLUS); break;
        case ';': add_token(TokenType::SEMICOLON); break;
        case '*': add_token(TokenType::STAR); break;
        case '!':
            add_token(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
            break;
        case '=':
            add_token(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
            break;
        case '<':
            add_token(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
            break;
        case '>':
            add_token(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
            break;
        case '/':
            if (match('/')) {
                while (peek() != '\n' && !is_at_end()) advance();
            } else {
                add_token(TokenType::SLASH);
            }
            break;
        case ' ':
        case '\r':
        case '\t':
            break;
        case '\n':
            line_++;
            break;
        case '"':
            string();
            break;
        default:
            if (is_digit(c)) {
                number();
            } else if (is_alpha(c)) {
                identifier();
            } else {
                throw std::runtime_error("Unexpected character at line " + std::to_string(line_));
            }
            break;
    }
}

void Lexer::string() {
    while (peek() != '"' && !is_at_end()) {
        if (peek() == '\n') line_++;
        advance();
    }

    if (is_at_end()) {
        throw std::runtime_error("Unterminated string at line " + std::to_string(line_));
    }

    advance(); // closing quote

    std::string value = source_.substr(start_ + 1, current_ - start_ - 2);
    add_token(TokenType::STRING, value);
}

void Lexer::number() {
    while (is_digit(peek())) advance();

    if (peek() == '.' && is_digit(peek_next())) {
        advance();
        while (is_digit(peek())) advance();
    }

    double value = std::stod(source_.substr(start_, current_ - start_));
    add_token(TokenType::NUMBER, value);
}

void Lexer::identifier() {
    while (is_alphanumeric(peek())) advance();

    std::string text = source_.substr(start_, current_ - start_);
    auto it = keywords_.find(text);
    if (it != keywords_.end()) {
        add_token(it->second);
    } else {
        add_token(TokenType::IDENTIFIER);
    }
}

void Lexer::add_token(TokenType type, Literal literal) {
    std::string text = source_.substr(start_, current_ - start_);
    tokens_.emplace_back(type, std::move(text), std::move(literal), line_);
}

bool Lexer::is_digit(char c) const {
    return std::isdigit(static_cast<unsigned char>(c));
}

bool Lexer::is_alpha(char c) const {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

bool Lexer::is_alphanumeric(char c) const {
    return is_digit(c) || is_alpha(c);
}

} // namespace tl

