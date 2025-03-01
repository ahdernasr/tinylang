#pragma once

#include "chunk.hpp"
#include "opcodes.hpp"
#include <vector>
#include <memory>

namespace tl {

class Optimizer {
private:
    Chunk& chunk_;
    
    // Peephole optimization patterns
    bool remove_push_pop();
    bool merge_jumps();
    bool fold_constant_arithmetic();
    bool replace_loadk_with_constants();
    bool remove_redundant_operations();
    
    // Dead code elimination
    bool eliminate_dead_code();
    bool remove_unreachable_code();
    
    // Instruction merging
    bool merge_arithmetic_operations();
    bool optimize_local_access();
    
    // Helper methods
    bool is_constant_opcode(OpCode opcode) const;
    bool is_arithmetic_opcode(OpCode opcode) const;
    bool is_comparison_opcode(OpCode opcode) const;
    bool is_jump_opcode(OpCode opcode) const;
    
    OpCode get_arithmetic_result(OpCode op1, OpCode op2, const Value& val1, const Value& val2) const;
    Value compute_arithmetic(OpCode opcode, const Value& val1, const Value& val2) const;
    
    // Pattern matching
    bool matches_pattern(size_t offset, const std::vector<OpCode>& pattern) const;
    bool can_optimize_sequence(size_t start, size_t end) const;

public:
    Optimizer(Chunk& chunk) : chunk_(chunk) {}
    
    // Main optimization entry point
    void optimize();
    
    // Individual optimization passes
    void peephole_optimize();
    void dead_code_elimination();
    void constant_folding();
    void instruction_merging();
    
    // Statistics
    size_t instructions_removed() const;
    size_t constants_folded() const;
    double optimization_ratio() const;
    
    // Debugging
    void print_optimization_stats() const;
    bool verify_chunk() const;
};

} // namespace tl
