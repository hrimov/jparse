default: build

# Build only the library
build:
    cmake -B build
    cmake --build build

# Build library + tests
build-tests:
    cmake -B build -DJPARSE_BUILD_TESTS=ON
    cmake --build build

# Build library + examples
build-examples:
    cmake -B build -DJPARSE_BUILD_EXAMPLES=ON
    cmake --build build

# Build everything (library + tests + examples)
build-all:
    cmake -B build -DJPARSE_BUILD_TESTS=ON -DJPARSE_BUILD_EXAMPLES=ON
    cmake --build build

test:
    just build-tests
    ctest --test-dir build --output-on-failure

clean:
    rm -rf build

# Running examples
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
