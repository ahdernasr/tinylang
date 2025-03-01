#pragma once

#include "value.hpp"
#include "bytecode.hpp"
#include "table.hpp"
#include "gc.hpp"
#include "string.hpp"
#include <vector>
#include <memory>
#include <chrono>

namespace tl {

class VM {
private:
    std::vector<Value> stack_;
    std::vector<CallFrame> frames_;
    std::vector<Value> globals_;
    Table global_table_;
    StringInterner string_interner_;
    GarbageCollector gc_;
    
    // Built-in functions
    void init_builtins();
    void define_builtin(const std::string& name, Value value);
    
    // Stack operations
    void push(const Value& value);
    Value pop();
    Value peek(int distance = 0) const;
    
    // Call frame operations
    void call_function(std::shared_ptr<Closure> closure, int arg_count);
    void call_value(const Value& callee, int arg_count);
    
    // Upvalue operations
    Value capture_upvalue(Value* local);
    void close_upvalues(Value* last);
    
    // Runtime operations
    bool is_falsy(const Value& value) const;
    bool values_equal(const Value& a, const Value& b) const;
    bool values_less(const Value& a, const Value& b) const;
    
    // Error handling
    void runtime_error(const std::string& message);
    void runtime_error(const std::string& message, const std::string& operand);
    
    // Instruction execution
    bool run();
    bool execute_instruction(OpCode instruction);
    
    // Built-in function implementations
    Value builtin_print(const std::vector<Value>& args);
    Value builtin_clock(const std::vector<Value>& args);
    Value builtin_len(const std::vector<Value>& args);
    Value builtin_assert(const std::vector<Value>& args);
    Value builtin_to_number(const std::vector<Value>& args);
    Value builtin_to_string(const std::vector<Value>& args);
    Value builtin_range(const std::vector<Value>& args);

public:
    VM();
    ~VM() = default;
    
    // VM operations
    InterpretResult interpret(const std::string& source);
    InterpretResult interpret_file(const std::string& filename);
    
    // Stack inspection
    const std::vector<Value>& stack() const { return stack_; }
    size_t stack_size() const { return stack_.size(); }
    
    // Global variables
    void define_global(const std::string& name, const Value& value);
    bool get_global(const std::string& name, Value& value) const;
    void set_global(const std::string& name, const Value& value);
    
    // Garbage collection
    void collect_garbage() { gc_.collect(); }
    size_t memory_usage() const { return gc_.bytes_allocated(); }
    
    // Statistics
    size_t instruction_count() const;
    double execution_time() const;
    
    // Debugging
    void print_stack_trace() const;
    void print_globals() const;
};

enum class InterpretResult {
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

} // namespace tl
