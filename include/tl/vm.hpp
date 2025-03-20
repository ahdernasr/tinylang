#pragma once

#include "ast.hpp"
#include "parser.hpp"
#include "value.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace tl {

enum class InterpretResult {
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

class RuntimeError : public std::runtime_error {
public:
    explicit RuntimeError(const std::string& message)
        : std::runtime_error(message) {}
};

class VM : public ExprVisitor, public StmtVisitor {
public:
    VM();

    InterpretResult interpret(const std::string& source);

    // ExprVisitor implementation
    Value visit_literal_expr(LiteralExpr& expr) override;
    Value visit_variable_expr(VariableExpr& expr) override;
    Value visit_unary_expr(UnaryExpr& expr) override;
    Value visit_binary_expr(BinaryExpr& expr) override;
    Value visit_assign_expr(AssignExpr& expr) override;

    // StmtVisitor implementation
    void visit_expression_stmt(ExpressionStmt& stmt) override;
    void visit_print_stmt(PrintStmt& stmt) override;
    void visit_let_stmt(LetStmt& stmt) override;
    void visit_block_stmt(BlockStmt& stmt) override;
    void visit_if_stmt(IfStmt& stmt) override;
    void visit_while_stmt(WhileStmt& stmt) override;

private:
    std::unordered_map<std::string, Value> globals_;
    std::vector<std::unordered_map<std::string, Value>> scopes_;

    void execute(const std::vector<StmtPtr>& statements);
    void execute_block(const std::vector<StmtPtr>& statements);

    void define(const std::string& name, const Value& value);
    void assign(const std::string& name, const Value& value);
    Value evaluate(Expr& expr);

    std::unordered_map<std::string, Value>& current_scope();
};

} // namespace tl

