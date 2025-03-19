#!/bin/bash

# TinyLang Build Script
set -e

# Default values
BUILD_TYPE="Release"
BUILD_DIR="build"
CLEAN=false
TESTS=false
BENCHMARKS=false
VERBOSE=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -r|--release)
            BUILD_TYPE="Release"
            shift
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -t|--tests)
            TESTS=true
            shift
            ;;
        -b|--benchmarks)
            BENCHMARKS=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  -d, --debug      Build in Debug mode"
            echo "  -r, --release    Build in Release mode (default)"
            echo "  -c, --clean      Clean build directory before building"
            echo "  -t, --tests      Build and run tests"
            echo "  -b, --benchmarks Build and run benchmarks"
            echo "  -v, --verbose    Verbose output"
            echo "  -h, --help       Show this help"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Clean build directory if requested
if [ "$CLEAN" = true ]; then
    echo "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure CMake
echo "Configuring CMake..."
CMAKE_ARGS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"

if [ "$VERBOSE" = true ]; then
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_VERBOSE_MAKEFILE=ON"
fi

cmake .. $CMAKE_ARGS

# Build
echo "Building..."
if [ "$VERBOSE" = true ]; then
    make VERBOSE=1
else
    make -j$(nproc)
fi

# Run tests if requested
if [ "$TESTS" = true ]; then
    echo "Running tests..."
    ctest --output-on-failure
fi

# Run benchmarks if requested
if [ "$BENCHMARKS" = true ]; then
    echo "Running benchmarks..."
    make bench_all
fi

echo "Build completed successfully!"
echo "Executables:"
echo "  REPL: ./$BUILD_DIR/tl"
echo "  Compiler: ./$BUILD_DIR/tlc"
echo "  Disassembler: ./$BUILD_DIR/tldis"
