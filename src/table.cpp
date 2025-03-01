#include "tl/table.hpp"
#include "tl/value.hpp"
#include <sstream>

namespace tl {

bool Table::get(const std::string& key, Value& value) const {
    auto it = map_.find(key);
    if (it != map_.end()) {
        value = it->second;
        return true;
    }
    return false;
}

void Table::set(const std::string& key, const Value& value) {
    auto it = map_.find(key);
    if (it == map_.end()) {
        keys_.push_back(key);
    }
    map_[key] = value;
}

bool Table::remove(const std::string& key) {
    auto it = map_.find(key);
    if (it != map_.end()) {
        map_.erase(it);
        keys_.erase(std::remove(keys_.begin(), keys_.end(), key), keys_.end());
        return true;
    }
    return false;
}

bool Table::contains(const std::string& key) const {
    return map_.find(key) != map_.end();
}

void Table::clear() {
    map_.clear();
    keys_.clear();
}

Table Table::copy() const {
    Table new_table;
    new_table.map_ = map_;
    new_table.keys_ = keys_;
    return new_table;
}

void Table::merge(const Table& other) {
    for (const auto& pair : other.map_) {
        set(pair.first, pair.second);
    }
}

std::string Table::to_string() const {
    if (map_.empty()) {
        return "{}";
    }
    
    std::ostringstream oss;
    oss << "{";
    
    bool first = true;
    for (const auto& key : keys_) {
        if (!first) {
            oss << ", ";
        }
        first = false;
        
        oss << key << ": " << value_to_string(map_.at(key));
    }
    
    oss << "}";
    return oss.str();
}

} // namespace tl
