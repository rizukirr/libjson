# libjson

A lightweight, header-only C JSON parser library. libjson provides a simple and efficient API for parsing and querying JSON data without external dependencies.

## Installation

libjson is a header-only library. Simply copy `libjson.h` to your project and include it in one source file with the implementation macro defined:

```c
#define LIBJSON_IMPLEMENTATION
#include "libjson.h"
```

In other files, include the header normally:

```c
#include "libjson.h"
```

## Example Usage

```c
#define LIBJSON_IMPLEMENTATION
#include "libjson.h"
#include <stdio.h>

int main() {
    char err[256] = {0};
    char *json_str = "{"
        "\"name\":\"John\","
        "\"age\":30,"
        "\"address\":{\"city\":\"New York\",\"zip\":10001},"
        "\"items\":[{\"id\":1},{\"id\":2}]"
    "}";

    // Initialize JSON parser
    JsonCtx *ctx = json_begin(json_str, err);
    if (!ctx) {
        fprintf(stderr, "Error: %s\n", err);
        return 1;
    }

    // Parse object properties
    char *name = json_get_string(ctx, "name", err);
    int *age = json_get_int(ctx, "age", err);

    printf("Name: %s\n", name);
    printf("Age: %d\n", *age);

    // Navigate to nested object using json_obj_key
    JsonCtx *address = json_obj_key(ctx, "address", err);
    if (address) {
        char *city = json_get_string(address, "city", err);
        int *zip = json_get_int(address, "zip", err);
        printf("City: %s\n", city);
        printf("ZIP: %d\n", *zip);
    }

    // Parse array
    char **items = json_array_key(ctx, "items", err);
    int count = json_array_count(ctx, err);

    printf("Items:\n");
    for (int i = 0; i < count; i++) {
        JsonCtx *item = json_obj(ctx, items[i], err);
        int *id = json_get_int(item, "id", err);
        printf("  - ID: %d\n", *id);
    }

    // Clean up
    json_end(ctx, err);
    return 0;
}
```

## API Reference

### Core Functions

#### `JsonCtx *json_begin(char *json, char *err)`
Create a new JSON object from a JSON string.

**Parameters:**
- `json` - JSON string to parse
- `err` - Error buffer (minimum 256 bytes recommended)

**Returns:** Pointer to JSON object, or NULL on error

---

#### `void json_end(JsonCtx *ctx, char *err)`
Free a JSON object and all associated memory.

**Parameters:**
- `ctx` - Pointer to JSON object
- `err` - Error buffer

---

#### `void *json_get(JsonCtx *ctx, char *key, char *err)`
Get a value from a JSON object by key. Returns a generic pointer that should be cast to the appropriate type.

**Parameters:**
- `ctx` - Pointer to JSON object
- `key` - Key to search for
- `err` - Error buffer

**Returns:** Pointer to value, or NULL if not found

---

#### `JsonCtx *json_obj(JsonCtx *ctx, char *json_obj, char *err)`
Update the JSON object's source to point to a new JSON string.

**Parameters:**
- `ctx` - Pointer to existing JSON object
- `json_obj` - New JSON string to parse
- `err` - Error buffer

**Returns:** The same JSON object with updated source, or NULL on error

---

#### `JsonCtx *json_obj_key(JsonCtx *ctx, char *key, char *err)`
Navigate to a nested JSON object by key.

**Parameters:**
- `ctx` - Pointer to JSON object
- `key` - Key of nested object
- `err` - Error buffer

**Returns:** JSON object pointing to nested object, or NULL if not found

---

#### `char **json_array(JsonCtx *ctx, char *source, char *err)`
Parse a JSON array and return array of string items.

**Parameters:**
- `ctx` - Pointer to JSON object
- `source` - JSON array string (must start with '[')
- `err` - Error buffer

**Returns:** Array of string pointers, or NULL on error

---

#### `char **json_array_key(JsonCtx *ctx, char *key, char *err)`
Parse a JSON array by key and return array of string items.

**Parameters:**
- `ctx` - Pointer to JSON object
- `key` - Key of the array to parse
- `err` - Error buffer

**Returns:** Array of string pointers, or NULL on error

---

#### `int json_array_count(JsonCtx *ctx, char *err)`
Get the count of items in a parsed JSON array.

**Parameters:**
- `ctx` - Pointer to JSON object containing parsed array
- `err` - Error buffer

**Returns:** Number of items, or -1 on error

---

### Type-Safe Helper Macros

These macros provide convenient type-safe access to JSON values:

- `json_get_int(ctx, key, err)` - Get integer value
- `json_get_string(ctx, key, err)` - Get string value
- `json_get_bool(ctx, key, err)` - Get boolean value
- `json_get_double(ctx, key, err)` - Get double value
- `json_get_float(ctx, key, err)` - Get float value

## Configuration

The following constants can be modified in the implementation section:

- `JSON_ERROR_BUFFER_SIZE` - Error message buffer size (default: 256)
- `JSON_DEFAULT_ARENA_SIZE` - Default arena block size (default: 4096)
- `JSON_MAX_INPUT_SIZE` - Maximum JSON input size (default: 10MB)
- `JSON_DEPTH_LIMIT` - Maximum nesting depth (default: 100)

## License

libjson is licensed under the Apache License 2.0. See the [LICENSE](LICENSE) file for details.

## Support

For bug reports, feature requests, or questions, please open an issue on the project repository.

## Contributing

Contributions are welcome. Please ensure code follows the existing style and includes appropriate tests and documentation.
