# libjson

`libjson` is a small single-header JSON helper for C that is optimized for
read-only, zero-copy access.

It is aimed at embedded-style workloads where:
- the input buffer already exists
- heap allocation should be avoided in the hot path
- the caller can consume values as slices instead of owned strings

## Design

The library does not build a DOM and does not allocate while parsing.

Instead it returns:

```c
typedef struct {
  const char *data;
  size_t len;
} JsonSlice;
```

String values are returned without the surrounding quotes. Object and array
values are returned as slices that still include their `{...}` or `[...]`
delimiters. Number, boolean, and `null` values are returned as raw tokens.

## API

```c
JsonSlice json_from_cstr(const char *json);
JsonSlice json_from_parts(const char *data, size_t len);

bool json_get(JsonSlice object, const char *key, JsonSlice *out);
bool json_getn(JsonSlice object, const char *key, size_t key_len, JsonSlice *out);

void json_object_iter_init(JsonSlice object, JsonObjectIter *iter);
bool json_object_iter_next(JsonObjectIter *iter, JsonSlice *key, JsonSlice *value);

void json_array_iter_init(JsonSlice array, JsonArrayIter *iter);
bool json_array_iter_next(JsonArrayIter *iter, JsonSlice *out);

bool json_slice_copy(JsonSlice slice, char *buffer, size_t buffer_size);
```

## Example

```c
#include "libjson.h"
#include <stdio.h>

int main(void) {
  const char *json =
      "{\"meta\":{\"name\":\"sensor\"},\"values\":[1,2,3]}";

  JsonSlice root = json_from_cstr(json);
  JsonSlice meta;
  JsonSlice name;
  JsonSlice values;
  JsonArrayIter iter;
  JsonSlice item;

  if (!json_get(root, "meta", &meta) ||
      !json_get(meta, "name", &name) ||
      !json_get(root, "values", &values)) {
    return 1;
  }

  printf("name=%.*s\n", (int)name.len, name.data);

  json_array_iter_init(values, &iter);
  while (json_array_iter_next(&iter, &item)) {
    printf("value=%.*s\n", (int)item.len, item.data);
  }

  return 0;
}
```

If you already know the key length, prefer `json_getn()` to avoid the extra
`strlen()` in repeated lookup paths.

## Notes

- Input is read-only. Pass `const char *` data.
- Returned slices point into the original JSON buffer.
- Keep the source buffer alive while you use returned slices.
- For single-pass object traversal, use `json_object_iter_*()` instead of
  calling `json_get()` repeatedly on the same object.
- `json_slice_copy()` is available when you need a null-terminated copy in a
  caller-owned buffer.
- Define `JSON_NO_STDIO` before including `libjson.h` to compile out diagnostic
  printing for embedded builds.
- The parser is intentionally small. It is designed for selective access, not
  full JSON validation or schema-aware parsing.

## Build

```bash
clang -Wall -Wextra -pedantic example.c -o example
```
