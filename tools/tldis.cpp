#include "tl/chunk.hpp"
#include "tl/disasm.hpp"
#include "tl/value.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

class DisassemblerCLI {
private:
    bool verbose_;
    bool show_constants_;
    bool show_lines_;
    bool show_flow_;
    bool show_stack_;
    
public:
    DisassemblerCLI() : verbose_(false), show_constants_(true), show_lines_(true), 
                       show_flow_(false), show_stack_(false) {}
    
    int run(int argc, char* argv[]) {
        std::string input_file;
        
        // Parse command line arguments
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            
            if (arg == "-v" || arg == "--verbose") {
                verbose_ = true;
            } else if (arg == "--no-constants") {
                show_constants_ = false;
            } else if (arg == "--no-lines") {
                show_lines_ = false;
            } else if (arg == "--flow") {
                show_flow_ = true;
            } else if (arg == "--stack") {
                show_stack_ = true;
            } else if (arg == "-h" || arg == "--help") {
                print_help();
                return 0;
            } else if (arg[0] == '-') {
                std::cerr << "Error: Unknown option " << arg << std::endl;
                return 1;
            } else {
                if (input_file.empty()) {
                    input_file = arg;
                } else {
                    std::cerr << "Error: Multiple input files not supported" << std::endl;
                    return 1;
                }
            }
        }
        
        if (input_file.empty()) {
            std::cerr << "Error: No input file specified" << std::endl;
            print_help();
            return 1;
        }
        
        return disassemble_file(input_file);
    }
    
private:
    int disassemble_file(const std::string& filename) {
        if (verbose_) {
            std::cout << "Disassembling " << filename << std::endl;
        }
        
        // Read bytecode file
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return 1;
        }
        
        // Read header
        char magic[4];
        file.read(magic, 3);
        magic[3] = '\0';
        
        if (std::string(magic) != "TBC") {
            std::cerr << "Error: Invalid bytecode file format" << std::endl;
            return 1;
        }
        
        char version;
        file.read(&version, 1);
        
        if (version != 1) {
            std::cerr << "Error: Unsupported bytecode version " << static_cast<int>(version) << std::endl;
            return 1;
        }
        
        // Read chunk data
        Chunk chunk = read_chunk(file);
        file.close();
        
        if (verbose_) {
            std::cout << "Bytecode file loaded successfully" << std::endl;
        }
        
        // Disassemble
        Disassembler disassembler(chunk);
        
        if (show_constants_) {
            disassembler.print_constant_table();
        }
        
        if (show_lines_) {
            disassembler.print_line_table();
        }
        
        disassembler.disassemble();
        
        if (show_flow_) {
            disassembler.print_control_flow_graph();
        }
        
        if (show_stack_) {
            disassembler.print_stack_analysis();
        }
        
        if (verbose_) {
            disassembler.print_statistics();
        }
        
        return 0;
    }
    
    Chunk read_chunk(std::ifstream& file) {
        Chunk chunk;
        
        // Read code
        uint32_t code_size;
        file.read(reinterpret_cast<char*>(&code_size), sizeof(code_size));
        
        chunk.code.resize(code_size);
        file.read(reinterpret_cast<char*>(chunk.code.data()), code_size);
        
        // Read line numbers
        uint32_t line_size;
        file.read(reinterpret_cast<char*>(&line_size), sizeof(line_size));
        
        chunk.lines.resize(line_size);
        file.read(reinterpret_cast<char*>(chunk.lines.data()), line_size * sizeof(int));
        
        // Read constants
        uint32_t const_size;
        file.read(reinterpret_cast<char*>(&const_size), sizeof(const_size));
        
        chunk.constants.resize(const_size);
        for (uint32_t i = 0; i < const_size; i++) {
            chunk.constants[i] = read_value(file);
        }
        
        return chunk;
    }
    
    Value read_value(std::ifstream& file) {
        char type;
        file.read(&type, 1);
        
        switch (type) {
            case 0: // nil
                return make_nil();
                
            case 1: // bool
                bool b;
                file.read(reinterpret_cast<char*>(&b), sizeof(b));
                return make_bool(b);
                
            case 2: // number
                double d;
                file.read(reinterpret_cast<char*>(&d), sizeof(d));
                return make_number(d);
                
            case 3: // string
                uint32_t len;
                file.read(reinterpret_cast<char*>(&len), sizeof(len));
                std::string s(len, '\0');
                file.read(s.data(), len);
                return make_string(s);
                
            case 4: // function
                return make_string("<function>");
                
            default:
                return make_nil();
        }
    }
    
    void print_help() {
        std::cout << "TinyLang Disassembler (tldis)" << std::endl;
        std::cout << "Usage: tldis [options] <input.tbc>" << std::endl;
        std::cout << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  -v, --verbose          Verbose output" << std::endl;
        std::cout << "  --no-constants         Don't show constant table" << std::endl;
        std::cout << "  --no-lines             Don't show line table" << std::endl;
        std::cout << "  --flow                 Show control flow graph" << std::endl;
        std::cout << "  --stack                Show stack analysis" << std::endl;
        std::cout << "  -h, --help             Show this help" << std::endl;
        std::cout << std::endl;
        std::cout << "Examples:" << std::endl;
        std::cout << "  tldis program.tbc" << std::endl;
        std::cout << "  tldis -v --flow program.tbc" << std::endl;
        std::cout << "  tldis --no-constants program.tbc" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    DisassemblerCLI cli;
    return cli.run(argc, argv);
}
