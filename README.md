# jparse

Simple recursive descent parser for JSON, [RFC-8259](https://datatracker.ietf.org/doc/html/rfc8259) compatible.
This is done for fun, specifically to get practice with C language (C11).

## Functionality

- Basic JSON parsing and value access (without hashmap)
- Working with arrays and objects (nested ones as well)
- File I/O operations

## Build, test, lint

```bash
just build  # Build library
just test   # Run test suite
just lint   # Run lint (requires clang-tidy)
just format # Run formatter (requires clang-format)
just clean  # Clean build artifacts
```

## Examples

```bash
just run-primitives  # Primitive types
just run-array       # Array structure
just run-nested      # Nested objects
just run-file-io     # File save/load
```

## TODOs

- [ ] Resolve existing TODOs in the code
- [ ] Consolidate to/from string API (right now we have from string, but no to-conversion, only printing to stdout)
- [ ] Add more tests, diagnose on memory leaks
- [ ] Add streaming parser for large JSONs
- [ ] Consider using hashmap
