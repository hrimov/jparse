cmake_minimum_required(VERSION 3.31)
project(jparse C)

set(CMAKE_C_STANDARD 11)

# Options to control what gets built
option(JPARSE_BUILD_TESTS "Build tests" OFF)
option(JPARSE_BUILD_EXAMPLES "Build examples" OFF)

# Main library (always built)
add_library(jparse STATIC src/jparse.c)
target_include_directories(jparse PUBLIC include)

### Testing (optional)
if(JPARSE_BUILD_TESTS)
    enable_testing()
    
    add_executable(test_jparse tests/test_jparse.c)
    target_include_directories(test_jparse PRIVATE include)
    target_link_libraries(test_jparse jparse)
    
    add_test(NAME jparse_tests COMMAND test_jparse)
endif()

### Examples (optional)
if(JPARSE_BUILD_EXAMPLES)
    add_executable(primitives_parsing examples/primitives_parsing.c)
    target_include_directories(primitives_parsing PRIVATE include)
    target_link_libraries(primitives_parsing jparse)
    
    add_executable(array_example examples/array_example.c)
    target_include_directories(array_example PRIVATE include)
    target_link_libraries(array_example jparse)
    
    add_executable(file_io examples/file_io.c)
    target_include_directories(file_io PRIVATE include)
    target_link_libraries(file_io jparse)
    
    add_executable(nested_json examples/nested_json.c)
    target_include_directories(nested_json PRIVATE include)
    target_link_libraries(nested_json jparse)
endif()
