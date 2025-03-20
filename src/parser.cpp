#include "tl/parser.hpp"

#include <stdexcept>

namespace tl {

Parser::Parser(std::vector<Token> tokens)
    : tokens_(std::move(tokens)), current_(0) {}

std::vector<StmtPtr> Parser::parse() {
    std::vector<StmtPtr> statements;
    while (!is_at_end()) {
        statements.push_back(declaration());
    }
    return statements;
}

StmtPtr Parser::declaration() {
    try {
        if (match({TokenType::LET})) {
            return let_declaration();
        }
        return statement();
    } catch (const ParseError&) {
        synchronize();
        return nullptr;
    }
}

StmtPtr Parser::let_declaration() {
    const Token& name = consume(TokenType::IDENTIFIER, "Expected variable name after 'let'.");
    consume(TokenType::EQUAL, "Expected '=' after variable name.");
    ExprPtr initializer = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration.");
    return std::make_unique<LetStmt>(name.lexeme, std::move(initializer));
}

StmtPtr Parser::statement() {
    if (match({TokenType::PRINT})) {
        return print_statement();
    }
    if (match({TokenType::LEFT_BRACE})) {
        return block_statement();
    }
    if (match({TokenType::IF})) {
        return if_statement();
    }
    if (match({TokenType::WHILE})) {
        return while_statement();
    }
    return expression_statement();
}

StmtPtr Parser::print_statement() {
    ExprPtr value = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after value.");
    return std::make_unique<PrintStmt>(std::move(value));
}

StmtPtr Parser::expression_statement() {
    ExprPtr expr = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression.");
    return std::make_unique<ExpressionStmt>(std::move(expr));
}

StmtPtr Parser::block_statement() {
    std::vector<StmtPtr> statements;
    while (!check(TokenType::RIGHT_BRACE) && !is_at_end()) {
        statements.push_back(declaration());
    }
    consume(TokenType::RIGHT_BRACE, "Expected '}' after block.");
    return std::make_unique<BlockStmt>(std::move(statements));
}

StmtPtr Parser::if_statement() {
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'if'.");
    ExprPtr condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after condition.");

    StmtPtr then_branch = statement();
    StmtPtr else_branch = nullptr;
    if (match({TokenType::ELSE})) {
        else_branch = statement();
    }
    return std::make_unique<IfStmt>(std::move(condition), std::move(then_branch), std::move(else_branch));
}

StmtPtr Parser::while_statement() {
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'while'.");
    ExprPtr condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after condition.");
    StmtPtr body = statement();
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

ExprPtr Parser::expression() {
    return assignment();
}

ExprPtr Parser::assignment() {
    ExprPtr expr = or_expression();

    if (match({TokenType::EQUAL})) {
        const Token& equals = previous();
        ExprPtr value = assignment();

        if (auto* var_expr = dynamic_cast<VariableExpr*>(expr.get())) {
            std::string name = var_expr->name;
            return std::make_unique<AssignExpr>(name, std::move(value));
        }

        throw ParseError("Invalid assignment target at line " + std::to_string(equals.line));
    }

    return expr;
}

ExprPtr Parser::or_expression() {
    ExprPtr expr = and_expression();

    while (match({TokenType::OR})) {
        Token op = previous();
        ExprPtr right = and_expression();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

ExprPtr Parser::and_expression() {
    ExprPtr expr = equality();

    while (match({TokenType::AND})) {
        Token op = previous();
        ExprPtr right = equality();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

ExprPtr Parser::equality() {
    ExprPtr expr = comparison();

    while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL})) {
        Token op = previous();
        ExprPtr right = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

ExprPtr Parser::comparison() {
    ExprPtr expr = term();

    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL})) {
        Token op = previous();
        ExprPtr right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

ExprPtr Parser::term() {
    ExprPtr expr = factor();

    while (match({TokenType::PLUS, TokenType::MINUS})) {
        Token op = previous();
        ExprPtr right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

ExprPtr Parser::factor() {
    ExprPtr expr = unary();

    while (match({TokenType::STAR, TokenType::SLASH})) {
        Token op = previous();
        ExprPtr right = unary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

ExprPtr Parser::unary() {
    if (match({TokenType::BANG, TokenType::MINUS})) {
        Token op = previous();
        ExprPtr right = unary();
        return std::make_unique<UnaryExpr>(op, std::move(right));
    }

    return primary();
}

ExprPtr Parser::primary() {
    if (match({TokenType::FALSE})) return std::make_unique<LiteralExpr>(Value{false});
    if (match({TokenType::TRUE})) return std::make_unique<LiteralExpr>(Value{true});
    if (match({TokenType::NIL})) return std::make_unique<LiteralExpr>(Value{});

    if (match({TokenType::NUMBER})) {
        double value = std::get<double>(previous().literal);
        return std::make_unique<LiteralExpr>(Value{value});
    }

    if (match({TokenType::STRING})) {
        std::string value = std::get<std::string>(previous().literal);
        return std::make_unique<LiteralExpr>(Value{value});
    }

    if (match({TokenType::IDENTIFIER})) {
        return std::make_unique<VariableExpr>(previous().lexeme);
    }

    if (match({TokenType::LEFT_PAREN})) {
        ExprPtr expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expected ')' after expression.");
        return expr;
    }

    throw ParseError("Expected expression at line " + std::to_string(peek().line));
}

const Token& Parser::peek() const {
    return tokens_[current_];
}

const Token& Parser::previous() const {
    return tokens_[current_ - 1];
}

bool Parser::is_at_end() const {
    return peek().type == TokenType::END_OF_FILE;
}

const Token& Parser::advance() {
    if (!is_at_end()) current_++;
    return previous();
}

bool Parser::check(TokenType type) const {
    if (is_at_end()) return false;
    return peek().type == type;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

const Token& Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw ParseError(message + " (line " + std::to_string(peek().line) + ")");
}

void Parser::synchronize() {
    advance();

    while (!is_at_end()) {
        if (previous().type == TokenType::SEMICOLON) return;

        switch (peek().type) {
            case TokenType::LET:
            case TokenType::PRINT:
                return;
            default:
                break;
        }

        advance();
    }
}

} // namespace tl

