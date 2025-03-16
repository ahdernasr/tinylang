#include <catch2/catch_test_macros.hpp>
#include "tl/vm.hpp"

TEST_CASE("VM basic functionality", "[vm]") {
    VM vm;
    
    SECTION("Arithmetic") {
        InterpretResult result = vm.interpret("print(2 + 3);");
        REQUIRE(result == InterpretResult::OK);
        
        result = vm.interpret("print(10 - 4);");
        REQUIRE(result == InterpretResult::OK);
        
        result = vm.interpret("print(3 * 4);");
        REQUIRE(result == InterpretResult::OK);
        
        result = vm.interpret("print(15 / 3);");
        REQUIRE(result == InterpretResult::OK);
    }
    
    SECTION("Variables") {
        InterpretResult result = vm.interpret("let x = 42; print(x);");
        REQUIRE(result == InterpretResult::OK);
        
        result = vm.interpret("var y = 10; y = 20; print(y);");
        REQUIRE(result == InterpretResult::OK);
    }
    
    SECTION("Functions") {
        InterpretResult result = vm.interpret(
            "fn add(a, b) { return a + b; } print(add(3, 4));"
        );
        REQUIRE(result == InterpretResult::OK);
    }
    
    SECTION("Control flow") {
        InterpretResult result = vm.interpret(
            "if (true) { print(\"true branch\"); } else { print(\"false branch\"); }"
        );
        REQUIRE(result == InterpretResult::OK);
        
        result = vm.interpret(
            "let i = 0; while (i < 3) { print(i); i = i + 1; }"
        );
        REQUIRE(result == InterpretResult::OK);
    }
    
    SECTION("Built-in functions") {
        InterpretResult result = vm.interpret("print(clock());");
        REQUIRE(result == InterpretResult::OK);
        
        result = vm.interpret("print(len(\"hello\"));");
        REQUIRE(result == InterpretResult::OK);
        
        result = vm.interpret("print(toNumber(\"123\"));");
        REQUIRE(result == InterpretResult::OK);
        
        result = vm.interpret("print(toString(456));");
        REQUIRE(result == InterpretResult::OK);
    }
    
    SECTION("Error handling") {
        InterpretResult result = vm.interpret("print(undefined_var);");
        REQUIRE(result == InterpretResult::RUNTIME_ERROR);
        
        result = vm.interpret("1 / 0;");
        REQUIRE(result == InterpretResult::RUNTIME_ERROR);
    }
}
