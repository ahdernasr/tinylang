# Test configuration
CPMAddPackage(
    NAME Catch2
    GITHUB_REPOSITORY catchorg/Catch2
    VERSION 3.5.2
)

if(Catch2_ADDED)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    include(CTest)
    include(Catch)
    
    # Test executables
    add_executable(lexer_test lexer_test.cpp)
    target_link_libraries(lexer_test tinylang Catch2::Catch2WithMain)
    
    add_executable(parser_test parser_test.cpp)
    target_link_libraries(parser_test tinylang Catch2::Catch2WithMain)
    
    add_executable(compiler_test compiler_test.cpp)
    target_link_libraries(compiler_test tinylang Catch2::Catch2WithMain)
    
    add_executable(vm_test vm_test.cpp)
    target_link_libraries(vm_test tinylang Catch2::Catch2WithMain)
    
    add_executable(optimizer_test optimizer_test.cpp)
    target_link_libraries(optimizer_test tinylang Catch2::Catch2WithMain)
    
    add_executable(stdlib_test stdlib_test.cpp)
    target_link_libraries(stdlib_test tinylang Catch2::Catch2WithMain)
    
    # Add tests to CTest
    catch_discover_tests(lexer_test)
    catch_discover_tests(parser_test)
    catch_discover_tests(compiler_test)
    catch_discover_tests(vm_test)
    catch_discover_tests(optimizer_test)
    catch_discover_tests(stdlib_test)
    
    # Custom test target
    add_custom_target(test_all
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS lexer_test parser_test compiler_test vm_test optimizer_test stdlib_test
        COMMENT "Running all tests"
    )
endif()
