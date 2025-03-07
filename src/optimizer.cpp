#include "tl/optimizer.hpp"
#include "tl/opcodes.hpp"
#include "tl/value.hpp"
#include <algorithm>
#include <cmath>

namespace tl {

void Optimizer::optimize() {
    peephole_optimize();
    dead_code_elimination();
    constant_folding();
    instruction_merging();
}

void Optimizer::peephole_optimize() {
    bool changed = true;
    while (changed) {
        changed = false;
        changed |= remove_push_pop();
        changed |= merge_jumps();
        changed |= fold_constant_arithmetic();
        changed |= replace_loadk_with_constants();
        changed |= remove_redundant_operations();
    }
}

void Optimizer::dead_code_elimination() {
    eliminate_dead_code();
    remove_unreachable_code();
}

void Optimizer::constant_folding() {
    // This is handled at the AST level in the compiler
    // Here we can do additional constant folding on bytecode
}

void Optimizer::instruction_merging() {
    merge_arithmetic_operations();
    optimize_local_access();
}

bool Optimizer::remove_push_pop() {
    bool changed = false;
    
    for (size_t i = 0; i < chunk_.code.size() - 1; i++) {
        OpCode op1 = static_cast<OpCode>(chunk_.get_byte(i));
        OpCode op2 = static_cast<OpCode>(chunk_.get_byte(i + 1));
        
        if (op1 == OpCode::OP_POP && op2 == OpCode::OP_POP) {
            // Remove duplicate POP instructions
            chunk_.remove_instruction(i);
            changed = true;
            i--; // Check the same position again
        }
    }
    
    return changed;
}

bool Optimizer::merge_jumps() {
    bool changed = false;
    
    for (size_t i = 0; i < chunk_.code.size() - 3; i++) {
        OpCode op1 = static_cast<OpCode>(chunk_.get_byte(i));
        
        if (op1 == OpCode::OP_JUMP) {
            uint16_t jump1 = chunk_.get_operand(i + 1);
            size_t target1 = i + 3 + jump1;
            
            if (target1 < chunk_.code.size()) {
                OpCode op2 = static_cast<OpCode>(chunk_.get_byte(target1));
                
                if (op2 == OpCode::OP_JUMP) {
                    // Merge consecutive jumps
                    uint16_t jump2 = chunk_.get_operand(target1 + 1);
                    size_t final_target = target1 + 3 + jump2;
                    
                    // Update the first jump to go directly to the final target
                    uint16_t new_jump = static_cast<uint16_t>(final_target - i - 3);
                    chunk_.patch_operand(i + 1, new_jump);
                    changed = true;
                }
            }
        }
    }
    
    return changed;
}

bool Optimizer::fold_constant_arithmetic() {
    bool changed = false;
    
    for (size_t i = 0; i < chunk_.code.size() - 2; i++) {
        OpCode op1 = static_cast<OpCode>(chunk_.get_byte(i));
        OpCode op2 = static_cast<OpCode>(chunk_.get_byte(i + 1));
        OpCode op3 = static_cast<OpCode>(chunk_.get_byte(i + 2));
        
        if (is_constant_opcode(op1) && is_constant_opcode(op2) && is_arithmetic_opcode(op3)) {
            // Get the constants
            Value val1 = chunk_.constants[chunk_.get_byte(i + 1)];
            Value val2 = chunk_.constants[chunk_.get_byte(i + 2)];
            
            // Compute the result
            Value result = compute_arithmetic(op3, val1, val2);
            
            if (!is_nil(result)) {
                // Replace the three instructions with a single constant
                size_t constant_index = chunk_.add_constant(result);
                chunk_.replace_instruction(i, OpCode::OP_CONSTANT, chunk_.get_line(i));
                chunk_.write_byte(static_cast<uint8_t>(constant_index), chunk_.get_line(i + 1));
                
                // Remove the other instructions
                chunk_.remove_instruction(i + 1);
                chunk_.remove_instruction(i + 1);
                
                changed = true;
            }
        }
    }
    
    return changed;
}

bool Optimizer::replace_loadk_with_constants() {
    bool changed = false;
    
    for (size_t i = 0; i < chunk_.code.size(); i++) {
        OpCode op = static_cast<OpCode>(chunk_.get_byte(i));
        
        if (op == OpCode::OP_CONSTANT) {
            uint8_t constant_index = chunk_.get_byte(i + 1);
            Value constant = chunk_.constants[constant_index];
            
            // Replace with specialized opcodes for common constants
            if (is_nil(constant)) {
                chunk_.replace_instruction(i, OpCode::OP_NIL, chunk_.get_line(i));
                chunk_.remove_instruction(i + 1);
                changed = true;
            } else if (is_bool(constant) && as_bool(constant)) {
                chunk_.replace_instruction(i, OpCode::OP_TRUE, chunk_.get_line(i));
                chunk_.remove_instruction(i + 1);
                changed = true;
            } else if (is_bool(constant) && !as_bool(constant)) {
                chunk_.replace_instruction(i, OpCode::OP_FALSE, chunk_.get_line(i));
                chunk_.remove_instruction(i + 1);
                changed = true;
            }
        }
    }
    
    return changed;
}

bool Optimizer::remove_redundant_operations() {
    bool changed = false;
    
    for (size_t i = 0; i < chunk_.code.size() - 1; i++) {
        OpCode op1 = static_cast<OpCode>(chunk_.get_byte(i));
        OpCode op2 = static_cast<OpCode>(chunk_.get_byte(i + 1));
        
        // Remove redundant operations
        if (op1 == OpCode::OP_POP && op2 == OpCode::OP_POP) {
            chunk_.remove_instruction(i);
            changed = true;
            i--;
        }
    }
    
    return changed;
}

bool Optimizer::eliminate_dead_code() {
    bool changed = false;
    
    // Simple dead code elimination for unreachable code after returns
    for (size_t i = 0; i < chunk_.code.size() - 1; i++) {
        OpCode op = static_cast<OpCode>(chunk_.get_byte(i));
        
        if (op == OpCode::OP_RETURN) {
            // Remove instructions after return until next label or end
            size_t j = i + 1;
            while (j < chunk_.code.size()) {
                OpCode next_op = static_cast<OpCode>(chunk_.get_byte(j));
                if (next_op == OpCode::OP_JUMP || next_op == OpCode::OP_JUMP_IF_FALSE) {
                    break; // Stop at control flow instructions
                }
                chunk_.remove_instruction(j);
                changed = true;
            }
        }
    }
    
    return changed;
}

bool Optimizer::remove_unreachable_code() {
    // This would require more sophisticated control flow analysis
    // For now, we'll implement a simple version
    return false;
}

bool Optimizer::merge_arithmetic_operations() {
    // This would require more sophisticated analysis
    // For now, we'll implement a simple version
    return false;
}

bool Optimizer::optimize_local_access() {
    // This would require more sophisticated analysis
    // For now, we'll implement a simple version
    return false;
}

bool Optimizer::is_constant_opcode(OpCode opcode) const {
    return opcode == OpCode::OP_CONSTANT;
}

bool Optimizer::is_arithmetic_opcode(OpCode opcode) const {
    return opcode == OpCode::OP_ADD || opcode == OpCode::OP_SUBTRACT ||
           opcode == OpCode::OP_MULTIPLY || opcode == OpCode::OP_DIVIDE ||
           opcode == OpCode::OP_MODULO;
}

bool Optimizer::is_comparison_opcode(OpCode opcode) const {
    return opcode == OpCode::OP_EQUAL || opcode == OpCode::OP_NOT_EQUAL ||
           opcode == OpCode::OP_LESS || opcode == OpCode::OP_LESS_EQUAL ||
           opcode == OpCode::OP_GREATER || opcode == OpCode::OP_GREATER_EQUAL;
}

bool Optimizer::is_jump_opcode(OpCode opcode) const {
    return opcode == OpCode::OP_JUMP || opcode == OpCode::OP_JUMP_IF_FALSE ||
           opcode == OpCode::OP_LOOP;
}

OpCode Optimizer::get_arithmetic_result(OpCode op1, OpCode op2, const Value& val1, const Value& val2) const {
    // This would be used for more sophisticated arithmetic merging
    return OpCode::OP_CONSTANT;
}

Value Optimizer::compute_arithmetic(OpCode opcode, const Value& val1, const Value& val2) const {
    if (!is_number(val1) || !is_number(val2)) {
        return make_nil();
    }
    
    double a = as_number(val1);
    double b = as_number(val2);
    
    switch (opcode) {
        case OpCode::OP_ADD:
            return make_number(a + b);
        case OpCode::OP_SUBTRACT:
            return make_number(a - b);
        case OpCode::OP_MULTIPLY:
            return make_number(a * b);
        case OpCode::OP_DIVIDE:
            if (b == 0.0) return make_nil();
            return make_number(a / b);
        case OpCode::OP_MODULO:
            if (b == 0.0) return make_nil();
            return make_number(std::fmod(a, b));
        default:
            return make_nil();
    }
}

bool Optimizer::matches_pattern(size_t offset, const std::vector<OpCode>& pattern) const {
    if (offset + pattern.size() > chunk_.code.size()) {
        return false;
    }
    
    for (size_t i = 0; i < pattern.size(); i++) {
        if (static_cast<OpCode>(chunk_.get_byte(offset + i)) != pattern[i]) {
            return false;
        }
    }
    
    return true;
}

bool Optimizer::can_optimize_sequence(size_t start, size_t end) const {
    // Check if a sequence of instructions can be optimized
    return end > start && end <= chunk_.code.size();
}

size_t Optimizer::instructions_removed() const {
    // This would track the number of instructions removed
    return 0;
}

size_t Optimizer::constants_folded() const {
    // This would track the number of constants folded
    return 0;
}

double Optimizer::optimization_ratio() const {
    size_t removed = instructions_removed();
    size_t total = chunk_.code.size() + removed;
    return total > 0 ? static_cast<double>(removed) / total : 0.0;
}

void Optimizer::print_optimization_stats() const {
    std::cout << "Optimization Statistics:" << std::endl;
    std::cout << "  Instructions removed: " << instructions_removed() << std::endl;
    std::cout << "  Constants folded: " << constants_folded() << std::endl;
    std::cout << "  Optimization ratio: " << optimization_ratio() * 100 << "%" << std::endl;
}

bool Optimizer::verify_chunk() const {
    // Verify that the chunk is still valid after optimization
    for (size_t i = 0; i < chunk_.code.size(); i++) {
        OpCode opcode = static_cast<OpCode>(chunk_.get_byte(i));
        int operand_count = opcode_operand_count(opcode);
        
        if (i + operand_count >= chunk_.code.size()) {
            return false; // Not enough operands
        }
    }
    
    return true;
}

} // namespace tl
