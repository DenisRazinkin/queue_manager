cmake_minimum_required(VERSION 3.15)
project ("qm_benchmarks")

# Add source to this project's executable.
add_executable (qm_benchmarks "bench1.cpp")

# Link Google Benchmark to the project
target_link_libraries(qm_benchmarks benchmark::benchmark)