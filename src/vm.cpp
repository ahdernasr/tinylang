#include "tl/vm.hpp"

#include <iostream>

namespace tl {

VM::VM() = default;

InterpretResult VM::interpret(const std::string& source) {
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(std::move(tokens));
        auto statements = parser.parse();

        execute(statements);
        return InterpretResult::OK;
    } catch (const ParseError& error) {
        std::cerr << "[compile error] " << error.what() << std::endl;
        return InterpretResult::COMPILE_ERROR;
    } catch (const RuntimeError& error) {
        std::cerr << "[runtime error] " << error.what() << std::endl;
        return InterpretResult::RUNTIME_ERROR;
    } catch (const std::exception& error) {
        std::cerr << "[error] " << error.what() << std::endl;
        return InterpretResult::RUNTIME_ERROR;
    }
}

Value VM::visit_literal_expr(LiteralExpr& expr) {
    return expr.value;
}

Value VM::visit_variable_expr(VariableExpr& expr) {
    for (auto scope_it = scopes_.rbegin(); scope_it != scopes_.rend(); ++scope_it) {
        auto it = scope_it->find(expr.name);
        if (it != scope_it->end()) {
            return it->second;
        }
    }

    auto global_it = globals_.find(expr.name);
    if (global_it != globals_.end()) {
        return global_it->second;
    }

    throw RuntimeError("Undefined variable '" + expr.name + "'.");
}

Value VM::visit_unary_expr(UnaryExpr& expr) {
    Value right = evaluate(*expr.right);

    switch (expr.op.type) {
        case TokenType::BANG:
            return Value{!is_truthy(right)};
        case TokenType::MINUS:
            if (!std::holds_alternative<double>(right)) {
                throw RuntimeError("Operand must be a number.");
            }
            return Value{-std::get<double>(right)};
        default:
            break;
    }

    throw RuntimeError("Unknown unary operator.");
}

Value VM::visit_binary_expr(BinaryExpr& expr) {
    Value left = evaluate(*expr.left);
    Value right = evaluate(*expr.right);

    switch (expr.op.type) {
        case TokenType::PLUS:
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
                return Value{std::get<double>(left) + std::get<double>(right)};
            }
            if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
                return Value{std::get<std::string>(left) + std::get<std::string>(right)};
            }
            throw RuntimeError("Operands must be two numbers or two strings.");
        case TokenType::MINUS:
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
                return Value{std::get<double>(left) - std::get<double>(right)};
            }
            throw RuntimeError("Operands must be numbers.");
        case TokenType::STAR:
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
                return Value{std::get<double>(left) * std::get<double>(right)};
            }
            throw RuntimeError("Operands must be numbers.");
        case TokenType::SLASH:
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
                double divisor = std::get<double>(right);
                if (divisor == 0.0) {
                    throw RuntimeError("Division by zero.");
                }
                return Value{std::get<double>(left) / divisor};
            }
            throw RuntimeError("Operands must be numbers.");
        case TokenType::GREATER:
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
                return Value{std::get<double>(left) > std::get<double>(right)};
            }
            throw RuntimeError("Operands must be numbers.");
        case TokenType::GREATER_EQUAL:
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
                return Value{std::get<double>(left) >= std::get<double>(right)};
            }
            throw RuntimeError("Operands must be numbers.");
        case TokenType::LESS:
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
                return Value{std::get<double>(left) < std::get<double>(right)};
            }
            throw RuntimeError("Operands must be numbers.");
        case TokenType::LESS_EQUAL:
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
                return Value{std::get<double>(left) <= std::get<double>(right)};
            }
            throw RuntimeError("Operands must be numbers.");
        case TokenType::BANG_EQUAL:
            return Value{!values_equal(left, right)};
        case TokenType::EQUAL_EQUAL:
            return Value{values_equal(left, right)};
        case TokenType::AND:
            return Value{is_truthy(left) && is_truthy(right)};
        case TokenType::OR:
            return Value{is_truthy(left) || is_truthy(right)};
        default:
            break;
    }

    throw RuntimeError("Unknown operator.");
}

Value VM::visit_assign_expr(AssignExpr& expr) {
    Value value = evaluate(*expr.value);

    for (auto scope_it = scopes_.rbegin(); scope_it != scopes_.rend(); ++scope_it) {
        auto it = scope_it->find(expr.name);
        if (it != scope_it->end()) {
            it->second = value;
            return value;
        }
    }

    auto global_it = globals_.find(expr.name);
    if (global_it != globals_.end()) {
        global_it->second = value;
        return value;
    }

    throw RuntimeError("Undefined variable '" + expr.name + "'.");
}

void VM::visit_expression_stmt(ExpressionStmt& stmt) {
    evaluate(*stmt.expression);
}

void VM::visit_print_stmt(PrintStmt& stmt) {
    Value value = evaluate(*stmt.expression);
    std::cout << to_string(value) << std::endl;
}

void VM::visit_let_stmt(LetStmt& stmt) {
    Value value = stmt.initializer ? evaluate(*stmt.initializer) : Value{};
    define(stmt.name, value);
}

void VM::visit_block_stmt(BlockStmt& stmt) {
    scopes_.emplace_back();
    execute_block(stmt.statements);
    scopes_.pop_back();
}

void VM::visit_if_stmt(IfStmt& stmt) {
    if (is_truthy(evaluate(*stmt.condition))) {
        if (stmt.then_branch) {
            stmt.then_branch->accept(*this);
        }
    } else if (stmt.else_branch) {
        stmt.else_branch->accept(*this);
    }
}

void VM::visit_while_stmt(WhileStmt& stmt) {
    while (is_truthy(evaluate(*stmt.condition))) {
        stmt.body->accept(*this);
    }
}

void VM::execute(const std::vector<StmtPtr>& statements) {
    for (const auto& stmt : statements) {
        if (!stmt) continue;
        stmt->accept(*this);
    }
}

void VM::execute_block(const std::vector<StmtPtr>& statements) {
    for (const auto& stmt : statements) {
        if (!stmt) continue;
        stmt->accept(*this);
    }
}

void VM::define(const std::string& name, const Value& value) {
    current_scope()[name] = value;
}

void VM::assign(const std::string& name, const Value& value) {
    for (auto scope_it = scopes_.rbegin(); scope_it != scopes_.rend(); ++scope_it) {
        auto it = scope_it->find(name);
        if (it != scope_it->end()) {
            it->second = value;
            return;
        }
    }

    auto global_it = globals_.find(name);
    if (global_it != globals_.end()) {
        global_it->second = value;
        return;
    }

    throw RuntimeError("Undefined variable '" + name + "'.");
}

Value VM::evaluate(Expr& expr) {
    return expr.accept(*this);
}

std::unordered_map<std::string, Value>& VM::current_scope() {
    if (scopes_.empty()) {
        return globals_;
    }
    return scopes_.back();
}

} // namespace tl

