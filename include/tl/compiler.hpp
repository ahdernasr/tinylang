#pragma once

#include "ast.hpp"
#include "bytecode.hpp"
#include "error.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace tl {

enum class FunctionType {
    SCRIPT,
    FUNCTION,
    METHOD
};

struct Local {
    std::string name;
    int depth;
    bool is_captured;
    
    Local(const std::string& name, int depth) : name(name), depth(depth), is_captured(false) {}
};

struct Compiler {
    std::shared_ptr<Compiler> enclosing;
    std::shared_ptr<Function> function;
    FunctionType type;
    std::vector<Local> locals;
    int scope_depth;
    std::vector<Upvalue> upvalues;
    
    Compiler(FunctionType type, std::shared_ptr<Compiler> enclosing = nullptr);
};

class CompilerImpl : public AstVisitor {
private:
    ErrorReporter& error_reporter_;
    std::shared_ptr<Compiler> current_;
    std::shared_ptr<Function> current_function_;
    Chunk* current_chunk_;
    
    // Constant folding
    bool is_constant_foldable(const Expr& expr) const;
    Value fold_constant(const Expr& expr);
    Value fold_binary(const BinaryExpr& expr);
    Value fold_unary(const UnaryExpr& expr);
    
    // Compilation helpers
    void begin_scope();
    void end_scope();
    void declare_variable(const std::string& name);
    void define_variable(const std::string& name);
    int resolve_local(const std::string& name);
    int add_upvalue(uint8_t index, bool is_local);
    int resolve_upvalue(const std::string& name);
    
    void emit_byte(uint8_t byte);
    void emit_opcode(OpCode opcode);
    void emit_operand(uint16_t operand);
    void emit_constant(Value value);
    void emit_jump(OpCode opcode);
    void emit_loop(int loop_start);
    void patch_jump(int offset);
    
    void parse_precedence(Precedence precedence);
    void binary(bool can_assign);
    void unary(bool can_assign);
    void literal(bool can_assign);
    void variable(bool can_assign);
    void call(bool can_assign);
    
    // Error handling
    void error_at(const Token& token, const std::string& message);
    void error_at_current(const std::string& message);

public:
    CompilerImpl(ErrorReporter& error_reporter);
    
    std::shared_ptr<Function> compile(const std::unique_ptr<Program>& program);
    
    // Expression visitors
    void visit_literal(LiteralExpr& expr) override;
    void visit_variable(VariableExpr& expr) override;
    void visit_unary(UnaryExpr& expr) override;
    void visit_binary(BinaryExpr& expr) override;
    void visit_call(CallExpr& expr) override;
    void visit_assign(AssignExpr& expr) override;
    
    // Statement visitors
    void visit_expr_stmt(ExprStmt& stmt) override;
    void visit_var_stmt(VarStmt& stmt) override;
    void visit_block_stmt(BlockStmt& stmt) override;
    void visit_if_stmt(IfStmt& stmt) override;
    void visit_while_stmt(WhileStmt& stmt) override;
    void visit_for_stmt(ForStmt& stmt) override;
    void visit_break_stmt(BreakStmt& stmt) override;
    void visit_continue_stmt(ContinueStmt& stmt) override;
    void visit_return_stmt(ReturnStmt& stmt) override;
    void visit_function_stmt(FunctionStmt& stmt) override;
    
    // Program visitor
    void visit_program(Program& program) override;
};

enum class Precedence {
    NONE,
    ASSIGNMENT,  // =
    OR,          // ||
    AND,         // &&
    EQUALITY,    // == !=
    COMPARISON,  // < > <= >=
    TERM,        // + -
    FACTOR,      // * / %
    UNARY,       // ! -
    CALL,        // . ()
    PRIMARY
};

struct ParseRule {
    void (CompilerImpl::*prefix)(bool);
    void (CompilerImpl::*infix)(bool);
    Precedence precedence;
};

} // namespace tl
