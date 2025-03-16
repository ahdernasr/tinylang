#include <catch2/catch_test_macros.hpp>
#include "tl/lexer.hpp"
#include "tl/error.hpp"

TEST_CASE("Lexer basic functionality", "[lexer]") {
    ErrorReporter error_reporter;
    
    SECTION("Numbers") {
        Lexer lexer("123 45.67", error_reporter);
        auto tokens = lexer.scan_all();
        
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].type == TokenType::NUMBER);
        REQUIRE(tokens[0].lexeme == "123");
        REQUIRE(std::get<double>(tokens[0].literal) == 123.0);
        
        REQUIRE(tokens[1].type == TokenType::NUMBER);
        REQUIRE(tokens[1].lexeme == "45.67");
        REQUIRE(std::get<double>(tokens[1].literal) == 45.67);
    }
    
    SECTION("Strings") {
        Lexer lexer("\"hello\" \"world\"", error_reporter);
        auto tokens = lexer.scan_all();
        
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].type == TokenType::STRING);
        REQUIRE(tokens[0].lexeme == "\"hello\"");
        REQUIRE(std::get<std::string>(tokens[0].literal) == "hello");
        
        REQUIRE(tokens[1].type == TokenType::STRING);
        REQUIRE(tokens[1].lexeme == "\"world\"");
        REQUIRE(std::get<std::string>(tokens[1].literal) == "world");
    }
    
    SECTION("Keywords") {
        Lexer lexer("let var fn if else while for", error_reporter);
        auto tokens = lexer.scan_all();
        
        REQUIRE(tokens.size() == 7);
        REQUIRE(tokens[0].type == TokenType::LET);
        REQUIRE(tokens[1].type == TokenType::VAR);
        REQUIRE(tokens[2].type == TokenType::FN);
        REQUIRE(tokens[3].type == TokenType::IF);
        REQUIRE(tokens[4].type == TokenType::ELSE);
        REQUIRE(tokens[5].type == TokenType::WHILE);
        REQUIRE(tokens[6].type == TokenType::FOR);
    }
    
    SECTION("Operators") {
        Lexer lexer("+ - * / % == != < <= > >=", error_reporter);
        auto tokens = lexer.scan_all();
        
        REQUIRE(tokens.size() == 10);
        REQUIRE(tokens[0].type == TokenType::PLUS);
        REQUIRE(tokens[1].type == TokenType::MINUS);
        REQUIRE(tokens[2].type == TokenType::STAR);
        REQUIRE(tokens[3].type == TokenType::SLASH);
        REQUIRE(tokens[4].type == TokenType::PERCENT);
        REQUIRE(tokens[5].type == TokenType::EQUAL_EQUAL);
        REQUIRE(tokens[6].type == TokenType::BANG_EQUAL);
        REQUIRE(tokens[7].type == TokenType::LESS);
        REQUIRE(tokens[8].type == TokenType::LESS_EQUAL);
        REQUIRE(tokens[9].type == TokenType::GREATER_EQUAL);
    }
    
    SECTION("Comments") {
        Lexer lexer("// This is a comment\n123", error_reporter);
        auto tokens = lexer.scan_all();
        
        REQUIRE(tokens.size() == 1);
        REQUIRE(tokens[0].type == TokenType::NUMBER);
        REQUIRE(tokens[0].lexeme == "123");
    }
    
    SECTION("Block comments") {
        Lexer lexer("/* This is a block comment */ 123", error_reporter);
        auto tokens = lexer.scan_all();
        
        REQUIRE(tokens.size() == 1);
        REQUIRE(tokens[0].type == TokenType::NUMBER);
        REQUIRE(tokens[0].lexeme == "123");
    }
}
