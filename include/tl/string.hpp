#pragma once

#include "value.hpp"
#include <string>
#include <unordered_set>
#include <memory>

namespace tl {

class StringInterner {
private:
    std::unordered_set<std::string> strings_;
    
public:
    StringInterner() = default;
    
    // Intern a string - returns a reference to the interned string
    const std::string& intern(const std::string& str);
    const std::string& intern(std::string&& str);
    
    // Check if a string is interned
    bool is_interned(const std::string& str) const;
    
    // Get all interned strings (for debugging)
    const std::unordered_set<std::string>& all_strings() const { return strings_; }
    
    // Clear all interned strings
    void clear();
    
    // Statistics
    size_t count() const { return strings_.size(); }
    size_t memory_usage() const;
};

} // namespace tl
