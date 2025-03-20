#pragma once

#include "token.hpp"
#include "value.hpp"

#include <memory>
#include <string>
#include <vector>

namespace tl {

class ExprVisitor;
class StmtVisitor;

class Expr {
public:
    virtual ~Expr() = default;
    virtual Value accept(ExprVisitor& visitor) = 0;
};

class LiteralExpr : public Expr {
public:
    explicit LiteralExpr(Value value);

    Value accept(ExprVisitor& visitor) override;

    Value value;
};

class VariableExpr : public Expr {
public:
    explicit VariableExpr(std::string name);

    Value accept(ExprVisitor& visitor) override;

    std::string name;
};

class UnaryExpr : public Expr {
public:
    UnaryExpr(Token op, std::unique_ptr<Expr> right);

    Value accept(ExprVisitor& visitor) override;

    Token op;
    std::unique_ptr<Expr> right;
};

class BinaryExpr : public Expr {
public:
    BinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right);

    Value accept(ExprVisitor& visitor) override;

    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
};

class AssignExpr : public Expr {
public:
    AssignExpr(std::string name, std::unique_ptr<Expr> value);

    Value accept(ExprVisitor& visitor) override;

    std::string name;
    std::unique_ptr<Expr> value;
};

class Stmt {
public:
    virtual ~Stmt() = default;
    virtual void accept(StmtVisitor& visitor) = 0;
};

class ExpressionStmt : public Stmt {
public:
    explicit ExpressionStmt(std::unique_ptr<Expr> expression);

    void accept(StmtVisitor& visitor) override;

    std::unique_ptr<Expr> expression;
};

class PrintStmt : public Stmt {
public:
    explicit PrintStmt(std::unique_ptr<Expr> expression);

    void accept(StmtVisitor& visitor) override;

    std::unique_ptr<Expr> expression;
};

class LetStmt : public Stmt {
public:
    LetStmt(std::string name, std::unique_ptr<Expr> initializer);

    void accept(StmtVisitor& visitor) override;

    std::string name;
    std::unique_ptr<Expr> initializer;
};

class BlockStmt : public Stmt {
public:
    explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> statements);

    void accept(StmtVisitor& visitor) override;

    std::vector<std::unique_ptr<Stmt>> statements;
};

class IfStmt : public Stmt {
public:
    IfStmt(std::unique_ptr<Expr> condition,
           std::unique_ptr<Stmt> then_branch,
           std::unique_ptr<Stmt> else_branch);

    void accept(StmtVisitor& visitor) override;

    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> then_branch;
    std::unique_ptr<Stmt> else_branch;
};

class WhileStmt : public Stmt {
public:
    WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body);

    void accept(StmtVisitor& visitor) override;

    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
};

class ExprVisitor {
public:
    virtual ~ExprVisitor() = default;
    virtual Value visit_literal_expr(LiteralExpr& expr) = 0;
    virtual Value visit_variable_expr(VariableExpr& expr) = 0;
    virtual Value visit_unary_expr(UnaryExpr& expr) = 0;
    virtual Value visit_binary_expr(BinaryExpr& expr) = 0;
    virtual Value visit_assign_expr(AssignExpr& expr) = 0;
};

class StmtVisitor {
public:
    virtual ~StmtVisitor() = default;
    virtual void visit_expression_stmt(ExpressionStmt& stmt) = 0;
    virtual void visit_print_stmt(PrintStmt& stmt) = 0;
    virtual void visit_let_stmt(LetStmt& stmt) = 0;
    virtual void visit_block_stmt(BlockStmt& stmt) = 0;
    virtual void visit_if_stmt(IfStmt& stmt) = 0;
    virtual void visit_while_stmt(WhileStmt& stmt) = 0;
};

using StmtPtr = std::unique_ptr<Stmt>;
using ExprPtr = std::unique_ptr<Expr>;

} // namespace tl

