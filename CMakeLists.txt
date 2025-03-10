cmake_minimum_required(VERSION 3.16)
project(tinylang VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include CPM
include(cmake/CPM.cmake)

# Compiler options
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Build type specific options
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(TL_DEBUG=1)
    if(MSVC)
        add_compile_options(/Od /Zi)
    else()
        add_compile_options(-g -O0)
    endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        add_compile_options(/O2)
    else()
        add_compile_options(-O3 -DNDEBUG)
    endif()
endif()

# Optional features
option(TL_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(TL_ENABLE_GC_STRESS "Enable GC stress testing" OFF)
option(TL_ENABLE_CLOSURES "Enable closures support" ON)

if(TL_ENABLE_ASAN)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

if(TL_ENABLE_GC_STRESS)
    add_compile_definitions(TL_GC_STRESS=1)
endif()

if(TL_ENABLE_CLOSURES)
    add_compile_definitions(TL_CLOSURES=1)
endif()

# Include directories
include_directories(include)

# Source files
set(TL_SOURCES
    src/lexer.cpp
    src/parser.cpp
    src/ast.cpp
    src/compiler.cpp
    src/optimizer.cpp
    src/bytecode.cpp
    src/vm.cpp
    src/value.cpp
    src/error.cpp
    src/opcodes.cpp
    src/chunk.cpp
    src/gc.cpp
    src/table.cpp
    src/string.cpp
    src/disasm.cpp
)

# Create library
add_library(tinylang ${TL_SOURCES})

# Set target properties
set_target_properties(tinylang PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Executables
add_executable(tl src/main_repl.cpp)
target_link_libraries(tl tinylang)

add_executable(tlc tools/tlc.cpp)
target_link_libraries(tlc tinylang)

add_executable(tldis tools/tldis.cpp)
target_link_libraries(tldis tinylang)

# Tests
enable_testing()
add_subdirectory(tests)

# Benchmarks
add_subdirectory(benchmarks)

# Installation
install(TARGETS tl tlc tldis
    RUNTIME DESTINATION bin
)

install(TARGETS tinylang
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
)

# Package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    tinylangConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    cmake/tinylangConfig.cmake.in
    tinylangConfig.cmake
    INSTALL_DESTINATION lib/cmake/tinylang
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/tinylangConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/tinylangConfigVersion.cmake
    DESTINATION lib/cmake/tinylang
)
