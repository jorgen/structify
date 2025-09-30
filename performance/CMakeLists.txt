include(FetchContent)

FetchContent_Declare(
        glaze
        GIT_REPOSITORY https://github.com/stephenberry/glaze.git
        GIT_TAG main
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glaze)
if(MSVC)
    add_compile_options(/arch:AVX2)
endif()

add_executable(benchmark
    benchmark.cpp
        include/simdjson/simdjson.cpp
    )
target_compile_definitions(benchmark PRIVATE CATCH_CONFIG_ENABLE_BENCHMARKING)
target_include_directories(benchmark PRIVATE ${PROJECT_SOURCE_DIR}/include)
if (MSVC)
  target_compile_options(benchmark PRIVATE /w)
else()
  target_compile_options(benchmark PRIVATE -w)
endif()
target_link_libraries(benchmark PRIVATE glaze::glaze)

add_library(catch_main_benchmark STATIC catch-main.cpp)
target_compile_definitions(catch_main_benchmark PRIVATE CATCH_CONFIG_ENABLE_BENCHMARKING)
target_link_libraries(benchmark PRIVATE catch_main_benchmark)
                
