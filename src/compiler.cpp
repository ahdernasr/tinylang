#include "tl/compiler.hpp"
#include "tl/ast.hpp"
#include "tl/opcodes.hpp"
#include "tl/error.hpp"
#include <algorithm>
#include <unordered_map>

namespace tl {

Compiler::Compiler(FunctionType type, std::shared_ptr<Compiler> enclosing)
    : enclosing(enclosing), type(type), scope_depth(0) {
    function = std::make_shared<Function>();
    if (type == FunctionType::FUNCTION) {
        function->name = "";
    }
}

CompilerImpl::CompilerImpl(ErrorReporter& error_reporter)
    : error_reporter_(error_reporter) {
    current_ = std::make_shared<Compiler>(FunctionType::SCRIPT);
    current_function_ = current_->function;
    current_chunk_ = &current_function_->chunk;
}

std::shared_ptr<Function> CompilerImpl::compile(const std::unique_ptr<Program>& program) {
    program->accept(*this);
    return current_function_;
}

void CompilerImpl::begin_scope() {
    current_->scope_depth++;
}

void CompilerImpl::end_scope() {
    current_->scope_depth--;
    
    // Pop locals that went out of scope
    while (!current_->locals.empty() && 
           current_->locals.back().depth > current_->scope_depth) {
        current_->locals.pop_back();
    }
}

void CompilerImpl::declare_variable(const std::string& name) {
    if (current_->scope_depth == 0) {
        return; // Global variable
    }
    
    // Check for duplicate local variables
    for (auto it = current_->locals.rbegin(); it != current_->locals.rend(); it++) {
        if (it->depth < current_->scope_depth) {
            break;
        }
        if (it->name == name) {
            error_at_current("Already a variable with this name in this scope.");
        }
    }
    
    current_->locals.emplace_back(name, current_->scope_depth);
}

void CompilerImpl::define_variable(const std::string& name) {
    if (current_->scope_depth > 0) {
        // Local variable
        current_->locals.back().is_captured = false;
    } else {
        // Global variable
        emit_constant(make_string(name));
    }
}

int CompilerImpl::resolve_local(const std::string& name) {
    for (int i = static_cast<int>(current_->locals.size()) - 1; i >= 0; i--) {
        if (current_->locals[i].name == name) {
            return i;
        }
    }
    return -1;
}

int CompilerImpl::add_upvalue(uint8_t index, bool is_local) {
    for (int i = 0; i < static_cast<int>(current_->upvalues.size()); i++) {
        if (current_->upvalues[i].index == index && current_->upvalues[i].is_local == is_local) {
            return i;
        }
    }
    
    current_->upvalues.emplace_back(index, is_local);
    return static_cast<int>(current_->upvalues.size()) - 1;
}

int CompilerImpl::resolve_upvalue(const std::string& name) {
    if (current_->enclosing == nullptr) {
        return -1;
    }
    
    int local = current_->enclosing->locals.empty() ? -1 : 
                resolve_local(name);
    if (local != -1) {
        current_->enclosing->locals[local].is_captured = true;
        return add_upvalue(static_cast<uint8_t>(local), true);
    }
    
    int upvalue = resolve_upvalue(name);
    if (upvalue != -1) {
        return add_upvalue(static_cast<uint8_t>(upvalue), false);
    }
    
    return -1;
}

void CompilerImpl::emit_byte(uint8_t byte) {
    current_chunk_->write_byte(byte, 0); // Line number will be set properly
}

void CompilerImpl::emit_opcode(OpCode opcode) {
    current_chunk_->write_opcode(opcode, 0);
}

void CompilerImpl::emit_operand(uint16_t operand) {
    current_chunk_->write_operand(operand, 0);
}

void CompilerImpl::emit_constant(Value value) {
    current_chunk_->write_constant(value, 0);
}

int CompilerImpl::emit_jump(OpCode opcode) {
    emit_opcode(opcode);
    emit_operand(0xFFFF); // Placeholder
    return static_cast<int>(current_chunk_->code.size()) - 2;
}

void CompilerImpl::emit_loop(int loop_start) {
    emit_opcode(OpCode::OP_LOOP);
    
    int offset = current_chunk_->code.size() - loop_start + 2;
    emit_operand(static_cast<uint16_t>(offset));
}

void CompilerImpl::patch_jump(int offset) {
    int jump = current_chunk_->code.size() - offset - 2;
    current_chunk_->patch_operand(offset, static_cast<uint16_t>(jump));
}

void CompilerImpl::error_at(const Token& token, const std::string& message) {
    error_reporter_.report(ErrorType::SEMANTIC, message, token.span);
}

void CompilerImpl::error_at_current(const std::string& message) {
    error_reporter_.report(ErrorType::SEMANTIC, message, Span());
}

// Expression visitors
void CompilerImpl::visit_literal(LiteralExpr& expr) {
    emit_constant(expr.value);
}

void CompilerImpl::visit_variable(VariableExpr& expr) {
    int arg = resolve_local(expr.name);
    if (arg != -1) {
        emit_opcode(OpCode::OP_GET_LOCAL);
        emit_byte(static_cast<uint8_t>(arg));
    } else {
        int upvalue = resolve_upvalue(expr.name);
        if (upvalue != -1) {
            emit_opcode(OpCode::OP_GET_UPVALUE);
            emit_byte(static_cast<uint8_t>(upvalue));
        } else {
            emit_opcode(OpCode::OP_GET_GLOBAL);
            emit_constant(make_string(expr.name));
        }
    }
}

void CompilerImpl::visit_unary(UnaryExpr& expr) {
    expr.operand->accept(*this);
    
    switch (expr.op) {
        case TokenType::BANG:
            emit_opcode(OpCode::OP_NOT);
            break;
        case TokenType::MINUS:
            emit_opcode(OpCode::OP_NEGATE);
            break;
        default:
            error_at_current("Invalid unary operator");
    }
}

void CompilerImpl::visit_binary(BinaryExpr& expr) {
    expr.left->accept(*this);
    expr.right->accept(*this);
    
    switch (expr.op) {
        case TokenType::PLUS:
            emit_opcode(OpCode::OP_ADD);
            break;
        case TokenType::MINUS:
            emit_opcode(OpCode::OP_SUBTRACT);
            break;
        case TokenType::STAR:
            emit_opcode(OpCode::OP_MULTIPLY);
            break;
        case TokenType::SLASH:
            emit_opcode(OpCode::OP_DIVIDE);
            break;
        case TokenType::PERCENT:
            emit_opcode(OpCode::OP_MODULO);
            break;
        case TokenType::EQUAL_EQUAL:
            emit_opcode(OpCode::OP_EQUAL);
            break;
        case TokenType::BANG_EQUAL:
            emit_opcode(OpCode::OP_NOT_EQUAL);
            break;
        case TokenType::LESS:
            emit_opcode(OpCode::OP_LESS);
            break;
        case TokenType::LESS_EQUAL:
            emit_opcode(OpCode::OP_LESS_EQUAL);
            break;
        case TokenType::GREATER:
            emit_opcode(OpCode::OP_GREATER);
            break;
        case TokenType::GREATER_EQUAL:
            emit_opcode(OpCode::OP_GREATER_EQUAL);
            break;
        case TokenType::AND:
            // Short-circuit evaluation
            emit_opcode(OpCode::OP_JUMP_IF_FALSE);
            emit_operand(0xFFFF); // Placeholder
            {
                int jump_offset = static_cast<int>(current_chunk_->code.size()) - 2;
                emit_opcode(OpCode::OP_POP);
                expr.right->accept(*this);
                patch_jump(jump_offset);
            }
            break;
        case TokenType::OR:
            // Short-circuit evaluation
            emit_opcode(OpCode::OP_JUMP_IF_FALSE);
            emit_operand(0xFFFF); // Placeholder
            {
                int jump_offset = static_cast<int>(current_chunk_->code.size()) - 2;
                emit_opcode(OpCode::OP_POP);
                expr.right->accept(*this);
                patch_jump(jump_offset);
            }
            break;
        default:
            error_at_current("Invalid binary operator");
    }
}

void CompilerImpl::visit_call(CallExpr& expr) {
    expr.callee->accept(*this);
    
    for (auto& arg : expr.arguments) {
        arg->accept(*this);
    }
    
    emit_opcode(OpCode::OP_CALL);
    emit_byte(static_cast<uint8_t>(expr.arguments.size()));
}

void CompilerImpl::visit_assign(AssignExpr& expr) {
    expr.value->accept(*this);
    
    int arg = resolve_local(expr.name);
    if (arg != -1) {
        emit_opcode(OpCode::OP_SET_LOCAL);
        emit_byte(static_cast<uint8_t>(arg));
    } else {
        int upvalue = resolve_upvalue(expr.name);
        if (upvalue != -1) {
            emit_opcode(OpCode::OP_SET_UPVALUE);
            emit_byte(static_cast<uint8_t>(upvalue));
        } else {
            emit_opcode(OpCode::OP_SET_GLOBAL);
            emit_constant(make_string(expr.name));
        }
    }
}

// Statement visitors
void CompilerImpl::visit_expr_stmt(ExprStmt& stmt) {
    stmt.expression->accept(*this);
    emit_opcode(OpCode::OP_POP);
}

void CompilerImpl::visit_var_stmt(VarStmt& stmt) {
    declare_variable(stmt.name);
    
    if (stmt.initializer != nullptr) {
        stmt.initializer->accept(*this);
    } else {
        emit_opcode(OpCode::OP_NIL);
    }
    
    define_variable(stmt.name);
}

void CompilerImpl::visit_block_stmt(BlockStmt& stmt) {
    begin_scope();
    
    for (auto& statement : stmt.statements) {
        statement->accept(*this);
    }
    
    end_scope();
}

void CompilerImpl::visit_if_stmt(IfStmt& stmt) {
    stmt.condition->accept(*this);
    
    int then_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);
    emit_opcode(OpCode::OP_POP);
    
    stmt.then_branch->accept(*this);
    
    int else_jump = emit_jump(OpCode::OP_JUMP);
    
    patch_jump(then_jump);
    emit_opcode(OpCode::OP_POP);
    
    if (stmt.else_branch != nullptr) {
        stmt.else_branch->accept(*this);
    }
    
    patch_jump(else_jump);
}

void CompilerImpl::visit_while_stmt(WhileStmt& stmt) {
    int loop_start = current_chunk_->code.size();
    
    stmt.condition->accept(*this);
    int exit_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);
    emit_opcode(OpCode::OP_POP);
    
    stmt.body->accept(*this);
    emit_loop(loop_start);
    
    patch_jump(exit_jump);
    emit_opcode(OpCode::OP_POP);
}

void CompilerImpl::visit_for_stmt(ForStmt& stmt) {
    // For loops are desugared to while loops in the parser
    stmt.body->accept(*this);
}

void CompilerImpl::visit_break_stmt(BreakStmt& stmt) {
    // Break statements are handled by the loop constructs
    emit_opcode(OpCode::OP_POP);
}

void CompilerImpl::visit_continue_stmt(ContinueStmt& stmt) {
    // Continue statements are handled by the loop constructs
    emit_opcode(OpCode::OP_POP);
}

void CompilerImpl::visit_return_stmt(ReturnStmt& stmt) {
    if (stmt.value != nullptr) {
        stmt.value->accept(*this);
    } else {
        emit_opcode(OpCode::OP_NIL);
    }
    
    emit_opcode(OpCode::OP_RETURN);
}

void CompilerImpl::visit_function_stmt(FunctionStmt& stmt) {
    declare_variable(stmt.name);
    define_variable(stmt.name);
    
    // Compile function body
    std::shared_ptr<Compiler> compiler = std::make_shared<Compiler>(FunctionType::FUNCTION, current_);
    compiler->function->name = stmt.name;
    compiler->function->arity = static_cast<uint8_t>(stmt.parameters.size());
    
    current_ = compiler;
    current_function_ = compiler->function;
    current_chunk_ = &current_function_->chunk;
    
    begin_scope();
    
    for (const auto& param : stmt.parameters) {
        declare_variable(param);
        define_variable(param);
    }
    
    stmt.body->accept(*this);
    
    end_scope();
    
    // Create closure
    emit_opcode(OpCode::OP_CLOSURE);
    emit_constant(make_string(stmt.name));
    
    // Restore previous compiler
    current_ = current_->enclosing;
    current_function_ = current_->function;
    current_chunk_ = &current_function_->chunk;
}

void CompilerImpl::visit_program(Program& program) {
    for (auto& statement : program.statements) {
        statement->accept(*this);
    }
}

// Constant folding
bool CompilerImpl::is_constant_foldable(const Expr& expr) const {
    if (dynamic_cast<const LiteralExpr*>(&expr)) {
        return true;
    }
    
    if (const auto* binary = dynamic_cast<const BinaryExpr*>(&expr)) {
        return is_constant_foldable(*binary->left) && is_constant_foldable(*binary->right);
    }
    
    if (const auto* unary = dynamic_cast<const UnaryExpr*>(&expr)) {
        return is_constant_foldable(*unary->operand);
    }
    
    return false;
}

Value CompilerImpl::fold_constant(const Expr& expr) {
    if (const auto* literal = dynamic_cast<const LiteralExpr*>(&expr)) {
        return literal->value;
    }
    
    if (const auto* binary = dynamic_cast<const BinaryExpr*>(&expr)) {
        return fold_binary(*binary);
    }
    
    if (const auto* unary = dynamic_cast<const UnaryExpr*>(&expr)) {
        return fold_unary(*unary);
    }
    
    return make_nil();
}

Value CompilerImpl::fold_binary(const BinaryExpr& expr) {
    Value left = fold_constant(*expr.left);
    Value right = fold_constant(*expr.right);
    
    if (is_number(left) && is_number(right)) {
        double a = as_number(left);
        double b = as_number(right);
        
        switch (expr.op) {
            case TokenType::PLUS:
                return make_number(a + b);
            case TokenType::MINUS:
                return make_number(a - b);
            case TokenType::STAR:
                return make_number(a * b);
            case TokenType::SLASH:
                if (b == 0.0) return make_nil(); // Division by zero
                return make_number(a / b);
            case TokenType::PERCENT:
                if (b == 0.0) return make_nil(); // Modulo by zero
                return make_number(std::fmod(a, b));
            case TokenType::EQUAL_EQUAL:
                return make_bool(a == b);
            case TokenType::BANG_EQUAL:
                return make_bool(a != b);
            case TokenType::LESS:
                return make_bool(a < b);
            case TokenType::LESS_EQUAL:
                return make_bool(a <= b);
            case TokenType::GREATER:
                return make_bool(a > b);
            case TokenType::GREATER_EQUAL:
                return make_bool(a >= b);
            default:
                break;
        }
    }
    
    if (is_string(left) && is_string(right) && expr.op == TokenType::PLUS) {
        return make_string(as_string(left) + as_string(right));
    }
    
    return make_nil();
}

Value CompilerImpl::fold_unary(const UnaryExpr& expr) {
    Value operand = fold_constant(*expr.operand);
    
    switch (expr.op) {
        case TokenType::MINUS:
            if (is_number(operand)) {
                return make_number(-as_number(operand));
            }
            break;
        case TokenType::BANG:
            return make_bool(!as_bool(operand));
        default:
            break;
    }
    
    return make_nil();
}

} // namespace tl
