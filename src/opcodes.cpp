#include "tl/opcodes.hpp"
#include <unordered_map>

namespace tl {

const char* opcode_name(OpCode opcode) {
    static const std::unordered_map<OpCode, const char*> names = {
        {OpCode::OP_CONSTANT, "OP_CONSTANT"},
        {OpCode::OP_NIL, "OP_NIL"},
        {OpCode::OP_TRUE, "OP_TRUE"},
        {OpCode::OP_FALSE, "OP_FALSE"},
        {OpCode::OP_ADD, "OP_ADD"},
        {OpCode::OP_SUBTRACT, "OP_SUBTRACT"},
        {OpCode::OP_MULTIPLY, "OP_MULTIPLY"},
        {OpCode::OP_DIVIDE, "OP_DIVIDE"},
        {OpCode::OP_MODULO, "OP_MODULO"},
        {OpCode::OP_NEGATE, "OP_NEGATE"},
        {OpCode::OP_EQUAL, "OP_EQUAL"},
        {OpCode::OP_NOT_EQUAL, "OP_NOT_EQUAL"},
        {OpCode::OP_LESS, "OP_LESS"},
        {OpCode::OP_LESS_EQUAL, "OP_LESS_EQUAL"},
        {OpCode::OP_GREATER, "OP_GREATER"},
        {OpCode::OP_GREATER_EQUAL, "OP_GREATER_EQUAL"},
        {OpCode::OP_NOT, "OP_NOT"},
        {OpCode::OP_GET_LOCAL, "OP_GET_LOCAL"},
        {OpCode::OP_SET_LOCAL, "OP_SET_LOCAL"},
        {OpCode::OP_GET_GLOBAL, "OP_GET_GLOBAL"},
        {OpCode::OP_SET_GLOBAL, "OP_SET_GLOBAL"},
        {OpCode::OP_JUMP, "OP_JUMP"},
        {OpCode::OP_JUMP_IF_FALSE, "OP_JUMP_IF_FALSE"},
        {OpCode::OP_LOOP, "OP_LOOP"},
        {OpCode::OP_CALL, "OP_CALL"},
        {OpCode::OP_RETURN, "OP_RETURN"},
        {OpCode::OP_POP, "OP_POP"},
        {OpCode::OP_CLOSURE, "OP_CLOSURE"},
        {OpCode::OP_GET_UPVALUE, "OP_GET_UPVALUE"},
        {OpCode::OP_SET_UPVALUE, "OP_SET_UPVALUE"},
        {OpCode::OP_CLOSE_UPVALUE, "OP_CLOSE_UPVALUE"},
        {OpCode::OP_PRINT, "OP_PRINT"},
        {OpCode::OP_CLOCK, "OP_CLOCK"},
        {OpCode::OP_LEN, "OP_LEN"},
        {OpCode::OP_ASSERT, "OP_ASSERT"},
        {OpCode::OP_TO_NUMBER, "OP_TO_NUMBER"},
        {OpCode::OP_TO_STRING, "OP_TO_STRING"},
        {OpCode::OP_RANGE, "OP_RANGE"}
    };
    
    auto it = names.find(opcode);
    return (it != names.end()) ? it->second : "UNKNOWN";
}

int opcode_operand_count(OpCode opcode) {
    switch (opcode) {
        case OpCode::OP_CONSTANT:
        case OpCode::OP_GET_LOCAL:
        case OpCode::OP_SET_LOCAL:
        case OpCode::OP_GET_GLOBAL:
        case OpCode::OP_SET_GLOBAL:
        case OpCode::OP_JUMP:
        case OpCode::OP_JUMP_IF_FALSE:
        case OpCode::OP_LOOP:
        case OpCode::OP_CALL:
        case OpCode::OP_CLOSURE:
        case OpCode::OP_GET_UPVALUE:
        case OpCode::OP_SET_UPVALUE:
        case OpCode::OP_CLOSE_UPVALUE:
            return 1;
            
        case OpCode::OP_NIL:
        case OpCode::OP_TRUE:
        case OpCode::OP_FALSE:
        case OpCode::OP_ADD:
        case OpCode::OP_SUBTRACT:
        case OpCode::OP_MULTIPLY:
        case OpCode::OP_DIVIDE:
        case OpCode::OP_MODULO:
        case OpCode::OP_NEGATE:
        case OpCode::OP_EQUAL:
        case OpCode::OP_NOT_EQUAL:
        case OpCode::OP_LESS:
        case OpCode::OP_LESS_EQUAL:
        case OpCode::OP_GREATER:
        case OpCode::OP_GREATER_EQUAL:
        case OpCode::OP_NOT:
        case OpCode::OP_RETURN:
        case OpCode::OP_POP:
        case OpCode::OP_PRINT:
        case OpCode::OP_CLOCK:
        case OpCode::OP_LEN:
        case OpCode::OP_ASSERT:
        case OpCode::OP_TO_NUMBER:
        case OpCode::OP_TO_STRING:
        case OpCode::OP_RANGE:
            return 0;
            
        default:
            return 0;
    }
}

} // namespace tl
