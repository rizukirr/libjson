# libjson

A lightweight, single-header JSON parser for C with arena-based memory management.

## Introduction

`libjson` is a minimalist JSON parsing library designed for C projects that need simple, efficient JSON parsing without heavy dependencies. It uses arena allocation for fast memory management and provides a straightforward API for extracting values from JSON documents.

**Key Features:**
- Single-header library - just include and use
- Arena-based memory allocation for performance
- No external dependencies (only standard C library)
- Simple, intuitive API
- Support for nested objects and arrays
- C89/C99 compatible

## How It Works

### Memory Management

The library uses an arena allocator (also known as a region-based allocator) to manage memory efficiently:

1. **Arena Structure**: Memory is allocated in fixed-size blocks (regions) of 4KB by default
2. **Alignment**: Proper memory alignment is ensured for all allocations using `alignof` (C11) or offset-based calculation for older standards
3. **No Individual Frees**: All memory is freed at once when `json_end()` is called, eliminating memory leaks and fragmentation
4. **Dynamic Growth**: When a region fills up, a new region is automatically allocated and linked

### Parsing Strategy

The parser uses a non-destructive, lazy parsing approach:

1. **Key Lookup**: `json_find_key()` traverses the JSON string to locate keys at the appropriate depth level
2. **Bracket Matching**: `find_matching_bracket()` uses a depth counter to find matching `{}` or `[]` brackets, accounting for nested structures and strings
3. **Value Extraction**: `json_extract_value()` extracts the value after a key, handling strings (with escape sequences), objects, arrays, numbers, booleans, and null
4. **Array Parsing**: Arrays are parsed into individual JSON objects/values stored in a dynamic array

**Technical Details:**
- Maximum nesting depth: 100 levels (configurable via `JSON_DEPTH_LIMIT`)
- String escape sequences are properly handled
- Whitespace is automatically skipped
- The original JSON string must remain valid during parsing

## How to Use

### Basic Setup

1. **Include the header in ONE C file with the implementation:**

```c
#define LIBJSON_IMPLEMENTATION
#include "libjson.h"
```

2. **In other files, just include the header:**

```c
#include "libjson.h"
```

### API Reference

#### Initialize and Cleanup

```c
JsonContext *json_begin();
void json_end(JsonContext *ctx);
```

- `json_begin()`: Creates a new JSON parsing context with arena allocator
- `json_end()`: Frees all memory associated with the context

#### Extract Values

```c
void *get_value(JsonContext *ctx, const char *key, char *raw_json);
```

Extracts a value for the given key from a JSON object. Returns a null-terminated string containing the value.

#### Extract Arrays

```c
void **get_array(JsonContext *ctx, const char *key, char *raw_json);
size_t json_array_count(JsonContext *ctx);
```

- `get_array()`: Parses a JSON array and returns an array of JSON objects/values
- `json_array_count()`: Returns the number of elements extracted from the last array operation

### Example

See `example.c` for a complete usage example demonstrating:
- Loading JSON from a file
- Extracting simple key-value pairs
- Working with nested objects
- Iterating over arrays

### Compilation

Compile with any C compiler:

```bash
gcc -o example example.c
clang -o example example.c
```

For older C standards (pre-C11):
```bash
gcc -std=c99 -o example example.c
```

## License

This project is under apache license 2.0. See `LICENSE` for details.

## Support

### Reporting Issues

If you encounter bugs or have feature requests, please:
1. Check existing issues first
2. Provide a minimal example demonstrating the problem
3. Include your compiler version and platform information

### Contributing

Contributions are welcome! Please ensure:
- Code follows the existing style
- Changes don't break existing functionality
- Memory safety is maintained

### Questions

For questions about usage or implementation details, feel free to open a discussion or issue in the repository.

---

**Note**: This library prioritizes simplicity and minimal dependencies. For production use with complex JSON schemas or strict validation requirements, consider more feature-complete libraries like cJSON or json-c.
