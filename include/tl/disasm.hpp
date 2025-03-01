#pragma once

#include "chunk.hpp"
#include "opcodes.hpp"
#include <string>
#include <ostream>

namespace tl {

class Disassembler {
private:
    const Chunk& chunk_;
    std::ostream& output_;
    
    // Disassembly helpers
    void disassemble_instruction(size_t offset);
    void print_instruction(const std::string& name, size_t offset);
    void print_constant_instruction(const std::string& name, size_t offset);
    void print_byte_instruction(const std::string& name, size_t offset);
    void print_jump_instruction(const std::string& name, int sign, size_t offset);
    
    // Formatting helpers
    std::string format_constant(const Value& value) const;
    std::string format_operand(uint16_t operand) const;
    std::string format_offset(size_t offset) const;
    
    // Analysis helpers
    void analyze_control_flow();
    void analyze_stack_usage();
    void print_analysis_summary();

public:
    Disassembler(const Chunk& chunk, std::ostream& output = std::cout);
    
    // Main disassembly methods
    void disassemble();
    void disassemble_range(size_t start, size_t end);
    void disassemble_instruction_at(size_t offset);
    
    // Analysis methods
    void print_constant_table();
    void print_line_table();
    void print_control_flow_graph();
    void print_stack_analysis();
    
    // Utility methods
    size_t instruction_count() const;
    size_t constant_count() const;
    size_t total_size() const;
    
    // Debugging
    void print_metadata();
    void print_statistics();
};

} // namespace tl
