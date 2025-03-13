#include "tl/vm.hpp"
#include "tl/lexer.hpp"
#include "tl/parser.hpp"
#include "tl/compiler.hpp"
#include "tl/disasm.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>

class REPL {
private:
    VM vm_;
    std::vector<std::string> history_;
    size_t history_index_;
    
public:
    REPL() : history_index_(0) {
        std::cout << "TinyLang REPL v1.0.0" << std::endl;
        std::cout << "Type :help for commands, :quit to exit" << std::endl;
    }
    
    void run() {
        std::string line;
        std::string input;
        bool in_multiline = false;
        
        while (true) {
            if (!in_multiline) {
                std::cout << "tl> ";
            } else {
                std::cout << "  > ";
            }
            
            if (!std::getline(std::cin, line)) {
                break;
            }
            
            // Handle commands
            if (line.empty()) {
                if (in_multiline) {
                    // Execute multiline input
                    execute_input(input);
                    input.clear();
                    in_multiline = false;
                }
                continue;
            }
            
            if (line[0] == ':') {
                handle_command(line);
                continue;
            }
            
            // Check for multiline input
            if (line.back() == '\\') {
                input += line.substr(0, line.length() - 1) + "\n";
                in_multiline = true;
                continue;
            }
            
            if (in_multiline) {
                input += line + "\n";
                execute_input(input);
                input.clear();
                in_multiline = false;
            } else {
                execute_input(line);
            }
        }
        
        std::cout << "Goodbye!" << std::endl;
    }
    
private:
    void execute_input(const std::string& input) {
        if (input.empty()) return;
        
        // Add to history
        history_.push_back(input);
        history_index_ = history_.size();
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        InterpretResult result = vm_.interpret(input);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        
        if (result == InterpretResult::OK) {
            std::cout << "✓ Executed in " << duration.count() / 1000.0 << "ms" << std::endl;
            std::cout << "  Stack depth: " << vm_.stack_size() << std::endl;
            std::cout << "  Instructions: " << vm_.instruction_count() << std::endl;
        } else {
            std::cout << "✗ Error" << std::endl;
        }
    }
    
    void handle_command(const std::string& command) {
        if (command == ":help") {
            print_help();
        } else if (command == ":quit" || command == ":exit") {
            std::cout << "Goodbye!" << std::endl;
            exit(0);
        } else if (command == ":clear") {
            system("clear");
        } else if (command == ":history") {
            print_history();
        } else if (command == ":stack") {
            print_stack();
        } else if (command == ":globals") {
            vm_.print_globals();
        } else if (command == ":gc") {
            vm_.collect_garbage();
            std::cout << "Garbage collection completed" << std::endl;
        } else if (command == ":stats") {
            print_stats();
        } else if (command == ":ast") {
            std::cout << "AST visualization not implemented yet" << std::endl;
        } else if (command == ":dis") {
            std::cout << "Disassembly not available in REPL mode" << std::endl;
        } else {
            std::cout << "Unknown command: " << command << std::endl;
            std::cout << "Type :help for available commands" << std::endl;
        }
    }
    
    void print_help() {
        std::cout << "TinyLang REPL Commands:" << std::endl;
        std::cout << "  :help     - Show this help message" << std::endl;
        std::cout << "  :quit     - Exit the REPL" << std::endl;
        std::cout << "  :clear    - Clear the screen" << std::endl;
        std::cout << "  :history  - Show command history" << std::endl;
        std::cout << "  :stack    - Show current stack contents" << std::endl;
        std::cout << "  :globals  - Show global variables" << std::endl;
        std::cout << "  :gc       - Force garbage collection" << std::endl;
        std::cout << "  :stats    - Show execution statistics" << std::endl;
        std::cout << "  :ast      - Show AST (not implemented)" << std::endl;
        std::cout << "  :dis      - Show disassembly (not available in REPL)" << std::endl;
        std::cout << std::endl;
        std::cout << "Multiline input: End lines with \\ to continue" << std::endl;
        std::cout << "Examples:" << std::endl;
        std::cout << "  let x = 42;" << std::endl;
        std::cout << "  print(x);" << std::endl;
        std::cout << "  fn fib(n) { \\" << std::endl;
        std::cout << "    if (n <= 1) return n; \\" << std::endl;
        std::cout << "    return fib(n-1) + fib(n-2); \\" << std::endl;
        std::cout << "  }" << std::endl;
    }
    
    void print_history() {
        std::cout << "Command History:" << std::endl;
        for (size_t i = 0; i < history_.size(); i++) {
            std::cout << "  " << std::setw(3) << i + 1 << ": " << history_[i] << std::endl;
        }
    }
    
    void print_stack() {
        std::cout << "Stack Contents:" << std::endl;
        const auto& stack = vm_.stack();
        if (stack.empty()) {
            std::cout << "  (empty)" << std::endl;
        } else {
            for (size_t i = 0; i < stack.size(); i++) {
                std::cout << "  " << i << ": " << value_to_string(stack[i]) << std::endl;
            }
        }
    }
    
    void print_stats() {
        std::cout << "Execution Statistics:" << std::endl;
        std::cout << "  Instructions executed: " << vm_.instruction_count() << std::endl;
        std::cout << "  Execution time: " << vm_.execution_time() << "ms" << std::endl;
        std::cout << "  Memory usage: " << vm_.memory_usage() << " bytes" << std::endl;
        std::cout << "  Stack depth: " << vm_.stack_size() << std::endl;
    }
};

int main() {
    try {
        REPL repl;
        repl.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
