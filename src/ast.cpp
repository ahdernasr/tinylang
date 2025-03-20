#include "tl/ast.hpp"
#include "tl/vm.hpp"

namespace tl {

LiteralExpr::LiteralExpr(Value value) : value(std::move(value)) {}
Value LiteralExpr::accept(ExprVisitor& visitor) { return visitor.visit_literal_expr(*this); }

VariableExpr::VariableExpr(std::string name) : name(std::move(name)) {}
Value VariableExpr::accept(ExprVisitor& visitor) { return visitor.visit_variable_expr(*this); }

UnaryExpr::UnaryExpr(Token op, std::unique_ptr<Expr> right)
    : op(std::move(op)), right(std::move(right)) {}
Value UnaryExpr::accept(ExprVisitor& visitor) { return visitor.visit_unary_expr(*this); }

BinaryExpr::BinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
    : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
Value BinaryExpr::accept(ExprVisitor& visitor) { return visitor.visit_binary_expr(*this); }

AssignExpr::AssignExpr(std::string name, std::unique_ptr<Expr> value)
    : name(std::move(name)), value(std::move(value)) {}
Value AssignExpr::accept(ExprVisitor& visitor) { return visitor.visit_assign_expr(*this); }

ExpressionStmt::ExpressionStmt(std::unique_ptr<Expr> expression)
    : expression(std::move(expression)) {}
void ExpressionStmt::accept(StmtVisitor& visitor) { visitor.visit_expression_stmt(*this); }

PrintStmt::PrintStmt(std::unique_ptr<Expr> expression)
    : expression(std::move(expression)) {}
void PrintStmt::accept(StmtVisitor& visitor) { visitor.visit_print_stmt(*this); }

LetStmt::LetStmt(std::string name, std::unique_ptr<Expr> initializer)
    : name(std::move(name)), initializer(std::move(initializer)) {}
void LetStmt::accept(StmtVisitor& visitor) { visitor.visit_let_stmt(*this); }

BlockStmt::BlockStmt(std::vector<std::unique_ptr<Stmt>> statements)
    : statements(std::move(statements)) {}
void BlockStmt::accept(StmtVisitor& visitor) { visitor.visit_block_stmt(*this); }

IfStmt::IfStmt(std::unique_ptr<Expr> condition,
               std::unique_ptr<Stmt> then_branch,
               std::unique_ptr<Stmt> else_branch)
    : condition(std::move(condition)),
      then_branch(std::move(then_branch)),
      else_branch(std::move(else_branch)) {}
void IfStmt::accept(StmtVisitor& visitor) { visitor.visit_if_stmt(*this); }

WhileStmt::WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
    : condition(std::move(condition)), body(std::move(body)) {}
void WhileStmt::accept(StmtVisitor& visitor) { visitor.visit_while_stmt(*this); }

} // namespace tl

