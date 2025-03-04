#include "tl/ast.hpp"
#include "tl/compiler.hpp"

namespace tl {

// Expression node implementations
void LiteralExpr::accept(AstVisitor& visitor) {
    visitor.visit_literal(*this);
}

void VariableExpr::accept(AstVisitor& visitor) {
    visitor.visit_variable(*this);
}

void UnaryExpr::accept(AstVisitor& visitor) {
    visitor.visit_unary(*this);
}

void BinaryExpr::accept(AstVisitor& visitor) {
    visitor.visit_binary(*this);
}

void CallExpr::accept(AstVisitor& visitor) {
    visitor.visit_call(*this);
}

void AssignExpr::accept(AstVisitor& visitor) {
    visitor.visit_assign(*this);
}

// Statement node implementations
void ExprStmt::accept(AstVisitor& visitor) {
    visitor.visit_expr_stmt(*this);
}

void VarStmt::accept(AstVisitor& visitor) {
    visitor.visit_var_stmt(*this);
}

void BlockStmt::accept(AstVisitor& visitor) {
    visitor.visit_block_stmt(*this);
}

void IfStmt::accept(AstVisitor& visitor) {
    visitor.visit_if_stmt(*this);
}

void WhileStmt::accept(AstVisitor& visitor) {
    visitor.visit_while_stmt(*this);
}

void ForStmt::accept(AstVisitor& visitor) {
    visitor.visit_for_stmt(*this);
}

void BreakStmt::accept(AstVisitor& visitor) {
    visitor.visit_break_stmt(*this);
}

void ContinueStmt::accept(AstVisitor& visitor) {
    visitor.visit_continue_stmt(*this);
}

void ReturnStmt::accept(AstVisitor& visitor) {
    visitor.visit_return_stmt(*this);
}

void FunctionStmt::accept(AstVisitor& visitor) {
    visitor.visit_function_stmt(*this);
}

// Program node implementation
void Program::accept(AstVisitor& visitor) {
    visitor.visit_program(*this);
}

} // namespace tl
