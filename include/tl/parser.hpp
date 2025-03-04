#pragma once

#include "ast.hpp"
#include "token.hpp"
#include "error.hpp"
#include <vector>
#include <memory>

namespace tl {

class Parser {
private:
    std::vector<Token> tokens_;
    std::size_t current_;
    ErrorReporter& error_reporter_;

public:
    Parser(const std::vector<Token>& tokens, ErrorReporter& error_reporter);
    
    std::unique_ptr<Program> parse();
    
private:
    // Parsing methods
    std::unique_ptr<Stmt> declaration();
    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> var_declaration();
    std::unique_ptr<Stmt> function_declaration();
    std::unique_ptr<Stmt> if_statement();
    std::unique_ptr<Stmt> while_statement();
    std::unique_ptr<Stmt> for_statement();
    std::unique_ptr<Stmt> break_statement();
    std::unique_ptr<Stmt> continue_statement();
    std::unique_ptr<Stmt> return_statement();
    std::unique_ptr<Stmt> block_statement();
    std::unique_ptr<Stmt> expression_statement();
    
    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> assignment();
    std::unique_ptr<Expr> logic_or();
    std::unique_ptr<Expr> logic_and();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    std::unique_ptr<Expr> unary();
    std::unique_ptr<Expr> call();
    std::unique_ptr<Expr> finish_call(std::unique_ptr<Expr> callee);
    std::unique_ptr<Expr> primary();
    
    // Utility methods
    bool is_at_end() const;
    Token advance();
    Token peek() const;
    Token previous() const;
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool match_any(const std::vector<TokenType>& types);
    
    Token consume(TokenType type, const std::string& message);
    void synchronize();
    
    // Error handling
    void error(const Token& token, const std::string& message);
    void error_at_current(const std::string& message);
};

} // namespace tl
