#pragma once

#include "token.hpp"
#include "value.hpp"
#include <string>
#include <vector>
#include <memory>

namespace tl {

// Forward declarations
class AstVisitor;

// Base AST node
class AstNode {
public:
    Span span;
    
    AstNode(const Span& span) : span(span) {}
    virtual ~AstNode() = default;
    virtual void accept(AstVisitor& visitor) = 0;
};

// Expression nodes
class Expr : public AstNode {
public:
    Expr(const Span& span) : AstNode(span) {}
};

class LiteralExpr : public Expr {
public:
    Value value;
    
    LiteralExpr(const Span& span, Value value) : Expr(span), value(value) {}
    void accept(AstVisitor& visitor) override;
};

class VariableExpr : public Expr {
public:
    std::string name;
    
    VariableExpr(const Span& span, const std::string& name) : Expr(span), name(name) {}
    void accept(AstVisitor& visitor) override;
};

class UnaryExpr : public Expr {
public:
    TokenType op;
    std::unique_ptr<Expr> operand;
    
    UnaryExpr(const Span& span, TokenType op, std::unique_ptr<Expr> operand)
        : Expr(span), op(op), operand(std::move(operand)) {}
    void accept(AstVisitor& visitor) override;
};

class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    TokenType op;
    std::unique_ptr<Expr> right;
    
    BinaryExpr(const Span& span, std::unique_ptr<Expr> left, TokenType op, std::unique_ptr<Expr> right)
        : Expr(span), left(std::move(left)), op(op), right(std::move(right)) {}
    void accept(AstVisitor& visitor) override;
};

class CallExpr : public Expr {
public:
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> arguments;
    
    CallExpr(const Span& span, std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> arguments)
        : Expr(span), callee(std::move(callee)), arguments(std::move(arguments)) {}
    void accept(AstVisitor& visitor) override;
};

class AssignExpr : public Expr {
public:
    std::string name;
    std::unique_ptr<Expr> value;
    
    AssignExpr(const Span& span, const std::string& name, std::unique_ptr<Expr> value)
        : Expr(span), name(name), value(std::move(value)) {}
    void accept(AstVisitor& visitor) override;
};

// Statement nodes
class Stmt : public AstNode {
public:
    Stmt(const Span& span) : AstNode(span) {}
};

class ExprStmt : public Stmt {
public:
    std::unique_ptr<Expr> expression;
    
    ExprStmt(const Span& span, std::unique_ptr<Expr> expression)
        : Stmt(span), expression(std::move(expression)) {}
    void accept(AstVisitor& visitor) override;
};

class VarStmt : public Stmt {
public:
    std::string name;
    bool is_mutable;
    std::unique_ptr<Expr> initializer;
    
    VarStmt(const Span& span, const std::string& name, bool is_mutable, std::unique_ptr<Expr> initializer)
        : Stmt(span), name(name), is_mutable(is_mutable), initializer(std::move(initializer)) {}
    void accept(AstVisitor& visitor) override;
};

class BlockStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> statements;
    
    BlockStmt(const Span& span, std::vector<std::unique_ptr<Stmt>> statements)
        : Stmt(span), statements(std::move(statements)) {}
    void accept(AstVisitor& visitor) override;
};

class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> then_branch;
    std::unique_ptr<Stmt> else_branch;
    
    IfStmt(const Span& span, std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> then_branch, std::unique_ptr<Stmt> else_branch)
        : Stmt(span), condition(std::move(condition)), then_branch(std::move(then_branch)), else_branch(std::move(else_branch)) {}
    void accept(AstVisitor& visitor) override;
};

class WhileStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    
    WhileStmt(const Span& span, std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
        : Stmt(span), condition(std::move(condition)), body(std::move(body)) {}
    void accept(AstVisitor& visitor) override;
};

class ForStmt : public Stmt {
public:
    std::unique_ptr<Stmt> initializer;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> increment;
    std::unique_ptr<Stmt> body;
    
    ForStmt(const Span& span, std::unique_ptr<Stmt> initializer, std::unique_ptr<Expr> condition, std::unique_ptr<Expr> increment, std::unique_ptr<Stmt> body)
        : Stmt(span), initializer(std::move(initializer)), condition(std::move(condition)), increment(std::move(increment)), body(std::move(body)) {}
    void accept(AstVisitor& visitor) override;
};

class BreakStmt : public Stmt {
public:
    BreakStmt(const Span& span) : Stmt(span) {}
    void accept(AstVisitor& visitor) override;
};

class ContinueStmt : public Stmt {
public:
    ContinueStmt(const Span& span) : Stmt(span) {}
    void accept(AstVisitor& visitor) override;
};

class ReturnStmt : public Stmt {
public:
    std::unique_ptr<Expr> value;
    
    ReturnStmt(const Span& span, std::unique_ptr<Expr> value)
        : Stmt(span), value(std::move(value)) {}
    void accept(AstVisitor& visitor) override;
};

class FunctionStmt : public Stmt {
public:
    std::string name;
    std::vector<std::string> parameters;
    std::unique_ptr<BlockStmt> body;
    
    FunctionStmt(const Span& span, const std::string& name, std::vector<std::string> parameters, std::unique_ptr<BlockStmt> body)
        : Stmt(span), name(name), parameters(std::move(parameters)), body(std::move(body)) {}
    void accept(AstVisitor& visitor) override;
};

// Program node
class Program : public AstNode {
public:
    std::vector<std::unique_ptr<Stmt>> statements;
    
    Program(const Span& span, std::vector<std::unique_ptr<Stmt>> statements)
        : AstNode(span), statements(std::move(statements)) {}
    void accept(AstVisitor& visitor) override;
};

// Visitor interface
class AstVisitor {
public:
    virtual ~AstVisitor() = default;
    
    // Expression visitors
    virtual void visit_literal(LiteralExpr& expr) = 0;
    virtual void visit_variable(VariableExpr& expr) = 0;
    virtual void visit_unary(UnaryExpr& expr) = 0;
    virtual void visit_binary(BinaryExpr& expr) = 0;
    virtual void visit_call(CallExpr& expr) = 0;
    virtual void visit_assign(AssignExpr& expr) = 0;
    
    // Statement visitors
    virtual void visit_expr_stmt(ExprStmt& stmt) = 0;
    virtual void visit_var_stmt(VarStmt& stmt) = 0;
    virtual void visit_block_stmt(BlockStmt& stmt) = 0;
    virtual void visit_if_stmt(IfStmt& stmt) = 0;
    virtual void visit_while_stmt(WhileStmt& stmt) = 0;
    virtual void visit_for_stmt(ForStmt& stmt) = 0;
    virtual void visit_break_stmt(BreakStmt& stmt) = 0;
    virtual void visit_continue_stmt(ContinueStmt& stmt) = 0;
    virtual void visit_return_stmt(ReturnStmt& stmt) = 0;
    virtual void visit_function_stmt(FunctionStmt& stmt) = 0;
    
    // Program visitor
    virtual void visit_program(Program& program) = 0;
};

} // namespace tl
