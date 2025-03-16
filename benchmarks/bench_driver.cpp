#include "tl/vm.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <filesystem>

struct BenchmarkResult {
    std::string name;
    double execution_time_ms;
    size_t instruction_count;
    size_t memory_usage;
    size_t bytecode_size;
    bool success;
};

class BenchmarkRunner {
private:
    std::vector<BenchmarkResult> results_;
    
public:
    void run_benchmark(const std::string& filename) {
        if (!std::filesystem::exists(filename)) {
            std::cerr << "Error: File " << filename << " not found" << std::endl;
            return;
        }
        
        std::ifstream file(filename);
        std::string source((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        
        BenchmarkResult result;
        result.name = std::filesystem::path(filename).stem().string();
        
        // Run benchmark
        auto start_time = std::chrono::high_resolution_clock::now();
        
        VM vm;
        InterpretResult interpret_result = vm.interpret(source);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        
        result.execution_time_ms = duration.count() / 1000.0;
        result.instruction_count = vm.instruction_count();
        result.memory_usage = vm.memory_usage();
        result.bytecode_size = 0; // Would need to get from compiled bytecode
        result.success = (interpret_result == InterpretResult::OK);
        
        results_.push_back(result);
        
        if (result.success) {
            std::cout << "✓ " << result.name << " completed in " 
                      << std::fixed << std::setprecision(2) << result.execution_time_ms << "ms" << std::endl;
        } else {
            std::cout << "✗ " << result.name << " failed" << std::endl;
        }
    }
    
    void print_summary() {
        std::cout << "\n=== Benchmark Summary ===" << std::endl;
        std::cout << std::left << std::setw(20) << "Program" 
                  << std::setw(15) << "Time (ms)"
                  << std::setw(15) << "Instructions"
                  << std::setw(15) << "Memory (bytes)"
                  << std::setw(10) << "Status" << std::endl;
        std::cout << std::string(75, '-') << std::endl;
        
        for (const auto& result : results_) {
            std::cout << std::left << std::setw(20) << result.name
                      << std::setw(15) << std::fixed << std::setprecision(2) << result.execution_time_ms
                      << std::setw(15) << result.instruction_count
                      << std::setw(15) << result.memory_usage
                      << std::setw(10) << (result.success ? "PASS" : "FAIL") << std::endl;
        }
        
        // Calculate statistics
        if (!results_.empty()) {
            double total_time = 0;
            size_t total_instructions = 0;
            size_t total_memory = 0;
            int success_count = 0;
            
            for (const auto& result : results_) {
                total_time += result.execution_time_ms;
                total_instructions += result.instruction_count;
                total_memory += result.memory_usage;
                if (result.success) success_count++;
            }
            
            std::cout << std::string(75, '-') << std::endl;
            std::cout << std::left << std::setw(20) << "TOTAL"
                      << std::setw(15) << std::fixed << std::setprecision(2) << total_time
                      << std::setw(15) << total_instructions
                      << std::setw(15) << total_memory
                      << std::setw(10) << (success_count == results_.size() ? "PASS" : "FAIL") << std::endl;
            
            std::cout << "\nStatistics:" << std::endl;
            std::cout << "  Total programs: " << results_.size() << std::endl;
            std::cout << "  Successful: " << success_count << std::endl;
            std::cout << "  Failed: " << (results_.size() - success_count) << std::endl;
            std::cout << "  Average time: " << (total_time / results_.size()) << "ms" << std::endl;
            std::cout << "  Average instructions: " << (total_instructions / results_.size()) << std::endl;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <program1.tl> [program2.tl] ..." << std::endl;
        return 1;
    }
    
    BenchmarkRunner runner;
    
    std::cout << "Running TinyLang benchmarks..." << std::endl;
    
    for (int i = 1; i < argc; i++) {
        runner.run_benchmark(argv[i]);
    }
    
    runner.print_summary();
    
    return 0;
}
