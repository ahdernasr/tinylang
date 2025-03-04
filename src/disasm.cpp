#include "tl/disasm.hpp"
#include "tl/opcodes.hpp"
#include "tl/value.hpp"
#include <iomanip>
#include <sstream>

namespace tl {

Disassembler::Disassembler(const Chunk& chunk, std::ostream& output)
    : chunk_(chunk), output_(output) {
}

void Disassembler::disassemble() {
    output_ << "== Disassembly ==\n";
    
    size_t offset = 0;
    while (offset < chunk_.code.size()) {
        offset = disassemble_instruction(offset);
    }
    
    print_constant_table();
    print_line_table();
}

void Disassembler::disassemble_range(size_t start, size_t end) {
    output_ << "== Disassembly (range " << start << "-" << end << ") ==\n";
    
    size_t offset = start;
    while (offset < end && offset < chunk_.code.size()) {
        offset = disassemble_instruction(offset);
    }
}

void Disassembler::disassemble_instruction_at(size_t offset) {
    output_ << "== Instruction at offset " << offset << " ==\n";
    disassemble_instruction(offset);
}

size_t Disassembler::disassemble_instruction(size_t offset) {
    output_ << std::setw(4) << std::setfill('0') << std::hex << offset << " ";
    
    if (offset > 0 && chunk_.get_line(offset) == chunk_.get_line(offset - 1)) {
        output_ << "   | ";
    } else {
        output_ << std::setw(4) << std::setfill(' ') << std::dec << chunk_.get_line(offset) << " ";
    }
    
    uint8_t instruction = chunk_.get_byte(offset);
    OpCode opcode = static_cast<OpCode>(instruction);
    
    switch (opcode) {
        case OpCode::OP_CONSTANT:
            return print_constant_instruction("OP_CONSTANT", offset);
            
        case OpCode::OP_NIL:
            return print_instruction("OP_NIL", offset);
            
        case OpCode::OP_TRUE:
            return print_instruction("OP_TRUE", offset);
            
        case OpCode::OP_FALSE:
            return print_instruction("OP_FALSE", offset);
            
        case OpCode::OP_ADD:
            return print_instruction("OP_ADD", offset);
            
        case OpCode::OP_SUBTRACT:
            return print_instruction("OP_SUBTRACT", offset);
            
        case OpCode::OP_MULTIPLY:
            return print_instruction("OP_MULTIPLY", offset);
            
        case OpCode::OP_DIVIDE:
            return print_instruction("OP_DIVIDE", offset);
            
        case OpCode::OP_MODULO:
            return print_instruction("OP_MODULO", offset);
            
        case OpCode::OP_NEGATE:
            return print_instruction("OP_NEGATE", offset);
            
        case OpCode::OP_EQUAL:
            return print_instruction("OP_EQUAL", offset);
            
        case OpCode::OP_NOT_EQUAL:
            return print_instruction("OP_NOT_EQUAL", offset);
            
        case OpCode::OP_LESS:
            return print_instruction("OP_LESS", offset);
            
        case OpCode::OP_LESS_EQUAL:
            return print_instruction("OP_LESS_EQUAL", offset);
            
        case OpCode::OP_GREATER:
            return print_instruction("OP_GREATER", offset);
            
        case OpCode::OP_GREATER_EQUAL:
            return print_instruction("OP_GREATER_EQUAL", offset);
            
        case OpCode::OP_NOT:
            return print_instruction("OP_NOT", offset);
            
        case OpCode::OP_GET_LOCAL:
            return print_byte_instruction("OP_GET_LOCAL", offset);
            
        case OpCode::OP_SET_LOCAL:
            return print_byte_instruction("OP_SET_LOCAL", offset);
            
        case OpCode::OP_GET_GLOBAL:
            return print_constant_instruction("OP_GET_GLOBAL", offset);
            
        case OpCode::OP_SET_GLOBAL:
            return print_constant_instruction("OP_SET_GLOBAL", offset);
            
        case OpCode::OP_JUMP:
            return print_jump_instruction("OP_JUMP", 1, offset);
            
        case OpCode::OP_JUMP_IF_FALSE:
            return print_jump_instruction("OP_JUMP_IF_FALSE", 1, offset);
            
        case OpCode::OP_LOOP:
            return print_jump_instruction("OP_LOOP", -1, offset);
            
        case OpCode::OP_CALL:
            return print_byte_instruction("OP_CALL", offset);
            
        case OpCode::OP_RETURN:
            return print_instruction("OP_RETURN", offset);
            
        case OpCode::OP_POP:
            return print_instruction("OP_POP", offset);
            
        case OpCode::OP_CLOSURE:
            return print_constant_instruction("OP_CLOSURE", offset);
            
        case OpCode::OP_GET_UPVALUE:
            return print_byte_instruction("OP_GET_UPVALUE", offset);
            
        case OpCode::OP_SET_UPVALUE:
            return print_byte_instruction("OP_SET_UPVALUE", offset);
            
        case OpCode::OP_CLOSE_UPVALUE:
            return print_instruction("OP_CLOSE_UPVALUE", offset);
            
        case OpCode::OP_PRINT:
            return print_instruction("OP_PRINT", offset);
            
        case OpCode::OP_CLOCK:
            return print_instruction("OP_CLOCK", offset);
            
        case OpCode::OP_LEN:
            return print_instruction("OP_LEN", offset);
            
        case OpCode::OP_ASSERT:
            return print_instruction("OP_ASSERT", offset);
            
        case OpCode::OP_TO_NUMBER:
            return print_instruction("OP_TO_NUMBER", offset);
            
        case OpCode::OP_TO_STRING:
            return print_instruction("OP_TO_STRING", offset);
            
        case OpCode::OP_RANGE:
            return print_instruction("OP_RANGE", offset);
            
        default:
            output_ << "Unknown opcode " << static_cast<int>(instruction) << "\n";
            return offset + 1;
    }
}

size_t Disassembler::print_instruction(const std::string& name, size_t offset) {
    output_ << name << "\n";
    return offset + 1;
}

size_t Disassembler::print_constant_instruction(const std::string& name, size_t offset) {
    uint8_t constant_index = chunk_.get_byte(offset + 1);
    output_ << name << " " << static_cast<int>(constant_index) << " '";
    output_ << format_constant(chunk_.constants[constant_index]) << "'\n";
    return offset + 2;
}

size_t Disassembler::print_byte_instruction(const std::string& name, size_t offset) {
    uint8_t slot = chunk_.get_byte(offset + 1);
    output_ << name << " " << static_cast<int>(slot) << "\n";
    return offset + 2;
}

size_t Disassembler::print_jump_instruction(const std::string& name, int sign, size_t offset) {
    uint16_t jump = chunk_.get_operand(offset + 1);
    size_t target = offset + 3 + sign * jump;
    output_ << name << " " << static_cast<int>(jump) << " -> " << target << "\n";
    return offset + 3;
}

std::string Disassembler::format_constant(const Value& value) const {
    return value_to_string(value);
}

std::string Disassembler::format_operand(uint16_t operand) const {
    return std::to_string(operand);
}

std::string Disassembler::format_offset(size_t offset) const {
    std::ostringstream oss;
    oss << std::setw(4) << std::setfill('0') << std::hex << offset;
    return oss.str();
}

void Disassembler::print_constant_table() {
    output_ << "\n== Constants ==\n";
    for (size_t i = 0; i < chunk_.constants.size(); i++) {
        output_ << "  " << i << ": " << format_constant(chunk_.constants[i]) << "\n";
    }
}

void Disassembler::print_line_table() {
    output_ << "\n== Line Table ==\n";
    size_t last_line = 0;
    for (size_t i = 0; i < chunk_.code.size(); i++) {
        int line = chunk_.get_line(i);
        if (line != last_line) {
            output_ << "  " << i << ": " << line << "\n";
            last_line = line;
        }
    }
}

void Disassembler::print_control_flow_graph() {
    output_ << "\n== Control Flow Graph ==\n";
    // Simplified control flow analysis
    for (size_t i = 0; i < chunk_.code.size(); i++) {
        OpCode opcode = static_cast<OpCode>(chunk_.get_byte(i));
        if (opcode == OpCode::OP_JUMP || opcode == OpCode::OP_JUMP_IF_FALSE || opcode == OpCode::OP_LOOP) {
            uint16_t jump = chunk_.get_operand(i + 1);
            size_t target = i + 3 + (opcode == OpCode::OP_LOOP ? -1 : 1) * jump;
            output_ << "  " << i << " -> " << target << " (" << opcode_name(opcode) << ")\n";
        }
    }
}

void Disassembler::print_stack_analysis() {
    output_ << "\n== Stack Analysis ==\n";
    // Simplified stack depth analysis
    int stack_depth = 0;
    for (size_t i = 0; i < chunk_.code.size(); i++) {
        OpCode opcode = static_cast<OpCode>(chunk_.get_byte(i));
        
        switch (opcode) {
            case OpCode::OP_CONSTANT:
            case OpCode::OP_NIL:
            case OpCode::OP_TRUE:
            case OpCode::OP_FALSE:
            case OpCode::OP_GET_LOCAL:
            case OpCode::OP_GET_GLOBAL:
                stack_depth++;
                break;
                
            case OpCode::OP_POP:
            case OpCode::OP_SET_LOCAL:
            case OpCode::OP_SET_GLOBAL:
                stack_depth--;
                break;
                
            case OpCode::OP_ADD:
            case OpCode::OP_SUBTRACT:
            case OpCode::OP_MULTIPLY:
            case OpCode::OP_DIVIDE:
            case OpCode::OP_MODULO:
            case OpCode::OP_EQUAL:
            case OpCode::OP_NOT_EQUAL:
            case OpCode::OP_LESS:
            case OpCode::OP_LESS_EQUAL:
            case OpCode::OP_GREATER:
            case OpCode::OP_GREATER_EQUAL:
                stack_depth--; // Two operands, one result
                break;
                
            case OpCode::OP_NEGATE:
            case OpCode::OP_NOT:
                // One operand, one result - no change
                break;
                
            default:
                break;
        }
        
        if (i % 10 == 0) {
            output_ << "  " << i << ": depth=" << stack_depth << "\n";
        }
    }
}

size_t Disassembler::instruction_count() const {
    return chunk_.code.size();
}

size_t Disassembler::constant_count() const {
    return chunk_.constants.size();
}

size_t Disassembler::total_size() const {
    return chunk_.code.size() + chunk_.constants.size() * sizeof(Value);
}

void Disassembler::print_metadata() {
    output_ << "\n== Metadata ==\n";
    output_ << "  Instructions: " << instruction_count() << "\n";
    output_ << "  Constants: " << constant_count() << "\n";
    output_ << "  Total size: " << total_size() << " bytes\n";
}

void Disassembler::print_statistics() {
    print_metadata();
    print_control_flow_graph();
    print_stack_analysis();
}

} // namespace tl
