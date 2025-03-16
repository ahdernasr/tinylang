# Benchmark configuration
add_executable(bench_driver bench_driver.cpp)
target_link_libraries(bench_driver tinylang)

# Benchmark programs
file(GLOB BENCHMARK_PROGRAMS "programs/*.tl")

# Create individual benchmark targets
foreach(program ${BENCHMARK_PROGRAMS})
    get_filename_component(program_name ${program} NAME_WE)
    add_custom_target(bench_${program_name}
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/bench_driver ${program}
        DEPENDS bench_driver
        COMMENT "Running benchmark: ${program_name}"
    )
endforeach()

# Run all benchmarks
add_custom_target(bench_all
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/bench_driver ${BENCHMARK_PROGRAMS}
    DEPENDS bench_driver
    COMMENT "Running all benchmarks"
)
