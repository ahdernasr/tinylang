#include "tl/vm.hpp"

#include <iostream>
#include <sstream>
#include <string>

namespace {

std::string trim(const std::string& str) {
    const auto first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    const auto last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

} // namespace

int main() {
    tl::VM vm;

    std::cout << "TinyLang (minimal)" << std::endl;
    std::cout << "Type :quit to exit" << std::endl;

    std::string buffer;

    while (true) {
        std::cout << "tl> " << std::flush;
        std::string line;
        if (!std::getline(std::cin, line)) {
            break;
        }

        auto trimmed = trim(line);
        if (trimmed == ":quit" || trimmed == ":exit") {
            break;
        }
        if (trimmed.empty()) {
            continue;
        }

        buffer += line;
        buffer += '\n';

        if (trimmed.back() != ';') {
            continue;
        }

        auto result = vm.interpret(buffer);
        if (result == tl::InterpretResult::OK) {
            buffer.clear();
        } else {
            buffer.clear();
        }
    }

    std::cout << "Goodbye!" << std::endl;
    return 0;
}

