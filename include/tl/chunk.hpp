#pragma once

#include "opcodes.hpp"
#include "value.hpp"
#include <vector>
#include <cstdint>

namespace tl {

class Chunk {
public:
    std::vector<uint8_t> code;
    std::vector<Value> constants;
    std::vector<int> lines; // Line number for each instruction

    Chunk() = default;

    void write_byte(uint8_t byte, int line);
    void write_opcode(OpCode opcode, int line);
    void write_constant(Value value, int line);
    void write_operand(uint16_t operand, int line);
    
    size_t add_constant(Value value);
    size_t count() const { return code.size(); }
    
    // For disassembly
    int get_line(size_t offset) const;
    uint8_t get_byte(size_t offset) const;
    uint16_t get_operand(size_t offset) const;
    
    void patch_operand(size_t offset, uint16_t operand);
    void patch_jump(size_t offset, size_t target);
    
    // Optimization helpers
    void remove_instruction(size_t offset);
    void insert_instruction(size_t offset, uint8_t byte, int line);
    void replace_instruction(size_t offset, OpCode opcode, int line);
};

} // namespace tl
