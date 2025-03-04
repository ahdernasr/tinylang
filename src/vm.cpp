#include "tl/vm.hpp"
#include "tl/lexer.hpp"
#include "tl/parser.hpp"
#include "tl/compiler.hpp"
#include "tl/optimizer.hpp"
#include "tl/disasm.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include <cmath>

namespace tl {

VM::VM() : instruction_count_(0), execution_start_time_(std::chrono::high_resolution_clock::now()) {
    init_builtins();
}

InterpretResult VM::interpret(const std::string& source) {
    ErrorReporter error_reporter;
    error_reporter.set_source(source);
    
    // Lexical analysis
    Lexer lexer(source, error_reporter);
    std::vector<Token> tokens = lexer.scan_all();
    
    if (error_reporter.has_errors()) {
        for (const auto& error : error_reporter.errors()) {
            std::cerr << error_reporter.format_error(error) << std::endl;
        }
        return InterpretResult::COMPILE_ERROR;
    }
    
    // Parsing
    Parser parser(tokens, error_reporter);
    std::unique_ptr<Program> program = parser.parse();
    
    if (error_reporter.has_errors()) {
        for (const auto& error : error_reporter.errors()) {
            std::cerr << error_reporter.format_error(error) << std::endl;
        }
        return InterpretResult::COMPILE_ERROR;
    }
    
    // Compilation
    CompilerImpl compiler(error_reporter);
    std::shared_ptr<Function> function = compiler.compile(program);
    
    if (error_reporter.has_errors()) {
        for (const auto& error : error_reporter.errors()) {
            std::cerr << error_reporter.format_error(error) << std::endl;
        }
        return InterpretResult::COMPILE_ERROR;
    }
    
    // Optimization
    Optimizer optimizer(function->chunk);
    optimizer.optimize();
    
    // Create closure and execute
    std::shared_ptr<Closure> closure = std::make_shared<Closure>(function);
    call_function(closure, 0);
    
    try {
        execution_start_time_ = std::chrono::high_resolution_clock::now();
        bool success = run();
        return success ? InterpretResult::OK : InterpretResult::RUNTIME_ERROR;
    } catch (const std::exception& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
        return InterpretResult::RUNTIME_ERROR;
    }
}

InterpretResult VM::interpret_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << filename << std::endl;
        return InterpretResult::COMPILE_ERROR;
    }
    
    std::string source((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    
    return interpret(source);
}

void VM::init_builtins() {
    define_builtin("print", make_string("print"));
    define_builtin("clock", make_string("clock"));
    define_builtin("len", make_string("len"));
    define_builtin("assert", make_string("assert"));
    define_builtin("toNumber", make_string("toNumber"));
    define_builtin("toString", make_string("toString"));
    define_builtin("range", make_string("range"));
}

void VM::define_builtin(const std::string& name, Value value) {
    global_table_.set(name, value);
}

void VM::push(const Value& value) {
    stack_.push_back(value);
}

Value VM::pop() {
    if (stack_.empty()) {
        throw std::runtime_error("Stack underflow");
    }
    Value value = stack_.back();
    stack_.pop_back();
    return value;
}

Value VM::peek(int distance) const {
    if (distance >= static_cast<int>(stack_.size())) {
        throw std::runtime_error("Stack underflow");
    }
    return stack_[stack_.size() - 1 - distance];
}

void VM::call_function(std::shared_ptr<Closure> closure, int arg_count) {
    if (arg_count != closure->function->arity) {
        runtime_error("Expected " + std::to_string(closure->function->arity) + 
                     " arguments but got " + std::to_string(arg_count) + ".");
        return;
    }
    
    if (frames_.size() >= 64) {
        runtime_error("Stack overflow.");
        return;
    }
    
    CallFrame frame(closure, stack_.data() + stack_.size() - arg_count - 1);
    frames_.push_back(frame);
}

void VM::call_value(const Value& callee, int arg_count) {
    if (std::holds_alternative<std::shared_ptr<Closure>>(callee)) {
        call_function(std::get<std::shared_ptr<Closure>>(callee), arg_count);
    } else if (std::holds_alternative<std::string>(callee)) {
        std::string name = std::get<std::string>(callee);
        
        if (name == "print") {
            builtin_print(std::vector<Value>(stack_.end() - arg_count, stack_.end()));
            stack_.resize(stack_.size() - arg_count);
            push(make_nil());
        } else if (name == "clock") {
            stack_.resize(stack_.size() - arg_count);
            push(builtin_clock({}));
        } else if (name == "len") {
            if (arg_count != 1) {
                runtime_error("Expected 1 argument for len()");
                return;
            }
            Value arg = pop();
            stack_.resize(stack_.size() - arg_count + 1);
            push(builtin_len({arg}));
        } else if (name == "assert") {
            if (arg_count != 1) {
                runtime_error("Expected 1 argument for assert()");
                return;
            }
            Value arg = pop();
            stack_.resize(stack_.size() - arg_count + 1);
            builtin_assert({arg});
            push(make_nil());
        } else if (name == "toNumber") {
            if (arg_count != 1) {
                runtime_error("Expected 1 argument for toNumber()");
                return;
            }
            Value arg = pop();
            stack_.resize(stack_.size() - arg_count + 1);
            push(builtin_to_number({arg}));
        } else if (name == "toString") {
            if (arg_count != 1) {
                runtime_error("Expected 1 argument for toString()");
                return;
            }
            Value arg = pop();
            stack_.resize(stack_.size() - arg_count + 1);
            push(builtin_to_string({arg}));
        } else if (name == "range") {
            if (arg_count != 1) {
                runtime_error("Expected 1 argument for range()");
                return;
            }
            Value arg = pop();
            stack_.resize(stack_.size() - arg_count + 1);
            push(builtin_range({arg}));
        } else {
            runtime_error("Undefined function: " + name);
        }
    } else {
        runtime_error("Can only call functions and classes.");
    }
}

Value VM::capture_upvalue(Value* local) {
    // Simplified upvalue capture - in a real implementation,
    // you'd manage upvalues more carefully
    return *local;
}

void VM::close_upvalues(Value* last) {
    // Simplified upvalue closing
}

bool VM::is_falsy(const Value& value) const {
    if (std::holds_alternative<std::monostate>(value)) return true;
    if (std::holds_alternative<bool>(value)) return !std::get<bool>(value);
    if (std::holds_alternative<double>(value)) return std::get<double>(value) == 0.0;
    if (std::holds_alternative<std::string>(value)) return std::get<std::string>(value).empty();
    return false;
}

bool VM::values_equal(const Value& a, const Value& b) const {
    return tl::values_equal(a, b);
}

bool VM::values_less(const Value& a, const Value& b) const {
    return tl::values_less(a, b);
}

void VM::runtime_error(const std::string& message) {
    std::cerr << "Runtime error: " << message << std::endl;
    print_stack_trace();
}

void VM::runtime_error(const std::string& message, const std::string& operand) {
    runtime_error(message + " " + operand);
}

bool VM::run() {
    CallFrame& frame = frames_.back();
    
    while (true) {
        if (frame.ip >= frame.closure->function->chunk.code.data() + frame.closure->function->chunk.code.size()) {
            break;
        }
        
        uint8_t instruction = *frame.ip++;
        instruction_count_++;
        
        if (!execute_instruction(static_cast<OpCode>(instruction))) {
            return false;
        }
    }
    
    return true;
}

bool VM::execute_instruction(OpCode instruction) {
    switch (instruction) {
        case OpCode::OP_CONSTANT: {
            uint8_t constant_index = *frames_.back().ip++;
            Value constant = frames_.back().closure->function->chunk.constants[constant_index];
            push(constant);
            break;
        }
        
        case OpCode::OP_NIL: {
            push(make_nil());
            break;
        }
        
        case OpCode::OP_TRUE: {
            push(make_bool(true));
            break;
        }
        
        case OpCode::OP_FALSE: {
            push(make_bool(false));
            break;
        }
        
        case OpCode::OP_ADD: {
            Value b = pop();
            Value a = pop();
            
            if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b)) {
                push(make_number(std::get<double>(a) + std::get<double>(b)));
            } else if (std::holds_alternative<std::string>(a) && std::holds_alternative<std::string>(b)) {
                push(make_string(std::get<std::string>(a) + std::get<std::string>(b)));
            } else {
                runtime_error("Operands must be two numbers or two strings.");
                return false;
            }
            break;
        }
        
        case OpCode::OP_SUBTRACT: {
            Value b = pop();
            Value a = pop();
            
            if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b)) {
                push(make_number(std::get<double>(a) - std::get<double>(b)));
            } else {
                runtime_error("Operands must be numbers.");
                return false;
            }
            break;
        }
        
        case OpCode::OP_MULTIPLY: {
            Value b = pop();
            Value a = pop();
            
            if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b)) {
                push(make_number(std::get<double>(a) * std::get<double>(b)));
            } else {
                runtime_error("Operands must be numbers.");
                return false;
            }
            break;
        }
        
        case OpCode::OP_DIVIDE: {
            Value b = pop();
            Value a = pop();
            
            if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b)) {
                double divisor = std::get<double>(b);
                if (divisor == 0.0) {
                    runtime_error("Division by zero.");
                    return false;
                }
                push(make_number(std::get<double>(a) / divisor));
            } else {
                runtime_error("Operands must be numbers.");
                return false;
            }
            break;
        }
        
        case OpCode::OP_MODULO: {
            Value b = pop();
            Value a = pop();
            
            if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b)) {
                double divisor = std::get<double>(b);
                if (divisor == 0.0) {
                    runtime_error("Modulo by zero.");
                    return false;
                }
                push(make_number(std::fmod(std::get<double>(a), divisor)));
            } else {
                runtime_error("Operands must be numbers.");
                return false;
            }
            break;
        }
        
        case OpCode::OP_NEGATE: {
            Value value = pop();
            if (std::holds_alternative<double>(value)) {
                push(make_number(-std::get<double>(value)));
            } else {
                runtime_error("Operand must be a number.");
                return false;
            }
            break;
        }
        
        case OpCode::OP_EQUAL: {
            Value b = pop();
            Value a = pop();
            push(make_bool(values_equal(a, b)));
            break;
        }
        
        case OpCode::OP_NOT_EQUAL: {
            Value b = pop();
            Value a = pop();
            push(make_bool(!values_equal(a, b)));
            break;
        }
        
        case OpCode::OP_LESS: {
            Value b = pop();
            Value a = pop();
            push(make_bool(values_less(a, b)));
            break;
        }
        
        case OpCode::OP_LESS_EQUAL: {
            Value b = pop();
            Value a = pop();
            push(make_bool(values_less(a, b) || values_equal(a, b)));
            break;
        }
        
        case OpCode::OP_GREATER: {
            Value b = pop();
            Value a = pop();
            push(make_bool(values_less(b, a)));
            break;
        }
        
        case OpCode::OP_GREATER_EQUAL: {
            Value b = pop();
            Value a = pop();
            push(make_bool(values_less(b, a) || values_equal(a, b)));
            break;
        }
        
        case OpCode::OP_NOT: {
            Value value = pop();
            push(make_bool(is_falsy(value)));
            break;
        }
        
        case OpCode::OP_GET_LOCAL: {
            uint8_t slot = *frames_.back().ip++;
            push(frames_.back().slots[slot]);
            break;
        }
        
        case OpCode::OP_SET_LOCAL: {
            uint8_t slot = *frames_.back().ip++;
            frames_.back().slots[slot] = peek(0);
            break;
        }
        
        case OpCode::OP_GET_GLOBAL: {
            uint8_t constant_index = *frames_.back().ip++;
            std::string name = std::get<std::string>(frames_.back().closure->function->chunk.constants[constant_index]);
            
            Value value;
            if (!global_table_.get(name, value)) {
                runtime_error("Undefined variable '" + name + "'.");
                return false;
            }
            push(value);
            break;
        }
        
        case OpCode::OP_SET_GLOBAL: {
            uint8_t constant_index = *frames_.back().ip++;
            std::string name = std::get<std::string>(frames_.back().closure->function->chunk.constants[constant_index]);
            
            global_table_.set(name, peek(0));
            break;
        }
        
        case OpCode::OP_JUMP: {
            uint16_t offset = frames_.back().closure->function->chunk.get_operand(frames_.back().ip - frames_.back().closure->function->chunk.code.data());
            frames_.back().ip += offset;
            break;
        }
        
        case OpCode::OP_JUMP_IF_FALSE: {
            uint16_t offset = frames_.back().closure->function->chunk.get_operand(frames_.back().ip - frames_.back().closure->function->chunk.code.data());
            frames_.back().ip += 2; // Skip operand
            
            if (is_falsy(peek(0))) {
                frames_.back().ip += offset;
            }
            break;
        }
        
        case OpCode::OP_LOOP: {
            uint16_t offset = frames_.back().closure->function->chunk.get_operand(frames_.back().ip - frames_.back().closure->function->chunk.code.data());
            frames_.back().ip -= offset;
            break;
        }
        
        case OpCode::OP_CALL: {
            uint8_t arg_count = *frames_.back().ip++;
            call_value(peek(arg_count), arg_count);
            break;
        }
        
        case OpCode::OP_RETURN: {
            Value result = pop();
            frames_.pop_back();
            
            if (frames_.empty()) {
                return true;
            }
            
            stack_.resize(stack_.size() - 1);
            push(result);
            break;
        }
        
        case OpCode::OP_POP: {
            pop();
            break;
        }
        
        default:
            runtime_error("Unknown opcode: " + std::to_string(static_cast<int>(instruction)));
            return false;
    }
    
    return true;
}

Value VM::builtin_print(const std::vector<Value>& args) {
    for (size_t i = 0; i < args.size(); i++) {
        if (i > 0) std::cout << " ";
        std::cout << value_to_string(args[i]);
    }
    std::cout << std::endl;
    return make_nil();
}

Value VM::builtin_clock(const std::vector<Value>& args) {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() / 1000.0;
    return make_number(seconds);
}

Value VM::builtin_len(const std::vector<Value>& args) {
    if (std::holds_alternative<std::string>(args[0])) {
        return make_number(static_cast<double>(std::get<std::string>(args[0]).length()));
    }
    runtime_error("len() expects a string");
    return make_nil();
}

Value VM::builtin_assert(const std::vector<Value>& args) {
    if (is_falsy(args[0])) {
        runtime_error("Assertion failed");
    }
    return make_nil();
}

Value VM::builtin_to_number(const std::vector<Value>& args) {
    return make_number(as_number(args[0]));
}

Value VM::builtin_to_string(const std::vector<Value>& args) {
    return make_string(as_string(args[0]));
}

Value VM::builtin_range(const std::vector<Value>& args) {
    double end = as_number(args[0]);
    // Simplified range - returns a string representation
    std::string result = "[";
    for (int i = 0; i < static_cast<int>(end); i++) {
        if (i > 0) result += ", ";
        result += std::to_string(i);
    }
    result += "]";
    return make_string(result);
}

void VM::define_global(const std::string& name, const Value& value) {
    global_table_.set(name, value);
}

bool VM::get_global(const std::string& name, Value& value) const {
    return global_table_.get(name, value);
}

void VM::set_global(const std::string& name, const Value& value) {
    global_table_.set(name, value);
}

size_t VM::instruction_count() const {
    return instruction_count_;
}

double VM::execution_time() const {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - execution_start_time_);
    return duration.count() / 1000.0; // Return milliseconds
}

void VM::print_stack_trace() const {
    std::cerr << "Stack trace:" << std::endl;
    for (int i = static_cast<int>(frames_.size()) - 1; i >= 0; i--) {
        const CallFrame& frame = frames_[i];
        std::string name = frame.closure->function->name.empty() ? "<script>" : frame.closure->function->name;
        std::cerr << "  at " << name << " (line " << frame.closure->function->chunk.get_line(frame.ip - frame.closure->function->chunk.code.data()) << ")" << std::endl;
    }
}

void VM::print_globals() const {
    std::cout << "Global variables:" << std::endl;
    for (const auto& key : global_table_.keys()) {
        Value value;
        if (global_table_.get(key, value)) {
            std::cout << "  " << key << " = " << value_to_string(value) << std::endl;
        }
    }
}

} // namespace tl
