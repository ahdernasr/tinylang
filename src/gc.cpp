#include "tl/gc.hpp"
#include "tl/value.hpp"
#include "tl/bytecode.hpp"
#include "tl/table.hpp"
#include <algorithm>
#include <cstring>

namespace tl {

GarbageCollector::GarbageCollector()
    : bytes_allocated_(0), next_gc_(GC_THRESHOLD), stress_mode_(false) {
}

void GarbageCollector::add_root(const Value& value) {
    roots_.push_back(value);
}

void GarbageCollector::remove_root(const Value& value) {
    auto it = std::find(roots_.begin(), roots_.end(), value);
    if (it != roots_.end()) {
        roots_.erase(it);
    }
}

void GarbageCollector::clear_roots() {
    roots_.clear();
}

void GarbageCollector::collect() {
    if (!should_collect()) {
        return;
    }
    
    mark_and_sweep();
    update_threshold();
}

void GarbageCollector::mark_and_sweep() {
    // Mark phase
    for (const auto& root : roots_) {
        mark_value(root);
    }
    
    trace_references();
    
    // Sweep phase
    sweep();
}

void* GarbageCollector::allocate(size_t size) {
    if (should_collect()) {
        collect();
    }
    
    void* ptr = std::malloc(size);
    if (ptr == nullptr) {
        throw std::bad_alloc();
    }
    
    bytes_allocated_ += size;
    return ptr;
}

void GarbageCollector::deallocate(void* ptr, size_t size) {
    if (ptr != nullptr) {
        std::free(ptr);
        bytes_allocated_ -= size;
    }
}

void GarbageCollector::set_stress_mode(bool enabled) {
    stress_mode_ = enabled;
}

bool GarbageCollector::is_stress_mode() const {
    return stress_mode_;
}

void GarbageCollector::mark_value(const Value& value) {
    if (std::holds_alternative<std::shared_ptr<Function>>(value)) {
        mark_function(std::get<std::shared_ptr<Function>>(value));
    } else if (std::holds_alternative<std::shared_ptr<Closure>>(value)) {
        mark_closure(std::get<std::shared_ptr<Closure>>(value));
    }
}

void GarbageCollector::mark_function(std::shared_ptr<Function> function) {
    if (function == nullptr) return;
    
    // Mark the function object itself
    // In a real implementation, you'd have a mark bit or use a different approach
    
    // Mark constants in the function's chunk
    for (const auto& constant : function->chunk.constants) {
        mark_value(constant);
    }
}

void GarbageCollector::mark_closure(std::shared_ptr<Closure> closure) {
    if (closure == nullptr) return;
    
    // Mark the closure object itself
    // Mark the function
    mark_function(closure->function);
    
    // Mark upvalues
    for (const auto& upvalue : closure->upvalues) {
        mark_value(upvalue);
    }
}

void GarbageCollector::mark_table(const Table& table) {
    // Mark all values in the table
    for (const auto& key : table.keys()) {
        Value value;
        if (table.get(key, value)) {
            mark_value(value);
        }
    }
}

void GarbageCollector::sweep() {
    // In a real implementation, you'd sweep through all allocated objects
    // and free those that aren't marked. For simplicity, we'll just
    // update the threshold here.
}

void GarbageCollector::trace_references() {
    // In a real implementation, you'd trace through all marked objects
    // and mark their references. This is where you'd implement the
    // gray stack and work list algorithms.
}

bool GarbageCollector::should_collect() const {
    if (stress_mode_) {
        return true;
    }
    
    return bytes_allocated_ > next_gc_;
}

void GarbageCollector::update_threshold() {
    next_gc_ = bytes_allocated_ * GC_HEAP_GROW_FACTOR;
}

} // namespace tl
