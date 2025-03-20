#pragma once

#include <iomanip>
#include <sstream>
#include <string>
#include <variant>

namespace tl {

using Value = std::variant<std::monostate, double, bool, std::string>;

inline bool is_truthy(const Value& value) {
    if (std::holds_alternative<std::monostate>(value)) {
        return false;
    }
    if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value);
    }
    if (std::holds_alternative<double>(value)) {
        return std::get<double>(value) != 0.0;
    }
    if (std::holds_alternative<std::string>(value)) {
        return !std::get<std::string>(value).empty();
    }
    return false;
}

inline std::string to_string(const Value& value) {
    if (std::holds_alternative<std::monostate>(value)) {
        return "nil";
    }
    if (std::holds_alternative<double>(value)) {
        double number = std::get<double>(value);
        std::ostringstream oss;
        oss << std::setprecision(12) << number;
        std::string result = oss.str();
        // Trim trailing zeros and decimal point for integers
        if (result.find('.') != std::string::npos) {
            while (!result.empty() && result.back() == '0') {
                result.pop_back();
            }
            if (!result.empty() && result.back() == '.') {
                result.pop_back();
            }
        }
        if (result.empty()) {
            result = "0";
        }
        return result;
    }
    if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value) ? "true" : "false";
    }
    if (std::holds_alternative<std::string>(value)) {
        return std::get<std::string>(value);
    }
    return "unknown";
}

inline bool values_equal(const Value& a, const Value& b) {
    return a == b;
}

} // namespace tl

