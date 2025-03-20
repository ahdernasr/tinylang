#pragma once

#include "ast.hpp"
#include "lexer.hpp"
#include "token.hpp"

#include <stdexcept>
#include <vector>

namespace tl {

class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string& message)
        : std::runtime_error(message) {}
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    std::vector<StmtPtr> parse();

private:
    std::vector<Token> tokens_;
    std::size_t current_;

    const Token& peek() const;
    const Token& previous() const;
    bool is_at_end() const;

    const Token& advance();
    bool check(TokenType type) const;
    bool match(std::initializer_list<TokenType> types);
    const Token& consume(TokenType type, const std::string& message);

    // Grammar rules
    StmtPtr declaration();
    StmtPtr let_declaration();
    StmtPtr statement();
    StmtPtr print_statement();
    StmtPtr expression_statement();
    StmtPtr block_statement();
    StmtPtr if_statement();
    StmtPtr while_statement();

    ExprPtr expression();
    ExprPtr assignment();
    ExprPtr or_expression();
    ExprPtr and_expression();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr unary();
    ExprPtr primary();

    void synchronize();
};

} // namespace tl

