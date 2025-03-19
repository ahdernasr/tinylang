#!/bin/bash

# TinyLang Run Script
set -e

# Default values
BUILD_DIR="build"
PROGRAM=""
ARGS=""

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -b|--build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS] [PROGRAM] [ARGS...]"
            echo "Options:"
            echo "  -b, --build-dir DIR  Build directory (default: build)"
            echo "  -h, --help           Show this help"
            echo ""
            echo "Programs:"
            echo "  repl                 Run the REPL"
            echo "  compile FILE         Compile a .tl file"
            echo "  disasm FILE          Disassemble a .tbc file"
            echo "  run FILE             Run a .tl file"
            echo ""
            echo "Examples:"
            echo "  $0 repl"
            echo "  $0 compile examples/fib.tl"
            echo "  $0 run examples/fizzbuzz.tl"
            exit 0
            ;;
        *)
            if [ -z "$PROGRAM" ]; then
                PROGRAM="$1"
            else
                ARGS="$ARGS $1"
            fi
            shift
            ;;
    esac
done

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Build directory '$BUILD_DIR' not found"
    echo "Run './scripts/build.sh' first"
    exit 1
fi

# Check if executables exist
if [ ! -f "$BUILD_DIR/tl" ] || [ ! -f "$BUILD_DIR/tlc" ] || [ ! -f "$BUILD_DIR/tldis" ]; then
    echo "Error: Executables not found in '$BUILD_DIR'"
    echo "Run './scripts/build.sh' first"
    exit 1
fi

# Run the requested program
case "$PROGRAM" in
    "repl"|"")
        echo "Starting TinyLang REPL..."
        "$BUILD_DIR/tl"
        ;;
    "compile")
        if [ -z "$ARGS" ]; then
            echo "Error: No file specified for compilation"
            exit 1
        fi
        echo "Compiling $ARGS..."
        "$BUILD_DIR/tlc" $ARGS
        ;;
    "disasm")
        if [ -z "$ARGS" ]; then
            echo "Error: No file specified for disassembly"
            exit 1
        fi
        echo "Disassembling $ARGS..."
        "$BUILD_DIR/tldis" $ARGS
        ;;
    "run")
        if [ -z "$ARGS" ]; then
            echo "Error: No file specified for execution"
            exit 1
        fi
        echo "Running $ARGS..."
        "$BUILD_DIR/tl" < "$ARGS"
        ;;
    *)
        echo "Error: Unknown program '$PROGRAM'"
        echo "Run '$0 --help' for usage information"
        exit 1
        ;;
esac
