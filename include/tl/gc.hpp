#pragma once

#include "value.hpp"
#include "bytecode.hpp"
#include <vector>
#include <unordered_set>
#include <memory>

namespace tl {

class GarbageCollector {
private:
    std::vector<Value> roots_;
    std::unordered_set<void*> gray_stack_;
    size_t bytes_allocated_;
    size_t next_gc_;
    
    static const size_t GC_HEAP_GROW_FACTOR = 2;
    static const size_t GC_THRESHOLD = 1024 * 1024; // 1MB

public:
    GarbageCollector();
    
    // Root management
    void add_root(const Value& value);
    void remove_root(const Value& value);
    void clear_roots();
    
    // Collection
    void collect();
    void mark_and_sweep();
    
    // Memory management
    void* allocate(size_t size);
    void deallocate(void* ptr, size_t size);
    
    // Statistics
    size_t bytes_allocated() const { return bytes_allocated_; }
    size_t next_gc_threshold() const { return next_gc_; }
    
    // Stress testing
    void set_stress_mode(bool enabled);
    bool is_stress_mode() const;

private:
    void mark_value(const Value& value);
    void mark_function(std::shared_ptr<Function> function);
    void mark_closure(std::shared_ptr<Closure> closure);
    void mark_table(const Table& table);
    
    void sweep();
    void trace_references();
    
    bool should_collect() const;
    void update_threshold();
};

} // namespace tl
