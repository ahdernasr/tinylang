#pragma once

#include "value.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace tl {

class Table {
private:
    std::unordered_map<std::string, Value> map_;
    std::vector<std::string> keys_; // For iteration order

public:
    Table() = default;
    
    // Basic operations
    bool get(const std::string& key, Value& value) const;
    void set(const std::string& key, const Value& value);
    bool remove(const std::string& key);
    bool contains(const std::string& key) const;
    void clear();
    
    // Size and iteration
    size_t size() const { return map_.size(); }
    bool empty() const { return map_.empty(); }
    
    // Get all keys (for iteration)
    const std::vector<std::string>& keys() const { return keys_; }
    
    // Copy operations
    Table copy() const;
    void merge(const Table& other);
    
    // Debug helpers
    std::string to_string() const;
};

} // namespace tl
