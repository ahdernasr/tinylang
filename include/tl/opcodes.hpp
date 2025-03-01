#pragma once

namespace tl {

enum class OpCode : uint8_t {
    // Constants
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    
    // Arithmetic
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_MODULO,
    OP_NEGATE,
    
    // Comparison
    OP_EQUAL,
    OP_NOT_EQUAL,
    OP_LESS,
    OP_LESS_EQUAL,
    OP_GREATER,
    OP_GREATER_EQUAL,
    OP_NOT,
    
    // Variables
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    
    // Control flow
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    
    // Functions
    OP_CALL,
    OP_RETURN,
    
    // Stack manipulation
    OP_POP,
    
    // Closures (optional)
    OP_CLOSURE,
    OP_GET_UPVALUE,
    OP_SET_UPVALUE,
    OP_CLOSE_UPVALUE,
    
    // Built-in functions
    OP_PRINT,
    OP_CLOCK,
    OP_LEN,
    OP_ASSERT,
    OP_TO_NUMBER,
    OP_TO_STRING,
    OP_RANGE
};

const char* opcode_name(OpCode opcode);
int opcode_operand_count(OpCode opcode);

} // namespace tl
