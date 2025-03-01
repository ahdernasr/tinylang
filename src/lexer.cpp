#include "tl/lexer.hpp"
#include "tl/error.hpp"
#include <unordered_map>
#include <cctype>

namespace tl {

const std::unordered_map<std::string, TokenType> Lexer::keywords_ = {
    {"let", TokenType::LET},
    {"var", TokenType::VAR},
    {"fn", TokenType::FN},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"for", TokenType::FOR},
    {"break", TokenType::BREAK},
    {"continue", TokenType::CONTINUE},
    {"return", TokenType::RETURN},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"nil", TokenType::NIL},
    {"print", TokenType::PRINT}
};

Lexer::Lexer(const std::string& source, ErrorReporter& error_reporter)
    : source_(source), start_(0), current_(0), line_(1), column_(1), error_reporter_(error_reporter) {
}

Token Lexer::scan_token() {
    skip_whitespace();
    
    start_ = current_;
    
    if (is_at_end()) {
        return make_token(TokenType::EOF_TOKEN);
    }
    
    char c = advance();
    
    if (is_alpha(c)) {
        return scan_identifier();
    }
    
    if (is_digit(c)) {
        return scan_number();
    }
    
    switch (c) {
        case '(': return make_token(TokenType::LEFT_PAREN);
        case ')': return make_token(TokenType::RIGHT_PAREN);
        case '{': return make_token(TokenType::LEFT_BRACE);
        case '}': return make_token(TokenType::RIGHT_BRACE);
        case ';': return make_token(TokenType::SEMICOLON);
        case ',': return make_token(TokenType::COMMA);
        case '-': return make_token(TokenType::MINUS);
        case '+': return make_token(TokenType::PLUS);
        case '/': return make_token(TokenType::SLASH);
        case '*': return make_token(TokenType::STAR);
        case '%': return make_token(TokenType::PERCENT);
        case '!': return make_token(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
        case '=': return make_token(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
        case '<': return make_token(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
        case '>': return make_token(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
        case '"': return scan_string();
        case '&': 
            if (match('&')) return make_token(TokenType::AND);
            break;
        case '|':
            if (match('|')) return make_token(TokenType::OR);
            break;
    }
    
    return error_token("Unexpected character.");
}

std::vector<Token> Lexer::scan_all() {
    std::vector<Token> tokens;
    
    while (!is_at_end()) {
        Token token = scan_token();
        tokens.push_back(token);
        
        if (token.type == TokenType::EOF_TOKEN) {
            break;
        }
    }
    
    return tokens;
}

bool Lexer::is_at_end() const {
    return current_ >= source_.length();
}

char Lexer::advance() {
    if (is_at_end()) return '\0';
    char c = source_[current_++];
    if (c == '\n') {
        line_++;
        column_ = 1;
    } else {
        column_++;
    }
    return c;
}

char Lexer::peek() const {
    if (is_at_end()) return '\0';
    return source_[current_];
}

char Lexer::peek_next() const {
    if (current_ + 1 >= source_.length()) return '\0';
    return source_[current_ + 1];
}

bool Lexer::match(char expected) {
    if (is_at_end()) return false;
    if (source_[current_] != expected) return false;
    
    current_++;
    column_++;
    return true;
}

void Lexer::skip_whitespace() {
    while (true) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                advance();
                break;
            case '/':
                if (peek_next() == '/') {
                    skip_line_comment();
                } else if (peek_next() == '*') {
                    skip_block_comment();
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

void Lexer::skip_line_comment() {
    while (peek() != '\n' && !is_at_end()) {
        advance();
    }
}

void Lexer::skip_block_comment() {
    advance(); // consume '*'
    
    while (!is_at_end()) {
        if (peek() == '*' && peek_next() == '/') {
            advance(); // consume '*'
            advance(); // consume '/'
            return;
        }
        advance();
    }
    
    error_reporter_.report(ErrorType::LEXICAL, "Unterminated block comment", current_span());
}

Token Lexer::make_token(TokenType type) {
    return Token(type, source_.substr(start_, current_ - start_), current_span());
}

Token Lexer::make_token(TokenType type, double number) {
    return Token(type, source_.substr(start_, current_ - start_), current_span(), number);
}

Token Lexer::make_token(TokenType type, const std::string& string) {
    return Token(type, source_.substr(start_, current_ - start_), current_span(), string);
}

Token Lexer::error_token(const std::string& message) {
    error_reporter_.report(ErrorType::LEXICAL, message, current_span());
    return Token(TokenType::ERROR, message, current_span());
}

Token Lexer::scan_string() {
    while (peek() != '"' && !is_at_end()) {
        if (peek() == '\n') {
            line_++;
            column_ = 1;
        }
        advance();
    }
    
    if (is_at_end()) {
        return error_token("Unterminated string.");
    }
    
    advance(); // consume closing quote
    
    // Remove quotes and process escape sequences
    std::string value = source_.substr(start_ + 1, current_ - start_ - 2);
    std::string processed;
    processed.reserve(value.length());
    
    for (size_t i = 0; i < value.length(); i++) {
        if (value[i] == '\\' && i + 1 < value.length()) {
            switch (value[i + 1]) {
                case 'n': processed += '\n'; break;
                case 't': processed += '\t'; break;
                case 'r': processed += '\r'; break;
                case '\\': processed += '\\'; break;
                case '"': processed += '"'; break;
                default: processed += value[i + 1]; break;
            }
            i++; // skip the escape character
        } else {
            processed += value[i];
        }
    }
    
    return make_token(TokenType::STRING, processed);
}

Token Lexer::scan_number() {
    while (is_digit(peek())) {
        advance();
    }
    
    // Look for fractional part
    if (peek() == '.' && is_digit(peek_next())) {
        advance(); // consume '.'
        
        while (is_digit(peek())) {
            advance();
        }
    }
    
    double value = std::stod(source_.substr(start_, current_ - start_));
    return make_token(TokenType::NUMBER, value);
}

Token Lexer::scan_identifier() {
    while (is_alphanumeric(peek())) {
        advance();
    }
    
    std::string text = source_.substr(start_, current_ - start_);
    
    auto it = keywords_.find(text);
    TokenType type = (it != keywords_.end()) ? it->second : TokenType::IDENTIFIER;
    
    return make_token(type);
}

bool Lexer::is_digit(char c) const {
    return std::isdigit(c);
}

bool Lexer::is_alpha(char c) const {
    return std::isalpha(c) || c == '_';
}

bool Lexer::is_alphanumeric(char c) const {
    return is_alpha(c) || is_digit(c);
}

Span Lexer::current_span() const {
    return Span(start_, current_, line_, column_ - (current_ - start_));
}

} // namespace tl
