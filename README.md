# libjson

`libjson` is a small single-header JSON library for C that provides zero-copy
reading and zero-allocation writing.

It is aimed at embedded-style workloads where:
- the input buffer already exists
- heap allocation should be avoided
- the caller can consume values as slices instead of owned strings
- JSON output must be written into a fixed-size buffer

## Design

The library does not build a DOM and does not allocate.

**Reader** — returns zero-copy slices into the original JSON buffer:

```c
typedef struct {
  const char *data;
  size_t len;
} JsonSlice;
```

String values are returned without the surrounding quotes. Object and array
values are returned as slices that still include their `{...}` or `[...]`
delimiters. Number, boolean, and `null` values are returned as raw tokens.

**Writer** — serializes JSON into a caller-provided buffer with automatic comma
insertion and overflow detection:

```c
typedef struct {
  char *buf;
  size_t cap;
  size_t pos;
  bool overflow;
  unsigned int depth;
  unsigned int needs_comma;
} JsonWriter;
```

Comma tracking uses a bitmask, supporting up to 32 levels of nesting with zero
extra RAM.

## Reader API

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

## Writer API

```c
void          json_writer_init(JsonWriter *w, char *buf, size_t cap);
size_t        json_writer_len(const JsonWriter *w);
bool          json_writer_ok(const JsonWriter *w);
const char   *json_writer_output(const JsonWriter *w);

void json_write_null(JsonWriter *w);
void json_write_bool(JsonWriter *w, bool value);
void json_write_int(JsonWriter *w, long value);
void json_write_uint(JsonWriter *w, unsigned long value);
void json_write_str(JsonWriter *w, const char *str);
void json_write_strn(JsonWriter *w, const char *str, size_t len);
void json_write_raw(JsonWriter *w, const char *raw, size_t len);

void json_write_object_begin(JsonWriter *w);
void json_write_object_end(JsonWriter *w);
void json_write_key(JsonWriter *w, const char *key);

void json_write_array_begin(JsonWriter *w);
void json_write_array_end(JsonWriter *w);
```

## Examples

### Reading JSON

```c
#include "libjson.h"
#include <stdio.h>

int main(void) {
  const char *json =
      "{\"meta\":{\"name\":\"sensor\"},\"values\":[1,2,3]}";

  JsonSlice root = json_from_cstr(json);
  JsonSlice meta, name, values;
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

### Writing JSON

```c
#include "libjson.h"
#include <stdio.h>

int main(void) {
  char buf[256];
  JsonWriter w;
  json_writer_init(&w, buf, sizeof(buf));

  json_write_object_begin(&w);
    json_write_key(&w, "name");
    json_write_str(&w, "sensor-01");

    json_write_key(&w, "temp");
    json_write_int(&w, 2350);

    json_write_key(&w, "readings");
    json_write_array_begin(&w);
      json_write_int(&w, 100);
      json_write_int(&w, 200);
      json_write_int(&w, 300);
    json_write_array_end(&w);

    json_write_key(&w, "active");
    json_write_bool(&w, true);
  json_write_object_end(&w);

  if (json_writer_ok(&w)) {
    printf("%s\n", json_writer_output(&w));
  }

  return 0;
}
```

If you already know the key length, prefer `json_getn()` to avoid the extra
`strlen()` in repeated lookup paths.

## Notes

- **Reader**: input is read-only. Returned slices point into the original JSON
  buffer — keep the source buffer alive while you use them.
- **Writer**: writes into a caller-provided buffer. If the buffer overflows,
  all subsequent writes become silent no-ops. Check once at the end with
  `json_writer_ok()`.
- For single-pass object traversal, use `json_object_iter_*()` instead of
  calling `json_get()` repeatedly on the same object.
- `json_slice_copy()` is available when you need a null-terminated copy in a
  caller-owned buffer.
- `json_write_raw()` is available for pre-formatted numbers (e.g. fixed-point)
  or embedding pre-built JSON fragments.
- Passing `NULL` to `json_write_str()` emits `null`.
- Define `JSON_NO_STDIO` before including `libjson.h` to compile out diagnostic
  printing for embedded builds.
- The library is intentionally small. It is designed for selective access and
  compact serialization, not full JSON validation or schema-aware parsing.

## Build

```bash
gcc -Wall -Wextra -pedantic examples/example_deserialize.c -o example_deserialize
gcc -Wall -Wextra -pedantic examples/example_serialize.c -o example_serialize
gcc -Wall -Wextra -pedantic test_reader.c -o test_reader && ./test_reader
gcc -Wall -Wextra -pedantic test_writer.c -o test_writer && ./test_writer
```
