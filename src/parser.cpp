#include "tl/parser.hpp"
#include "tl/error.hpp"
#include <algorithm>

namespace tl {

Parser::Parser(const std::vector<Token>& tokens, ErrorReporter& error_reporter)
    : tokens_(tokens), current_(0), error_reporter_(error_reporter) {
}

std::unique_ptr<Program> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> statements;
    
    while (!is_at_end()) {
        try {
            statements.push_back(declaration());
        } catch (const Error& e) {
            synchronize();
        }
    }
    
    return std::make_unique<Program>(Span(), std::move(statements));
}

std::unique_ptr<Stmt> Parser::declaration() {
    if (match(TokenType::FN)) {
        return function_declaration();
    }
    
    if (match(TokenType::LET) || match(TokenType::VAR)) {
        return var_declaration();
    }
    
    return statement();
}

std::unique_ptr<Stmt> Parser::statement() {
    if (match(TokenType::IF)) {
        return if_statement();
    }
    
    if (match(TokenType::WHILE)) {
        return while_statement();
    }
    
    if (match(TokenType::FOR)) {
        return for_statement();
    }
    
    if (match(TokenType::BREAK)) {
        return break_statement();
    }
    
    if (match(TokenType::CONTINUE)) {
        return continue_statement();
    }
    
    if (match(TokenType::RETURN)) {
        return return_statement();
    }
    
    if (match(TokenType::LEFT_BRACE)) {
        return block_statement();
    }
    
    return expression_statement();
}

std::unique_ptr<Stmt> Parser::var_declaration() {
    TokenType keyword = previous().type;
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name.");
    
    std::unique_ptr<Expr> initializer = nullptr;
    if (match(TokenType::EQUAL)) {
        initializer = expression();
    }
    
    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration.");
    
    return std::make_unique<VarStmt>(
        name.span,
        name.lexeme,
        keyword == TokenType::VAR,
        std::move(initializer)
    );
}

std::unique_ptr<Stmt> Parser::function_declaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expected function name.");
    consume(TokenType::LEFT_PAREN, "Expected '(' after function name.");
    
    std::vector<std::string> parameters;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            if (parameters.size() >= 255) {
                error_at_current("Can't have more than 255 parameters.");
            }
            
            Token param = consume(TokenType::IDENTIFIER, "Expected parameter name.");
            parameters.push_back(param.lexeme);
        } while (match(TokenType::COMMA));
    }
    
    consume(TokenType::RIGHT_PAREN, "Expected ')' after parameters.");
    consume(TokenType::LEFT_BRACE, "Expected '{' before function body.");
    
    std::unique_ptr<BlockStmt> body = block_statement();
    
    return std::make_unique<FunctionStmt>(
        name.span,
        name.lexeme,
        std::move(parameters),
        std::move(body)
    );
}

std::unique_ptr<Stmt> Parser::if_statement() {
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'if'.");
    std::unique_ptr<Expr> condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after if condition.");
    
    std::unique_ptr<Stmt> then_branch = statement();
    std::unique_ptr<Stmt> else_branch = nullptr;
    
    if (match(TokenType::ELSE)) {
        else_branch = statement();
    }
    
    return std::make_unique<IfStmt>(
        previous().span,
        std::move(condition),
        std::move(then_branch),
        std::move(else_branch)
    );
}

std::unique_ptr<Stmt> Parser::while_statement() {
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'while'.");
    std::unique_ptr<Expr> condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after while condition.");
    
    std::unique_ptr<Stmt> body = statement();
    
    return std::make_unique<WhileStmt>(
        previous().span,
        std::move(condition),
        std::move(body)
    );
}

std::unique_ptr<Stmt> Parser::for_statement() {
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'for'.");
    
    std::unique_ptr<Stmt> initializer = nullptr;
    if (match(TokenType::SEMICOLON)) {
        // No initializer
    } else if (match(TokenType::LET) || match(TokenType::VAR)) {
        initializer = var_declaration();
    } else {
        initializer = expression_statement();
    }
    
    std::unique_ptr<Expr> condition = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        condition = expression();
    }
    consume(TokenType::SEMICOLON, "Expected ';' after loop condition.");
    
    std::unique_ptr<Expr> increment = nullptr;
    if (!check(TokenType::RIGHT_PAREN)) {
        increment = expression();
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' after for clauses.");
    
    std::unique_ptr<Stmt> body = statement();
    
    // Desugar for loop to while loop
    if (increment != nullptr) {
        std::vector<std::unique_ptr<Stmt>> block_statements;
        block_statements.push_back(std::move(body));
        block_statements.push_back(std::make_unique<ExprStmt>(
            increment->span,
            std::move(increment)
        ));
        body = std::make_unique<BlockStmt>(
            body->span,
            std::move(block_statements)
        );
    }
    
    if (condition == nullptr) {
        condition = std::make_unique<LiteralExpr>(
            Span(),
            make_bool(true)
        );
    }
    
    body = std::make_unique<WhileStmt>(
        body->span,
        std::move(condition),
        std::move(body)
    );
    
    if (initializer != nullptr) {
        std::vector<std::unique_ptr<Stmt>> block_statements;
        block_statements.push_back(std::move(initializer));
        block_statements.push_back(std::move(body));
        body = std::make_unique<BlockStmt>(
            body->span,
            std::move(block_statements)
        );
    }
    
    return body;
}

std::unique_ptr<Stmt> Parser::break_statement() {
    consume(TokenType::SEMICOLON, "Expected ';' after 'break'.");
    return std::make_unique<BreakStmt>(previous().span);
}

std::unique_ptr<Stmt> Parser::continue_statement() {
    consume(TokenType::SEMICOLON, "Expected ';' after 'continue'.");
    return std::make_unique<ContinueStmt>(previous().span);
}

std::unique_ptr<Stmt> Parser::return_statement() {
    std::unique_ptr<Expr> value = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        value = expression();
    }
    
    consume(TokenType::SEMICOLON, "Expected ';' after return value.");
    return std::make_unique<ReturnStmt>(previous().span, std::move(value));
}

std::unique_ptr<BlockStmt> Parser::block_statement() {
    std::vector<std::unique_ptr<Stmt>> statements;
    
    while (!check(TokenType::RIGHT_BRACE) && !is_at_end()) {
        statements.push_back(declaration());
    }
    
    consume(TokenType::RIGHT_BRACE, "Expected '}' after block.");
    return std::make_unique<BlockStmt>(previous().span, std::move(statements));
}

std::unique_ptr<Stmt> Parser::expression_statement() {
    std::unique_ptr<Expr> expr = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression.");
    return std::make_unique<ExprStmt>(expr->span, std::move(expr));
}

std::unique_ptr<Expr> Parser::expression() {
    return assignment();
}

std::unique_ptr<Expr> Parser::assignment() {
    std::unique_ptr<Expr> expr = logic_or();
    
    if (match(TokenType::EQUAL)) {
        Token equals = previous();
        std::unique_ptr<Expr> value = assignment();
        
        if (auto* variable = dynamic_cast<VariableExpr*>(expr.get())) {
            return std::make_unique<AssignExpr>(
                equals.span,
                variable->name,
                std::move(value)
            );
        }
        
        error(equals, "Invalid assignment target.");
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::logic_or() {
    std::unique_ptr<Expr> expr = logic_and();
    
    while (match(TokenType::OR)) {
        Token op = previous();
        std::unique_ptr<Expr> right = logic_and();
        expr = std::make_unique<BinaryExpr>(
            op.span,
            std::move(expr),
            op.type,
            std::move(right)
        );
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::logic_and() {
    std::unique_ptr<Expr> expr = equality();
    
    while (match(TokenType::AND)) {
        Token op = previous();
        std::unique_ptr<Expr> right = equality();
        expr = std::make_unique<BinaryExpr>(
            op.span,
            std::move(expr),
            op.type,
            std::move(right)
        );
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::equality() {
    std::unique_ptr<Expr> expr = comparison();
    
    while (match_any({TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL})) {
        Token op = previous();
        std::unique_ptr<Expr> right = comparison();
        expr = std::make_unique<BinaryExpr>(
            op.span,
            std::move(expr),
            op.type,
            std::move(right)
        );
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
    std::unique_ptr<Expr> expr = term();
    
    while (match_any({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL})) {
        Token op = previous();
        std::unique_ptr<Expr> right = term();
        expr = std::make_unique<BinaryExpr>(
            op.span,
            std::move(expr),
            op.type,
            std::move(right)
        );
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::term() {
    std::unique_ptr<Expr> expr = factor();
    
    while (match_any({TokenType::PLUS, TokenType::MINUS})) {
        Token op = previous();
        std::unique_ptr<Expr> right = factor();
        expr = std::make_unique<BinaryExpr>(
            op.span,
            std::move(expr),
            op.type,
            std::move(right)
        );
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::factor() {
    std::unique_ptr<Expr> expr = unary();
    
    while (match_any({TokenType::STAR, TokenType::SLASH, TokenType::PERCENT})) {
        Token op = previous();
        std::unique_ptr<Expr> right = unary();
        expr = std::make_unique<BinaryExpr>(
            op.span,
            std::move(expr),
            op.type,
            std::move(right)
        );
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::unary() {
    if (match_any({TokenType::BANG, TokenType::MINUS})) {
        Token op = previous();
        std::unique_ptr<Expr> right = unary();
        return std::make_unique<UnaryExpr>(
            op.span,
            op.type,
            std::move(right)
        );
    }
    
    return call();
}

std::unique_ptr<Expr> Parser::call() {
    std::unique_ptr<Expr> expr = primary();
    
    while (true) {
        if (match(TokenType::LEFT_PAREN)) {
            expr = finish_call(std::move(expr));
        } else {
            break;
        }
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::finish_call(std::unique_ptr<Expr> callee) {
    std::vector<std::unique_ptr<Expr>> arguments;
    
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            if (arguments.size() >= 255) {
                error_at_current("Can't have more than 255 arguments.");
            }
            arguments.push_back(expression());
        } while (match(TokenType::COMMA));
    }
    
    Token paren = consume(TokenType::RIGHT_PAREN, "Expected ')' after arguments.");
    
    return std::make_unique<CallExpr>(
        paren.span,
        std::move(callee),
        std::move(arguments)
    );
}

std::unique_ptr<Expr> Parser::primary() {
    if (match(TokenType::FALSE)) {
        return std::make_unique<LiteralExpr>(previous().span, make_bool(false));
    }
    
    if (match(TokenType::TRUE)) {
        return std::make_unique<LiteralExpr>(previous().span, make_bool(true));
    }
    
    if (match(TokenType::NIL)) {
        return std::make_unique<LiteralExpr>(previous().span, make_nil());
    }
    
    if (match(TokenType::NUMBER)) {
        Token token = previous();
        return std::make_unique<LiteralExpr>(token.span, std::get<double>(token.literal));
    }
    
    if (match(TokenType::STRING)) {
        Token token = previous();
        return std::make_unique<LiteralExpr>(token.span, std::get<std::string>(token.literal));
    }
    
    if (match(TokenType::IDENTIFIER)) {
        return std::make_unique<VariableExpr>(previous().span, previous().lexeme);
    }
    
    if (match(TokenType::LEFT_PAREN)) {
        std::unique_ptr<Expr> expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expected ')' after expression.");
        return expr;
    }
    
    throw error_at_current("Expected expression.");
}

bool Parser::is_at_end() const {
    return peek().type == TokenType::EOF_TOKEN;
}

Token Parser::advance() {
    if (!is_at_end()) current_++;
    return previous();
}

Token Parser::peek() const {
    return tokens_[current_];
}

Token Parser::previous() const {
    return tokens_[current_ - 1];
}

bool Parser::check(TokenType type) const {
    if (is_at_end()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match_any(const std::vector<TokenType>& types) {
    for (TokenType type : types) {
        if (match(type)) {
            return true;
        }
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    
    throw error_at_current(message);
}

void Parser::synchronize() {
    advance();
    
    while (!is_at_end()) {
        if (previous().type == TokenType::SEMICOLON) return;
        
        switch (peek().type) {
            case TokenType::FN:
            case TokenType::LET:
            case TokenType::VAR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::FOR:
            case TokenType::RETURN:
                return;
            default:
                break;
        }
        
        advance();
    }
}

void Parser::error(const Token& token, const std::string& message) {
    error_reporter_.report(ErrorType::SYNTAX, message, token.span);
    throw Error(ErrorType::SYNTAX, message, token.span);
}

void Parser::error_at_current(const std::string& message) {
    error(peek(), message);
}

} // namespace tl
