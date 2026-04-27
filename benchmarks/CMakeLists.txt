add_executable(benchmark_monte_carlo
    benchmark_monte_carlo.cpp
)

target_link_libraries(benchmark_monte_carlo
    PRIVATE
        SeriesScopeLib
)

target_include_directories(benchmark_monte_carlo
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_compile_definitions(benchmark_monte_carlo
    PRIVATE
        SERIESSCOPE_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)
