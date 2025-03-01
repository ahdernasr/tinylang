#include "tl/string.hpp"

namespace tl {

const std::string& StringInterner::intern(const std::string& str) {
    auto it = strings_.find(str);
    if (it != strings_.end()) {
        return *it;
    }
    
    auto result = strings_.insert(str);
    return *result.first;
}

const std::string& StringInterner::intern(std::string&& str) {
    auto it = strings_.find(str);
    if (it != strings_.end()) {
        return *it;
    }
    
    auto result = strings_.insert(std::move(str));
    return *result.first;
}

bool StringInterner::is_interned(const std::string& str) const {
    return strings_.find(str) != strings_.end();
}

void StringInterner::clear() {
    strings_.clear();
}

size_t StringInterner::memory_usage() const {
    size_t total = 0;
    for (const auto& str : strings_) {
        total += str.capacity();
    }
    return total;
}

} // namespace tl
