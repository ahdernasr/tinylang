#pragma once

#include "chunk.hpp"
#include "value.hpp"
#include <string>
#include <vector>
#include <memory>

namespace tl {

// Forward declarations
class Function;
class Closure;

// Upvalue for closures
struct Upvalue {
    uint8_t index;
    bool is_local;
    
    Upvalue(uint8_t index, bool is_local) : index(index), is_local(is_local) {}
};

// Function object
class Function {
public:
    std::string name;
    uint8_t arity;
    Chunk chunk;
    std::vector<std::string> locals;
    
    Function() : arity(0) {}
    Function(const std::string& name) : name(name), arity(0) {}
};

// Closure object
class Closure {
public:
    std::shared_ptr<Function> function;
    std::vector<Value> upvalues;
    
    Closure(std::shared_ptr<Function> function) : function(function) {}
};

// Call frame for the VM
struct CallFrame {
    std::shared_ptr<Closure> closure;
    uint8_t* ip;  // Instruction pointer
    Value* slots; // Local variables
    
    CallFrame(std::shared_ptr<Closure> closure, Value* slots)
        : closure(closure), ip(closure->function->chunk.code.data()), slots(slots) {}
};

} // namespace tl
