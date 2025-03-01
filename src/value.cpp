#include "tl/value.hpp"
#include "tl/bytecode.hpp"
#include <sstream>
#include <cmath>
#include <iomanip>

namespace tl {

bool values_equal(const Value& a, const Value& b) {
    if (std::holds_alternative<std::monostate>(a) && std::holds_alternative<std::monostate>(b)) {
        return true;
    }
    
    if (std::holds_alternative<bool>(a) && std::holds_alternative<bool>(b)) {
        return std::get<bool>(a) == std::get<bool>(b);
    }
    
    if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b)) {
        double num_a = std::get<double>(a);
        double num_b = std::get<double>(b);
        
        // Handle NaN comparison
        if (std::isnan(num_a) && std::isnan(num_b)) {
            return true;
        }
        
        return num_a == num_b;
    }
    
    if (std::holds_alternative<std::string>(a) && std::holds_alternative<std::string>(b)) {
        return std::get<std::string>(a) == std::get<std::string>(b);
    }
    
    if (std::holds_alternative<std::shared_ptr<Function>>(a) && std::holds_alternative<std::shared_ptr<Function>>(b)) {
        return std::get<std::shared_ptr<Function>>(a) == std::get<std::shared_ptr<Function>>(b);
    }
    
    if (std::holds_alternative<std::shared_ptr<Closure>>(a) && std::holds_alternative<std::shared_ptr<Closure>>(b)) {
        return std::get<std::shared_ptr<Closure>>(a) == std::get<std::shared_ptr<Closure>>(b);
    }
    
    return false;
}

bool values_less(const Value& a, const Value& b) {
    if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b)) {
        return std::get<double>(a) < std::get<double>(b);
    }
    
    if (std::holds_alternative<std::string>(a) && std::holds_alternative<std::string>(b)) {
        return std::get<std::string>(a) < std::get<std::string>(b);
    }
    
    // Mixed types: convert to numbers
    double num_a = as_number(a);
    double num_b = as_number(b);
    
    return num_a < num_b;
}

std::string value_to_string(const Value& value) {
    if (std::holds_alternative<std::monostate>(value)) {
        return "nil";
    }
    
    if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value) ? "true" : "false";
    }
    
    if (std::holds_alternative<double>(value)) {
        double num = std::get<double>(value);
        
        // Handle special floating point values
        if (std::isnan(num)) {
            return "nan";
        }
        if (std::isinf(num)) {
            return num > 0 ? "inf" : "-inf";
        }
        
        // Format integers without decimal point
        if (num == static_cast<long long>(num)) {
            return std::to_string(static_cast<long long>(num));
        }
        
        // Format floating point numbers
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(6) << num;
        std::string result = oss.str();
        
        // Remove trailing zeros
        result.erase(result.find_last_not_of('0') + 1, std::string::npos);
        if (result.back() == '.') {
            result += '0';
        }
        
        return result;
    }
    
    if (std::holds_alternative<std::string>(value)) {
        return std::get<std::string>(value);
    }
    
    if (std::holds_alternative<std::shared_ptr<Function>>(value)) {
        auto func = std::get<std::shared_ptr<Function>>(value);
        if (func->name.empty()) {
            return "<script>";
        }
        return "<fn " + func->name + ">";
    }
    
    if (std::holds_alternative<std::shared_ptr<Closure>>(value)) {
        auto closure = std::get<std::shared_ptr<Closure>>(value);
        if (closure->function->name.empty()) {
            return "<script>";
        }
        return "<fn " + closure->function->name + ">";
    }
    
    return "unknown";
}

} // namespace tl
