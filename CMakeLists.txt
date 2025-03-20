cmake_minimum_required(VERSION 3.16)
project(tinylang LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(tinylang
    src/ast.cpp
    src/lexer.cpp
    src/parser.cpp
    src/vm.cpp
)

target_include_directories(tinylang PUBLIC include)

add_executable(tl src/main_repl.cpp)
target_link_libraries(tl PRIVATE tinylang)

