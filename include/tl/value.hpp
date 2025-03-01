#pragma once

#include <string>
#include <variant>
#include <vector>
#include <memory>

namespace tl {

// Forward declarations
class Function;
class Closure;

// Value type using std::variant for simplicity
// In a production system, you might use NaN-boxing for better performance
using Value = std::variant<
    std::monostate,  // nil
    bool,            // boolean
    double,          // number
    std::string,     // string
    std::shared_ptr<Function>,  // function
    std::shared_ptr<Closure>    // closure
>;

// Type checking helpers
inline bool is_nil(const Value& value) {
    return std::holds_alternative<std::monostate>(value);
}

inline bool is_bool(const Value& value) {
    return std::holds_alternative<bool>(value);
}

inline bool is_number(const Value& value) {
    return std::holds_alternative<double>(value);
}

inline bool is_string(const Value& value) {
    return std::holds_alternative<std::string>(value);
}

inline bool is_function(const Value& value) {
    return std::holds_alternative<std::shared_ptr<Function>>(value);
}

inline bool is_closure(const Value& value) {
    return std::holds_alternative<std::shared_ptr<Closure>>(value);
}

// Value access helpers
inline bool as_bool(const Value& value) {
    if (is_bool(value)) return std::get<bool>(value);
    if (is_nil(value)) return false;
    if (is_number(value)) return std::get<double>(value) != 0.0;
    if (is_string(value)) return !std::get<std::string>(value).empty();
    return true; // functions and closures are truthy
}

inline double as_number(const Value& value) {
    if (is_number(value)) return std::get<double>(value);
    if (is_bool(value)) return std::get<bool>(value) ? 1.0 : 0.0;
    if (is_nil(value)) return 0.0;
    if (is_string(value)) {
        try {
            return std::stod(std::get<std::string>(value));
        } catch (...) {
            return 0.0;
        }
    }
    return 0.0;
}

inline std::string as_string(const Value& value) {
    if (is_string(value)) return std::get<std::string>(value);
    if (is_nil(value)) return "nil";
    if (is_bool(value)) return std::get<bool>(value) ? "true" : "false";
    if (is_number(value)) {
        double num = std::get<double>(value);
        if (num == static_cast<long long>(num)) {
            return std::to_string(static_cast<long long>(num));
        }
        return std::to_string(num);
    }
    if (is_function(value)) return "<function>";
    if (is_closure(value)) return "<closure>";
    return "unknown";
}

// Value creation helpers
inline Value make_nil() { return std::monostate{}; }
inline Value make_bool(bool b) { return b; }
inline Value make_number(double n) { return n; }
inline Value make_string(const std::string& s) { return s; }

// Value comparison
bool values_equal(const Value& a, const Value& b);
bool values_less(const Value& a, const Value& b);

// String representation for debugging
std::string value_to_string(const Value& value);

} // namespace tl
