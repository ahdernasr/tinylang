#include "tl/chunk.hpp"
#include "tl/opcodes.hpp"
#include <algorithm>

namespace tl {

void Chunk::write_byte(uint8_t byte, int line) {
    code.push_back(byte);
    lines.push_back(line);
}

void Chunk::write_opcode(OpCode opcode, int line) {
    write_byte(static_cast<uint8_t>(opcode), line);
}

void Chunk::write_constant(Value value, int line) {
    size_t constant_index = add_constant(value);
    if (constant_index > 255) {
        // For now, we'll limit to 256 constants
        // In a production system, you'd use multi-byte encoding
        throw std::runtime_error("Too many constants in one chunk");
    }
    write_byte(static_cast<uint8_t>(constant_index), line);
}

void Chunk::write_operand(uint16_t operand, int line) {
    // Write operand as two bytes (little-endian)
    write_byte(static_cast<uint8_t>(operand & 0xFF), line);
    write_byte(static_cast<uint8_t>((operand >> 8) & 0xFF), line);
}

size_t Chunk::add_constant(Value value) {
    constants.push_back(value);
    return constants.size() - 1;
}

int Chunk::get_line(size_t offset) const {
    if (offset >= lines.size()) {
        return 0;
    }
    return lines[offset];
}

uint8_t Chunk::get_byte(size_t offset) const {
    if (offset >= code.size()) {
        return 0;
    }
    return code[offset];
}

uint16_t Chunk::get_operand(size_t offset) const {
    if (offset + 1 >= code.size()) {
        return 0;
    }
    uint8_t low = code[offset];
    uint8_t high = code[offset + 1];
    return static_cast<uint16_t>(low | (high << 8));
}

void Chunk::patch_operand(size_t offset, uint16_t operand) {
    if (offset + 1 >= code.size()) {
        return;
    }
    code[offset] = static_cast<uint8_t>(operand & 0xFF);
    code[offset + 1] = static_cast<uint8_t>((operand >> 8) & 0xFF);
}

void Chunk::patch_jump(size_t offset, size_t target) {
    if (offset + 1 >= code.size()) {
        return;
    }
    
    int jump_distance = static_cast<int>(target) - static_cast<int>(offset) - 2;
    if (jump_distance < 0 || jump_distance > 65535) {
        throw std::runtime_error("Jump distance too large");
    }
    
    patch_operand(offset, static_cast<uint16_t>(jump_distance));
}

void Chunk::remove_instruction(size_t offset) {
    if (offset >= code.size()) {
        return;
    }
    
    code.erase(code.begin() + offset);
    lines.erase(lines.begin() + offset);
}

void Chunk::insert_instruction(size_t offset, uint8_t byte, int line) {
    if (offset > code.size()) {
        offset = code.size();
    }
    
    code.insert(code.begin() + offset, byte);
    lines.insert(lines.begin() + offset, line);
}

void Chunk::replace_instruction(size_t offset, OpCode opcode, int line) {
    if (offset >= code.size()) {
        return;
    }
    
    code[offset] = static_cast<uint8_t>(opcode);
    lines[offset] = line;
}

} // namespace tl
