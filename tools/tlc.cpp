#include "tl/vm.hpp"
#include "tl/lexer.hpp"
#include "tl/parser.hpp"
#include "tl/compiler.hpp"
#include "tl/disasm.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

class CompilerCLI {
private:
    std::string output_file_;
    bool verbose_;
    bool disassemble_;
    bool optimize_;
    
public:
    CompilerCLI() : verbose_(false), disassemble_(false), optimize_(true) {}
    
    int run(int argc, char* argv[]) {
        std::vector<std::string> input_files;
        
        // Parse command line arguments
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            
            if (arg == "-o" || arg == "--output") {
                if (i + 1 < argc) {
                    output_file_ = argv[++i];
                } else {
                    std::cerr << "Error: -o requires an output filename" << std::endl;
                    return 1;
                }
            } else if (arg == "-v" || arg == "--verbose") {
                verbose_ = true;
            } else if (arg == "-d" || arg == "--disassemble") {
                disassemble_ = true;
            } else if (arg == "-O0" || arg == "--no-optimize") {
                optimize_ = false;
            } else if (arg == "-h" || arg == "--help") {
                print_help();
                return 0;
            } else if (arg[0] == '-') {
                std::cerr << "Error: Unknown option " << arg << std::endl;
                return 1;
            } else {
                input_files.push_back(arg);
            }
        }
        
        if (input_files.empty()) {
            std::cerr << "Error: No input files specified" << std::endl;
            print_help();
            return 1;
        }
        
        if (input_files.size() > 1) {
            std::cerr << "Error: Multiple input files not supported" << std::endl;
            return 1;
        }
        
        // Set default output file if not specified
        if (output_file_.empty()) {
            std::filesystem::path input_path = input_files[0];
            output_file_ = input_path.stem().string() + ".tbc";
        }
        
        return compile_file(input_files[0]);
    }
    
private:
    int compile_file(const std::string& filename) {
        if (verbose_) {
            std::cout << "Compiling " << filename << " -> " << output_file_ << std::endl;
        }
        
        // Read input file
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return 1;
        }
        
        std::string source((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        
        // Compile
        ErrorReporter error_reporter;
        error_reporter.set_source(source);
        
        // Lexical analysis
        if (verbose_) {
            std::cout << "Lexical analysis..." << std::endl;
        }
        
        Lexer lexer(source, error_reporter);
        std::vector<Token> tokens = lexer.scan_all();
        
        if (error_reporter.has_errors()) {
            std::cerr << "Lexical errors:" << std::endl;
            for (const auto& error : error_reporter.errors()) {
                std::cerr << error_reporter.format_error(error) << std::endl;
            }
            return 1;
        }
        
        // Parsing
        if (verbose_) {
            std::cout << "Parsing..." << std::endl;
        }
        
        Parser parser(tokens, error_reporter);
        std::unique_ptr<Program> program = parser.parse();
        
        if (error_reporter.has_errors()) {
            std::cerr << "Parse errors:" << std::endl;
            for (const auto& error : error_reporter.errors()) {
                std::cerr << error_reporter.format_error(error) << std::endl;
            }
            return 1;
        }
        
        // Compilation
        if (verbose_) {
            std::cout << "Compiling to bytecode..." << std::endl;
        }
        
        CompilerImpl compiler(error_reporter);
        std::shared_ptr<Function> function = compiler.compile(program);
        
        if (error_reporter.has_errors()) {
            std::cerr << "Compilation errors:" << std::endl;
            for (const auto& error : error_reporter.errors()) {
                std::cerr << error_reporter.format_error(error) << std::endl;
            }
            return 1;
        }
        
        // Optimization
        if (optimize_) {
            if (verbose_) {
                std::cout << "Optimizing..." << std::endl;
            }
            
            Optimizer optimizer(function->chunk);
            optimizer.optimize();
            
            if (verbose_) {
                optimizer.print_optimization_stats();
            }
        }
        
        // Disassembly
        if (disassemble_) {
            if (verbose_) {
                std::cout << "Disassembling..." << std::endl;
            }
            
            Disassembler disassembler(function->chunk);
            disassembler.disassemble();
            disassembler.print_statistics();
        }
        
        // Write bytecode file
        if (verbose_) {
            std::cout << "Writing bytecode to " << output_file_ << std::endl;
        }
        
        std::ofstream out_file(output_file_, std::ios::binary);
        if (!out_file.is_open()) {
            std::cerr << "Error: Could not create output file " << output_file_ << std::endl;
            return 1;
        }
        
        // Write bytecode header
        out_file.write("TBC", 3); // Magic number
        out_file.write("\x01", 1); // Version
        
        // Write chunk data
        write_chunk(out_file, function->chunk);
        
        out_file.close();
        
        if (verbose_) {
            std::cout << "Compilation successful!" << std::endl;
            std::cout << "  Instructions: " << function->chunk.code.size() << std::endl;
            std::cout << "  Constants: " << function->chunk.constants.size() << std::endl;
            std::cout << "  Output size: " << std::filesystem::file_size(output_file_) << " bytes" << std::endl;
        }
        
        return 0;
    }
    
    void write_chunk(std::ofstream& out, const Chunk& chunk) {
        // Write code
        uint32_t code_size = static_cast<uint32_t>(chunk.code.size());
        out.write(reinterpret_cast<const char*>(&code_size), sizeof(code_size));
        out.write(reinterpret_cast<const char*>(chunk.code.data()), code_size);
        
        // Write line numbers
        uint32_t line_size = static_cast<uint32_t>(chunk.lines.size());
        out.write(reinterpret_cast<const char*>(&line_size), sizeof(line_size));
        out.write(reinterpret_cast<const char*>(chunk.lines.data()), line_size * sizeof(int));
        
        // Write constants
        uint32_t const_size = static_cast<uint32_t>(chunk.constants.size());
        out.write(reinterpret_cast<const char*>(&const_size), sizeof(const_size));
        
        for (const auto& constant : chunk.constants) {
            write_value(out, constant);
        }
    }
    
    void write_value(std::ofstream& out, const Value& value) {
        if (std::holds_alternative<std::monostate>(value)) {
            out.write("\x00", 1); // nil
        } else if (std::holds_alternative<bool>(value)) {
            out.write("\x01", 1); // bool
            bool b = std::get<bool>(value);
            out.write(reinterpret_cast<const char*>(&b), sizeof(b));
        } else if (std::holds_alternative<double>(value)) {
            out.write("\x02", 1); // number
            double d = std::get<double>(value);
            out.write(reinterpret_cast<const char*>(&d), sizeof(d));
        } else if (std::holds_alternative<std::string>(value)) {
            out.write("\x03", 1); // string
            const std::string& s = std::get<std::string>(value);
            uint32_t len = static_cast<uint32_t>(s.length());
            out.write(reinterpret_cast<const char*>(&len), sizeof(len));
            out.write(s.data(), len);
        } else {
            // Function/closure - simplified for now
            out.write("\x04", 1); // function
        }
    }
    
    void print_help() {
        std::cout << "TinyLang Compiler (tlc)" << std::endl;
        std::cout << "Usage: tlc [options] <input.tl>" << std::endl;
        std::cout << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  -o, --output <file>    Output file (default: input.tbc)" << std::endl;
        std::cout << "  -v, --verbose          Verbose output" << std::endl;
        std::cout << "  -d, --disassemble      Show disassembly" << std::endl;
        std::cout << "  -O0, --no-optimize     Disable optimizations" << std::endl;
        std::cout << "  -h, --help             Show this help" << std::endl;
        std::cout << std::endl;
        std::cout << "Examples:" << std::endl;
        std::cout << "  tlc program.tl" << std::endl;
        std::cout << "  tlc -o output.tbc program.tl" << std::endl;
        std::cout << "  tlc -v -d program.tl" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    CompilerCLI cli;
    return cli.run(argc, argv);
}
