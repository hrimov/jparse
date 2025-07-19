default: build

build:
    cmake -B build
    cmake --build build

build-tests:
    cmake -B build -DJPARSE_BUILD_TESTS=ON
    cmake --build build

build-examples:
    cmake -B build -DJPARSE_BUILD_EXAMPLES=ON
    cmake --build build

build-all:
    cmake -B build -DJPARSE_BUILD_TESTS=ON -DJPARSE_BUILD_EXAMPLES=ON
    cmake --build build

test:
    just build-tests
    ctest --test-dir build --output-on-failure

# TODO: add install dependencies commands for several OS

# TODO: add memleak-check
# memleak-check:
#     just build-tests
#     leaks --atExit -- ./build/test_jparse

lint:
    clang-tidy src/*.c include/*.h -- -Iinclude

format:
    find src/ include/ tests/ -name "*.c" -o -name "*.h" | xargs clang-format -i

clean:
    rm -rf build

run-primitives:
    just build-examples
    ./build/primitives_parsing

run-array:
    just build-examples
    ./build/array_example

run-nested:
    just build-examples
    ./build/nested_json

run-file-io:
    just build-examples
    ./build/file_io
