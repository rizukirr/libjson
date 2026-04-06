/*
 * Copyright (c) 2025 Rizki Rakasiwi <rizkirr.xyz@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * libjson — zero-copy JSON parser (STB-style single-header library)
 *
 * In exactly ONE C file, do:
 *   #define LIBJSON_IMPLEMENTATION
 *   #include "libjson.h"
 *
 * All other files just #include "libjson.h" for declarations only.
 */

#ifndef LIBJSON_H
#define LIBJSON_H

#include <stdbool.h>
#include <stddef.h>
#ifndef JSON_NO_STDIO
#include <stdio.h>
#endif
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  const char *data;
  size_t len;
} JsonSlice;

typedef struct {
  JsonSlice array;
  const char *cursor;
} JsonArrayIter;

typedef struct {
  JsonSlice object;
  const char *cursor;
} JsonObjectIter;

/*
 * Build a slice from an existing pointer and length.
 *
 * The slice does not own the memory. The caller must keep `data` alive for as
 * long as the slice is used.
 */
JsonSlice json_from_parts(const char *data, size_t len);

/*
 * Build a slice from a NUL-terminated JSON string.
 *
 * This is a convenience wrapper around `json_from_parts()`.
 */
JsonSlice json_from_cstr(const char *json);

/*
 * Copy a slice into a caller-provided buffer and append a trailing NUL byte.
 *
 * Returns false if `buffer` is NULL, the slice is invalid, or the buffer is
 * too small. This helper is optional and exists for callers that need a
 * conventional C string without using heap allocation.
 */
bool json_slice_copy(JsonSlice slice, char *buffer, size_t buffer_size);

/*
 * Look up `key` with an explicit key length.
 *
 * This avoids recomputing `strlen(key)` in hot lookup paths.
 */
bool json_getn(JsonSlice object, const char *key, size_t key_len,
               JsonSlice *out);

/*
 * Look up `key` in a JSON object slice and return the matched value as a
 * zero-copy slice.
 *
 * String results exclude the surrounding quotes. Object and array results keep
 * their delimiters. Number, boolean, and null results are returned as raw
 * tokens. Returns false if the key is missing or the input is not a valid
 * object-shaped slice.
 */
bool json_get(JsonSlice object, const char *key, JsonSlice *out);

/*
 * Initialize an iterator for a JSON object slice.
 *
 * The iterator walks key/value pairs in-place over the original input buffer.
 */
void json_object_iter_init(JsonSlice object, JsonObjectIter *iter);

/*
 * Return the next object key/value pair as zero-copy slices.
 *
 * Key slices exclude surrounding quotes. Returns true while entries remain.
 * Returns false when the object is exhausted or when invalid input is
 * encountered.
 */
bool json_object_iter_next(JsonObjectIter *iter, JsonSlice *key,
                           JsonSlice *value);

/*
 * Initialize an iterator for a JSON array slice.
 *
 * The iterator does not allocate and walks the array in-place over the
 * original input buffer.
 */
void json_array_iter_init(JsonSlice array, JsonArrayIter *iter);

/*
 * Return the next array item as a zero-copy slice.
 *
 * Returns true while elements remain. Returns false when the array is
 * exhausted or when invalid input is encountered.
 */
bool json_array_iter_next(JsonArrayIter *iter, JsonSlice *out);

#ifdef __cplusplus
}
#endif

#endif /* LIBJSON_H */

/* ========================================================================= */
/*                            IMPLEMENTATION                                 */
/* ========================================================================= */

#ifdef LIBJSON_IMPLEMENTATION

#ifdef __cplusplus
extern "C" {
#endif

#ifdef JSON_NO_STDIO
static void json__error(const char *message) { (void)message; }
#else
static void json__error(const char *message) {
  fprintf(stderr, "%s\n", message);
}
#endif

static const char *json__skip_whitespace(const char *cursor, const char *end) {
  while (cursor < end) {
    unsigned char ch = (unsigned char)*cursor;
    if (ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r')
      break;
    cursor++;
  }
  return cursor;
}

static const char *json__skip_string(const char *cursor, const char *end) {
  if (cursor >= end || *cursor != '"') {
    json__error("json: expected string");
    return NULL;
  }

  cursor++;
  while (cursor < end) {
    size_t remaining = (size_t)(end - cursor);
    const char *next_quote = (const char *)memchr(cursor, '"', remaining);
    const char *next_escape = (const char *)memchr(cursor, '\\', remaining);
    const char *next = next_quote;

    if (next_escape && (!next || next_escape < next))
      next = next_escape;

    if (!next)
      break;

    if (*next == '"')
      return next + 1;

    cursor = next + 1;
    if (cursor >= end) {
      json__error("json: unterminated escape");
      return NULL;
    }
    cursor++;
  }

  json__error("json: unterminated string");
  return NULL;
}

static const char *json__skip_compound(const char *cursor, const char *end,
                                       char open_ch, char close_ch) {
  int depth = 1;
  bool in_string = false;
  bool escaped = false;

  cursor++;
  while (cursor < end) {
    char ch = *cursor;

    if (in_string) {
      if (escaped) {
        escaped = false;
      } else if (ch == '\\') {
        escaped = true;
      } else if (ch == '"') {
        in_string = false;
      }
      cursor++;
      continue;
    }

    if (ch == '"') {
      in_string = true;
    } else if (ch == open_ch) {
      depth++;
    } else if (ch == close_ch) {
      depth--;
      if (depth == 0)
        return cursor + 1;
    }

    cursor++;
  }

  json__error("json: unterminated compound value");
  return NULL;
}

static const char *json__skip_primitive(const char *cursor, const char *end) {
  while (cursor < end) {
    char ch = *cursor;
    if (ch == ',' || ch == ']' || ch == '}' || ch == ' ' || ch == '\t' ||
        ch == '\n' || ch == '\r') {
      return cursor;
    }
    cursor++;
  }
  return cursor;
}

static const char *json__skip_value(const char *cursor, const char *end) {
  cursor = json__skip_whitespace(cursor, end);
  if (cursor >= end) {
    json__error("json: expected value");
    return NULL;
  }

  if (*cursor == '"')
    return json__skip_string(cursor, end);
  if (*cursor == '{')
    return json__skip_compound(cursor, end, '{', '}');
  if (*cursor == '[')
    return json__skip_compound(cursor, end, '[', ']');
  return json__skip_primitive(cursor, end);
}

static JsonSlice json__value_slice(const char *start, const char *end) {
  if (!start || !end || end < start)
    return json_from_parts(NULL, 0);

  if (*start == '"' && end > start + 1 && *(end - 1) == '"')
    return json_from_parts(start + 1, (size_t)((end - start) - 2));

  return json_from_parts(start, (size_t)(end - start));
}

static bool json__object_next(const char **cursor_ptr, const char *end,
                              JsonSlice *key, JsonSlice *value) {
  const char *cursor = *cursor_ptr;
  const char *key_start;
  const char *key_end;
  const char *value_start;
  const char *value_end;

  cursor = json__skip_whitespace(cursor, end);
  if (cursor >= end) {
    json__error("json: unterminated object");
    return false;
  }

  if (*cursor == '}') {
    *cursor_ptr = cursor;
    return false;
  }
  if (*cursor == ',') {
    cursor++;
    cursor = json__skip_whitespace(cursor, end);
  }

  if (cursor >= end) {
    json__error("json: unterminated object");
    return false;
  }
  if (*cursor == '}') {
    *cursor_ptr = cursor;
    return false;
  }
  if (*cursor != '"') {
    json__error("json: expected object key");
    return false;
  }

  key_start = cursor + 1;
  key_end = json__skip_string(cursor, end);
  if (!key_end)
    return false;

  cursor = json__skip_whitespace(key_end, end);
  if (cursor >= end || *cursor != ':') {
    json__error("json: expected ':' after key");
    return false;
  }

  cursor++;
  value_start = json__skip_whitespace(cursor, end);
  value_end = json__skip_value(value_start, end);
  if (!value_end)
    return false;

  if (key)
    *key = json_from_parts(key_start, (size_t)((key_end - 1) - key_start));
  if (value)
    *value = json__value_slice(value_start, value_end);

  cursor = json__skip_whitespace(value_end, end);
  if (cursor < end && *cursor == ',')
    cursor++;
  *cursor_ptr = cursor;
  return true;
}

JsonSlice json_from_parts(const char *data, size_t len) {
  JsonSlice slice;
  slice.data = data;
  slice.len = len;
  return slice;
}

JsonSlice json_from_cstr(const char *json) {
  if (!json)
    return json_from_parts(NULL, 0);
  return json_from_parts(json, strlen(json));
}

bool json_slice_copy(JsonSlice slice, char *buffer, size_t buffer_size) {
  if (!buffer) {
    json__error("json_slice_copy: buffer is NULL");
    return false;
  }

  if (!slice.data && slice.len != 0) {
    json__error("json_slice_copy: invalid slice");
    return false;
  }

  if (buffer_size <= slice.len) {
    json__error("json_slice_copy: buffer too small");
    return false;
  }

  if (slice.len > 0)
    memcpy(buffer, slice.data, slice.len);
  buffer[slice.len] = '\0';
  return true;
}

bool json_getn(JsonSlice object, const char *key, size_t key_len,
               JsonSlice *out) {
  const char *cursor;
  const char *end;
  JsonSlice current_key;
  JsonSlice current_value;

  if (!out || !key) {
    json__error("json_getn: invalid arguments");
    return false;
  }

  *out = json_from_parts(NULL, 0);
  if (!object.data) {
    json__error("json_getn: object slice is NULL");
    return false;
  }

  cursor = json__skip_whitespace(object.data, object.data + object.len);
  end = object.data + object.len;

  if (cursor >= end || *cursor != '{') {
    json__error("json_getn: input is not an object");
    return false;
  }

  cursor++;
  while (cursor < end) {
    if (!json__object_next(&cursor, end, &current_key, &current_value))
      return false;

    if (current_key.len == key_len &&
        strncmp(current_key.data, key, key_len) == 0) {
      *out = current_value;
      return true;
    }
  }

  json__error("json_getn: unterminated object");
  return false;
}

bool json_get(JsonSlice object, const char *key, JsonSlice *out) {
  if (!key) {
    json__error("json_get: invalid arguments");
    return false;
  }
  return json_getn(object, key, strlen(key), out);
}

void json_object_iter_init(JsonSlice object, JsonObjectIter *iter) {
  if (!iter) {
    json__error("json_object_iter_init: iter is NULL");
    return;
  }

  iter->object = object;
  iter->cursor = object.data;
}

bool json_object_iter_next(JsonObjectIter *iter, JsonSlice *key,
                           JsonSlice *value) {
  const char *cursor;
  const char *end;

  if (!iter || !key || !value) {
    json__error("json_object_iter_next: invalid arguments");
    return false;
  }

  *key = json_from_parts(NULL, 0);
  *value = json_from_parts(NULL, 0);
  if (!iter->object.data) {
    json__error("json_object_iter_next: object slice is NULL");
    return false;
  }

  end = iter->object.data + iter->object.len;
  cursor = iter->cursor ? iter->cursor : iter->object.data;
  cursor = json__skip_whitespace(cursor, end);

  if (cursor == iter->object.data) {
    if (cursor >= end || *cursor != '{') {
      json__error("json_object_iter_next: input is not an object");
      return false;
    }
    cursor++;
  }

  if (!json__object_next(&cursor, end, key, value)) {
    iter->cursor = cursor;
    return false;
  }

  iter->cursor = cursor;
  return true;
}

void json_array_iter_init(JsonSlice array, JsonArrayIter *iter) {
  if (!iter) {
    json__error("json_array_iter_init: iter is NULL");
    return;
  }

  iter->array = array;
  iter->cursor = array.data;
}

bool json_array_iter_next(JsonArrayIter *iter, JsonSlice *out) {
  const char *cursor;
  const char *end;
  const char *value_start;
  const char *value_end;

  if (!iter || !out) {
    json__error("json_array_iter_next: invalid arguments");
    return false;
  }

  *out = json_from_parts(NULL, 0);
  if (!iter->array.data) {
    json__error("json_array_iter_next: array slice is NULL");
    return false;
  }

  end = iter->array.data + iter->array.len;
  cursor = iter->cursor ? iter->cursor : iter->array.data;
  cursor = json__skip_whitespace(cursor, end);

  if (cursor == iter->array.data) {
    if (cursor >= end || *cursor != '[') {
      json__error("json_array_iter_next: input is not an array");
      return false;
    }
    cursor++;
  }
  cursor = json__skip_whitespace(cursor, end);

  if (cursor < end && *cursor == ',') {
    cursor++;
    cursor = json__skip_whitespace(cursor, end);
  }

  if (cursor >= end) {
    json__error("json_array_iter_next: unterminated array");
    return false;
  }

  if (*cursor == ']') {
    iter->cursor = cursor;
    return false;
  }

  value_start = cursor;
  value_end = json__skip_value(value_start, end);
  if (!value_end)
    return false;

  *out = json__value_slice(value_start, value_end);
  cursor = json__skip_whitespace(value_end, end);
  if (cursor < end && *cursor == ',')
    cursor++;
  iter->cursor = cursor;
  return true;
}

#ifdef __cplusplus
}
#endif

#endif /* LIBJSON_IMPLEMENTATION */
