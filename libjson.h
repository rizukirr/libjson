/**
 * @file libjson.h
 * @brief A lightweight, header-only JSON parser for C
 * @author rizki rakasiwi <rizkirr.xyz@gmail.com>
 * @copyright Copyright 2025 rizki rakasiwi
 * @license Apache License 2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @section DESCRIPTION
 *
 * libjson is a single-header JSON parsing library for C that provides:
 * - Simple and efficient API for parsing JSON strings
 * - Arena-based memory management for fast allocation
 * - Support for nested objects and arrays
 * - Type-safe helper macros for common data types
 * - Depth-limited parsing to prevent stack overflow
 * - Zero external dependencies
 *
 * @section USAGE
 *
 * To use this library, include this header in one source file with the
 * implementation macro defined:
 *
 * @code
 * #define LIBJSON_IMPLEMENTATION
 * #include "libjson.h"
 * @endcode
 *
 * In other files, simply include the header:
 *
 * @code
 * #include "libjson.h"
 * @endcode
 *
 * @section EXAMPLE
 *
 * Basic usage example:
 *
 * @code
 * char err[256] = {0};
 * JsonCtx *json = json_begin("{\"name\":\"John\",\"age\":30}", err);
 * if (!json) {
 *     fprintf(stderr, "Error: %s\n", err);
 *     return 1;
 * }
 *
 * char *name = json_get_string(json, "name", err);
 * int *age = json_get_int(json, "age", err);
 *
 * printf("Name: %s, Age: %d\n", name, *age);
 * json_end(json, err);
 * @endcode
 */

#ifndef LIBJSON_H
#define LIBJSON_H

#ifdef __cplusplus
extern "C" {
#endif

// Configuration constants
#define JSON_ERROR_BUFFER_SIZE 256
#define JSON_DEFAULT_ARENA_SIZE 4096
#define JSON_MAX_INPUT_SIZE (10 * 1024 * 1024)
#define JSON_DEPTH_LIMIT 100

/**
 * @defgroup TypeMacros Type-Safe Helper Macros
 * @brief Convenience macros for retrieving typed values from JSON objects
 *
 * These macros wrap json_get() and cast the result to the appropriate type,
 * providing type-safe retrieval of primitive values from JSON objects.
 *
 * @note All macros return a pointer to the value, which may be NULL if the
 * key is not found or an error occurs. Always check the error buffer.
 *
 * @{
 */

/**
 * @brief Retrieve an integer value from a JSON object
 * @param ctx Pointer to the JSON object
 * @param key The key to search for
 * @param err Error buffer for error messages
 * @return Pointer to int value, or NULL on error
 */
#define json_get_int(ctx, key, err) ((int *)json_get(ctx, key, err))

/**
 * @brief Retrieve a string value from a JSON object
 * @param ctx Pointer to the JSON object
 * @param key The key to search for
 * @param err Error buffer for error messages
 * @return Pointer to string value, or NULL on error
 */
#define json_get_string(ctx, key, err) ((char *)json_get(ctx, key, err))

/**
 * @brief Retrieve a boolean value from a JSON object
 * @param ctx Pointer to the JSON object
 * @param key The key to search for
 * @param err Error buffer for error messages
 * @return Pointer to int value (0 or 1), or NULL on error
 */
#define json_get_bool(ctx, key, err) ((int *)json_get(ctx, key, err))

/**
 * @brief Retrieve a double value from a JSON object
 * @param ctx Pointer to the JSON object
 * @param key The key to search for
 * @param err Error buffer for error messages
 * @return Pointer to double value, or NULL on error
 */
#define json_get_double(ctx, key, err) ((double *)json_get(ctx, key, err))

/**
 * @brief Retrieve a float value from a JSON object
 * @param ctx Pointer to the JSON object
 * @param key The key to search for
 * @param err Error buffer for error messages
 * @return Pointer to float value, or NULL on error
 */
#define json_get_float(ctx, key, err) ((float *)json_get(ctx, key, err))

/** @} */

/**
 * @brief Opaque JSON object handle
 *
 * This structure represents a parsed JSON object and maintains internal state
 * for parsing operations. Users should not access struct members directly;
 * use the provided API functions instead.
 */
typedef struct JsonCtx JsonCtx;

/**
 * @brief Create a new JSON parser object from a JSON string
 *
 * This function initializes a new JSON parser and validates the input JSON
 * string. The JSON string must be well-formed and start with '{' or '['.
 * An arena allocator is created internally for efficient memory management.
 *
 * @param json JSON string to parse (must be null-terminated)
 * @param err  Error buffer for error messages (minimum 256 bytes)
 * @return Pointer to a new JSON object, or NULL if parsing fails
 *
 * @note The JSON string must remain valid for the lifetime of the JSON object,
 * as the parser does not copy the input string.
 *
 * @warning Input size is limited to JSON_MAX_INPUT_SIZE (default: 10MB)
 *
 * @see json_end() to free the JSON object when done
 *
 * Example:
 * @code
 * char err[256] = {0};
 * JsonCtx *json = json_begin("{\"key\":\"value\"}", err);
 * if (!json) {
 *     fprintf(stderr, "Parse error: %s\n", err);
 * }
 * @endcode
 */
JsonCtx *json_begin(char *json, char *err);

/**
 * @brief Parse a JSON array from a string
 *
 * This function parses a JSON array string and returns an array of string
 * pointers, where each element represents an item in the array. Currently
 * optimized for arrays of objects.
 *
 * @param ctx       Pointer to a JSON Context object
 * @param json_array JSON array string (must start with '[')
 * @param err        Error buffer for error messages
 * @return Array of string pointers, or NULL on error
 *
 * @note After calling this function, use json_array_count() to get the
 * number of items in the array.
 *
 * @see json_array_key() to parse an array by key
 * @see json_array_count() to get the item count
 */
char **json_array(JsonCtx *ctx, char *json_array, char *err);

/**
 * @brief Parse a JSON array by key
 *
 * This is a convenience function that retrieves an array value by key and
 * parses it in one step. Equivalent to calling json_get() followed by
 * json_array().
 *
 * @param ctx Pointer to a JSON object
 * @param key  Key of the array to retrieve
 * @param err  Error buffer for error messages
 * @return Array of string pointers, or NULL if key not found or error
 *
 * @see json_array() for parsing an array from a string directly
 * @see json_array_count() to get the item count
 */
char **json_array_key(JsonCtx *ctx, char *key, char *err);

/**
 * @brief Update the JSON object's source to point to a new JSON string
 *
 * This function updates the internal source pointer of an existing JSON object
 * to parse a different JSON string. This is useful for reusing a JSON object
 * to navigate through nested JSON structures without creating new objects.
 * The original arena allocator is preserved, allowing all allocations to be
 * freed with a single json_end() call.
 *
 * @param ctx     Pointer to an existing JSON object
 * @param json_obj New JSON string to parse (can be a nested object or array)
 * @param err      Error buffer for error messages
 * @return The same JSON object with updated source, or NULL on error
 *
 * @note This does not validate the new JSON string; use with strings obtained
 * from json_get() or similar functions.
 *
 * Example:
 * @code
 * JsonCtx *json = json_begin("{\"user\":{\"name\":\"John\"}}", err);
 * char *user_obj = json_get_string("user", json, err);
 * json_obj(json, user_obj, err);
 * char *name = json_get_string("name", json, err);
 * @endcode
 */
JsonCtx *json_obj(JsonCtx *ctx, char *json_obj, char *err);

/**
 * @brief Navigate to a nested JSON object by key
 *
 * This function retrieves a nested JSON object by key and updates the JSON
 * object's source to point to that nested object. This is a convenience
 * function that combines json_get() and json_obj() to simplify navigation
 * through deeply nested JSON structures.
 *
 * @param ctx Pointer to a JSON object
 * @param key  Key of the nested object to navigate to
 * @param err  Error buffer for error messages
 * @return The same JSON object with source updated to the nested object,
 *         or NULL if key not found or invalid
 *
 * Example:
 * @code
 * JsonCtx *json = json_begin("{\"user\":{\"address\":{\"city\":\"NYC\"}}}",
 * err); json_obj_key(json, "user", err); json_obj_key(json, "address", err);
 * char *city = json_get_string("city", json, err);  // "NYC"
 * @endcode
 *
 * @see json_obj() for manual source updates
 */
JsonCtx *json_obj_key(JsonCtx *ctx, char *key, char *err);

/**
 * @brief Retrieve a value from a JSON object by key
 *
 * This function searches for a key in the current JSON object and returns
 * a pointer to its associated value. The value is extracted and allocated
 * in the arena, returned as a null-terminated string. For primitive types,
 * use the type-safe helper macros instead of this function directly.
 *
 * @param ctx Pointer to a JSON object
 * @param key  Key to search for (null-terminated string)
 * @param err  Error buffer for error messages (receives error if key not found)
 * @return Generic pointer to the value, or NULL if key not found
 *
 * @note The returned pointer is allocated in the JSON object's arena and
 * will be freed when json_end() is called.
 *
 * @warning For type-safe access, use the helper macros like json_get_string(),
 * json_get_int(), etc., instead of casting the void* directly.
 *
 * Example:
 * @code
 * // Prefer type-safe macros:
 * char *name = json_get_string(json, "name", err);
 * int *age = json_get_int(json, "age", err);
 *
 * // Or use json_get() for generic access:
 * void *value = json_get(json, "key", err);
 * @endcode
 *
 * @see json_get_string(), json_get_int(), json_get_bool() for type-safe access
 */
void *json_get(JsonCtx *ctx, char *key, char *err);

/**
 * @brief Get the count of items in a parsed JSON array
 *
 * Returns the number of items in the most recently parsed array. This should
 * be called after json_array() or json_array_key() to determine how many
 * elements are available for iteration.
 *
 * @param ctx Pointer to a JSON object containing a parsed array
 * @param err  Error buffer for error messages
 * @return Number of items in the array, or -1 on error
 *
 * Example:
 * @code
 * char **items = json_array_key(json, "items", err);
 * int count = json_array_count(json, err);
 * for (int i = 0; i < count; i++) {
 *     printf("Item %d: %s\n", i, items[i]);
 * }
 * @endcode
 *
 * @see json_array() to parse an array
 * @see json_array_key() to parse an array by key
 */
int json_array_count(JsonCtx *ctx, char *err);

/**
 * @brief Free a JSON object and all associated memory
 *
 * This function releases all memory allocated by the JSON parser, including
 * the arena allocator and all parsed values. After calling this function,
 * all pointers obtained from the JSON object become invalid and must not
 * be accessed.
 *
 * @param ctx Pointer to a JSON object to free
 * @param err  Error buffer for error messages
 *
 * @warning After calling json_end(), do not access any strings, arrays, or
 * values that were obtained from the JSON object, as they have been freed.
 *
 * @note This function is safe to call with NULL; it will simply return.
 *
 * Example:
 * @code
 * JsonCtx *json = json_begin("{\"key\":\"value\"}", err);
 * char *value = json_get_string("key", json, err);
 * printf("%s\n", value);
 * json_end(json, err);  // 'value' pointer is now invalid
 * @endcode
 *
 * @see json_begin() to create a JSON object
 */
void json_end(JsonCtx *ctx, char *err);

#ifdef LIBJSON_IMPLEMENTATION

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// ARENA
// ============================================================================

/**
 * @brief Get alignment of a type in a portable way.
 *
 * This macro expands to `alignof(type)` when compiling under C11 or newer, and
 * otherwise computes alignment using struct offset hack. This ensures the arena
 * allocator can correctly align memory on all compilers.
 *
 * @param type  Any C type whose alignment is needed.
 */
#if __STDC_VERSION__ >= 201112L
#include <stdalign.h>
#define ARENA_ALIGNOF(type) alignof(type)
#else
#define ARENA_ALIGNOF(type)                                                    \
  offsetof(                                                                    \
      struct {                                                                 \
        char c;                                                                \
        type d;                                                                \
      },                                                                       \
      d)
#endif

#define DA_INIT_CAP 256

#define da_append(da, item)                                                    \
  do {                                                                         \
    if ((da)->capacity == 0) {                                                 \
      (da)->capacity = DA_INIT_CAP;                                            \
      (da)->items = malloc(sizeof((da)->items[0]) * (da)->capacity);           \
      if ((da)->items == NULL) {                                               \
        fprintf(stderr, "Buy more RAM LOL!\n");                                \
        exit(1);                                                               \
      }                                                                        \
    }                                                                          \
                                                                               \
    if ((da)->count == (da)->capacity) {                                       \
      (da)->capacity *= 2;                                                     \
      (da)->items =                                                            \
          realloc((da)->items, sizeof((da)->items[0]) * (da)->capacity);       \
      if ((da)->items == NULL) {                                               \
        fprintf(stderr, "Buy more RAM LOL!\n");                                \
        exit(1);                                                               \
      }                                                                        \
    }                                                                          \
    (da)->items[(da)->count++] = item;                                         \
  } while (0)

#define da_clear(da)                                                           \
  do {                                                                         \
    (da)->count = 0;                                                           \
    (da)->capacity = 0;                                                        \
    free((da)->items);                                                         \
  } while (0)

struct ArenaBlock {
  struct ArenaBlock *next;
  size_t capacity;
  size_t index;
  uint8_t data[];
};

/**
 * @brief Opaque handle for an Arena allocator.
 *
 * The arena manages memory using fixed-size blocks
 * and fast bump-pointer allocation.
 */
typedef struct {
  struct ArenaBlock *head;
  struct ArenaBlock *current;
  size_t default_block_size;
} Arena;

/**
 * @brief Checkpoint structure for saving/restoring arena state.
 *
 * Represents a specific point in the arena's allocation history.
 * Can be used to restore the arena to a previous state, effectively
 * freeing all allocations made after the checkpoint while keeping
 * allocations made before it.
 */
typedef struct ArenaCheckpoint {
  struct ArenaBlock *block; // Block pointer at checkpoint
  size_t index;             // Index within block at checkpoint
} ArenaCheckpoint;

/**
 * @brief Compute padding needed to align a pointer.
 *
 * This uses a modulo trick:
 *
 *   padding = (alignment - (ptr % alignment)) % alignment
 *
 * This ensures:
 *   - If pointer is already aligned → padding = 0
 *   - Otherwise → padding = minimal offset to align
 *
 * @param ptr        Pointer value as integer.
 * @param alignment  Required alignment (must be power of two).
 *
 * @return Number of bytes of padding needed.
 */
static size_t align_up(uintptr_t ptr, size_t alignment) {
  return (alignment - (ptr % alignment)) % alignment;
}

Arena *arena_create(size_t default_block_size) {
  if (default_block_size == 0)
    return NULL;

  Arena *arena = (Arena *)calloc(1, sizeof(Arena));
  if (!arena)
    return NULL;

  arena->default_block_size = default_block_size;
  return arena;
}

void *arena_alloc(Arena *arena, size_t size, size_t alignment) {
  if (!arena || size == 0 || alignment == 0)
    return NULL;

  // Ensure alignment is power of two.
  if (alignment & (alignment - 1))
    return NULL;

  // Lazily allocate first block.
  if (!arena->current) {
    size_t block_size =
        (size > arena->default_block_size) ? size : arena->default_block_size;

    struct ArenaBlock *block =
        (struct ArenaBlock *)malloc(sizeof(struct ArenaBlock) + block_size);
    if (!block)
      return NULL;

    block->next = NULL;
    block->capacity = block_size;
    block->index = 0;

    arena->head = arena->current = block;
  }

  // Compute padding for alignment.
  uintptr_t current_ptr =
      (uintptr_t)(arena->current->data + arena->current->index);

  size_t padding = align_up(current_ptr, alignment);

  // If insufficient space, allocate a new block.
  if (arena->current->index + padding + size > arena->current->capacity) {

    size_t next_capacity =
        (size > arena->default_block_size) ? size : arena->default_block_size;

    struct ArenaBlock *new_block =
        (struct ArenaBlock *)malloc(sizeof(struct ArenaBlock) + next_capacity);
    if (!new_block)
      return NULL;

    new_block->next = NULL;
    new_block->capacity = next_capacity;
    new_block->index = 0;

    arena->current->next = new_block;
    arena->current = new_block;

    current_ptr = (uintptr_t)new_block->data;
    padding = align_up(current_ptr, alignment);
  }

  // Perform the allocation.
  arena->current->index += padding;
  void *ptr = arena->current->data + arena->current->index;
  arena->current->index += size;

  return ptr;
}

void arena_free(Arena *arena) {
  if (!arena)
    return;

  struct ArenaBlock *block = arena->head;
  while (block) {
    struct ArenaBlock *next = block->next;
    free(block);
    block = next;
  }
  free(arena);
}

// ============================================================================
// JSON PARSER
// ============================================================================

// JSON character constants
#define JSON_OBJECT_OPEN '{'
#define JSON_OBJECT_CLOSE '}'
#define JSON_ARRAY_OPEN '['
#define JSON_ARRAY_CLOSE ']'
#define JSON_STRING_QUOTE '"'
#define JSON_ESCAPE_CHAR '\\'
#define JSON_KEY_VALUE_SEP ':'
#define JSON_VALUE_SEP ','

struct JsonCtx {
  size_t capacity;
  size_t count;
  char **items;
  char *source;
  Arena *arena;
};

/**
 * @brief Skips whitespace characters in a JSON string.
 *
 * Advances the cursor past any whitespace characters (space, tab, newline,
 * etc.) and returns a pointer to the first non-whitespace character.
 *
 * @param cursor  Pointer to current position in JSON string.
 * @return        Pointer to first non-whitespace character.
 */
static inline const char *skip_whitespace(const char *cursor) {
  while (*cursor && isspace(*cursor))
    cursor++;
  return cursor;
}

/**
 * @brief Skips over a JSON string value.
 *
 * Starting from an opening quote, this function advances past the entire
 * string value, handling escaped characters correctly. Returns a pointer
 * to the character immediately after the closing quote.
 *
 * @param cursor  Pointer to opening quote of the string.
 * @param err     Pointer to an error buffer for error messages.
 * @return        Pointer after the closing quote, or NULL on error.
 */
static const char *skip_string(const char *cursor, char *err) {
  if (*cursor != JSON_STRING_QUOTE) {
    if (err)
      snprintf(err, JSON_ERROR_BUFFER_SIZE, "%s",
               "skip_string: expected opening quote");
    return NULL;
  }

  cursor++; // Skip opening quote
  bool escaped = false;

  while (*cursor) {
    if (escaped) {
      escaped = false;
    } else if (*cursor == JSON_ESCAPE_CHAR) {
      escaped = true;
    } else if (*cursor == JSON_STRING_QUOTE) {
      return cursor + 1; // Return pointer after closing quote
    }
    cursor++;
  }

  if (err)
    snprintf(err, JSON_ERROR_BUFFER_SIZE, "%s",
             "skip_string: unterminated string");
  return NULL;
}

/**
 * @brief Finds the matching closing bracket for an opening bracket.
 *
 * This function navigates through nested JSON structures to find the
 * matching closing bracket ('}' for '{', ']' for '['). It correctly
 * handles nested brackets and string values, and enforces a depth limit
 * to prevent stack overflow on malformed JSON.
 *
 * @param start         Pointer to the opening bracket.
 * @param open_bracket  The opening bracket character ('{' or '[').
 * @param err           Pointer to an error buffer for error messages.
 * @return              Pointer to the matching closing bracket, or NULL on
 * error.
 */
static const char *find_matching_bracket(const char *start, char open_bracket,
                                         char *err) {
  if (*start != open_bracket) {
    if (err)
      snprintf(err, JSON_ERROR_BUFFER_SIZE,
               "find_matching_bracket: expected '%c'", open_bracket);
    return NULL;
  }

  char close_bracket =
      (open_bracket == JSON_OBJECT_OPEN) ? JSON_OBJECT_CLOSE : JSON_ARRAY_CLOSE;
  const char *cursor = start + 1;
  int depth = 1;
  bool in_string = false;
  bool escaped = false;

  while (*cursor && depth > 0) {
    if (in_string) {
      if (escaped) {
        escaped = false;
      } else if (*cursor == JSON_ESCAPE_CHAR) {
        escaped = true;
      } else if (*cursor == JSON_STRING_QUOTE) {
        in_string = false;
      }
    } else {
      if (*cursor == JSON_STRING_QUOTE) {
        in_string = true;
      } else if (*cursor == open_bracket) {
        depth++;
        // Enforce depth limit
        if (depth > JSON_DEPTH_LIMIT) {
          if (err)
            snprintf(err, JSON_ERROR_BUFFER_SIZE,
                     "find_matching_bracket: depth exceeds limit %d",
                     JSON_DEPTH_LIMIT);
          return NULL;
        }
      } else if (*cursor == close_bracket) {
        depth--;
        if (depth == 0) {
          return cursor;
        }
      }
    }
    cursor++;
  }

  if (err)
    snprintf(err, JSON_ERROR_BUFFER_SIZE,
             "find_matching_bracket: no matching '%c'", close_bracket);
  return NULL;
}

/**
 * @brief Finds a key in a JSON object and returns pointer to its value.
 *
 * This function searches through a JSON object string to locate a specific
 * key and returns a pointer to the start of its associated value. It handles
 * nested objects and arrays correctly, only matching keys at the top level
 * (depth 1) of the provided JSON string.
 *
 * @param key      The key to search for.
 * @param key_len  Length of the key string.
 * @param json     JSON object string to search within.
 * @param err      Pointer to an error buffer for error messages.
 * @return         Pointer to the value associated with the key, or NULL if not
 * found.
 */
static char *json_find_key(char *key, size_t key_len, char *json, char *err) {
  if (!key || !json) {
    if (err)
      snprintf(err, JSON_ERROR_BUFFER_SIZE, "%s",
               "json_find_key: key or json is not initialize");
    return NULL;
  }

  const char *cursor = json;
  int depth = 0;
  bool in_string = false;
  bool escaped = false;

  size_t target_len = key_len;

  while (*cursor) {
    char current_char = *cursor;

    if (in_string) {
      if (escaped)
        escaped = false; // this character is escaped, move on
      else if (current_char == '\\')
        escaped = true; // Next character will be escaped
      else if (current_char == '"')
        in_string = false; // end of string

      cursor++;
      continue;
    }

    switch (current_char) {
    case JSON_STRING_QUOTE:
      if (depth == 1) {
        const char *key_start = cursor + 1;
        const char *key_end = key_start;

        // Find the closing quote
        while (*key_end && *key_end != '"') {
          if (*key_end == '\\')
            key_end++; // Skip the escaped quote
          key_end++;   // Skip the quote
        }

        if (!key_end)
          continue;

        size_t key_len = key_end - key_start;

        if (key_len == target_len && strncmp(key_start, key, key_len) == 0) {
          // skip past the closing quote and find the colon
          cursor = key_end + 1;
          while (*cursor && isspace((unsigned char)*cursor))
            cursor++;

          if (*cursor == ':') {
            // skip past the colon and find the value
            cursor++;
            while (*cursor && isspace((unsigned char)*cursor))
              cursor++;
            return (char *)cursor;
          }
        }
      }
      in_string = true;
      break;
    case JSON_ARRAY_OPEN:
    case JSON_OBJECT_OPEN:
      depth++;
      if (depth > JSON_DEPTH_LIMIT) {
        if (err)
          snprintf(err, JSON_ERROR_BUFFER_SIZE,
                   "json_find_key: depth exceeds limit %d", JSON_DEPTH_LIMIT);
        return NULL;
      }
      break;
    case JSON_ARRAY_CLOSE:
    case JSON_OBJECT_CLOSE:
      depth--;
      break;
    }

    cursor++;
  }

  if (err)
    snprintf(err, JSON_ERROR_BUFFER_SIZE, "json_find_key: key %s not found",
             key);
  return NULL;
}

/**
 * @brief Extracts a JSON value and allocates it in the arena.
 *
 * This function extracts a complete JSON value (string, number, boolean, null,
 * object, or array) from the given position. The extracted value is allocated
 * in the provided arena and returned as a null-terminated string. For strings,
 * the surrounding quotes are removed. For objects and arrays, the full
 * structure including brackets is preserved.
 *
 * @param arena        Arena allocator for memory allocation.
 * @param value_start  Pointer to the start of the value in the JSON string.
 * @param value_len    Output parameter for the length of the extracted value.
 * @param err          Pointer to an error buffer for error messages.
 * @return             Pointer to the extracted value string, or NULL on error.
 */
static char *json_extract_value(Arena *arena, const char *value_start,
                                size_t *value_len, char *err) {

  if (!value_start || !arena) {
    if (err)
      snprintf(err, JSON_ERROR_BUFFER_SIZE, "%s",
               "json_extract_value: value_start or arena is NULL");
    return NULL;
  }

  const char *cursor = skip_whitespace(value_start);
  const char *value_end;

  if (*cursor == '"') {
    // String value
    cursor++;
    value_end = cursor;
    while (*value_end && *value_end != '"') {
      if (*value_end == '\\')
        value_end++; // Skip escaped chars
      value_end++;
    }
    *value_len = value_end - cursor;
    char *result = arena_alloc(arena, *value_len + 1, ARENA_ALIGNOF(char));
    memcpy(result, cursor, *value_len);
    result[*value_len] = '\0';
    return result;

  } else if (*cursor == '{' || *cursor == '[') {
    // Object or array - use helper to find matching bracket
    const char *start = cursor;
    char open = *cursor;
    value_end = find_matching_bracket(cursor, open, err);

    if (!value_end)
      return NULL;

    *value_len = value_end - start + 1; // Include closing bracket
    char *result = arena_alloc(arena, *value_len + 1, ARENA_ALIGNOF(char));
    memcpy(result, start, *value_len);
    result[*value_len] = '\0';
    return result;

  } else {
    // Number, boolean, or null - find end
    value_end = cursor;
    while (*value_end && !isspace(*value_end) && *value_end != ',' &&
           *value_end != '}' && *value_end != ']') {
      value_end++;
    }

    *value_len = value_end - cursor;
    char *result = arena_alloc(arena, *value_len + 1, ARENA_ALIGNOF(char));
    memcpy(result, cursor, *value_len);
    result[*value_len] = '\0';
    return result;
  }
}

JsonCtx *json_begin(char *json, char *err) {
  if (!json) {
    if (err)
      snprintf(err, JSON_ERROR_BUFFER_SIZE, "%s", "json_begin json is NULL");
    return NULL;
  }

  // Validate input size
  size_t json_len = strlen(json);
  if (json_len > JSON_MAX_INPUT_SIZE) {
    if (err)
      snprintf(err, JSON_ERROR_BUFFER_SIZE,
               "json_begin: input size %zu exceeds max %d bytes", json_len,
               JSON_MAX_INPUT_SIZE);
    return NULL;
  }

  // Validate basic JSON structure
  const char *cursor = json;
  while (*cursor && isspace(*cursor))
    cursor++;

  if (*cursor != '{' && *cursor != '[') {
    if (err)
      snprintf(err, JSON_ERROR_BUFFER_SIZE,
               "json_begin: invalid JSON, must start with '{' or '['");
    return NULL;
  }

  Arena *arena = arena_create(JSON_DEFAULT_ARENA_SIZE);
  if (!arena) {
    if (err)
      snprintf(err, JSON_ERROR_BUFFER_SIZE, "%s",
               "json_begin arena_create failed");
    return NULL;
  }

  JsonCtx *json_obj =
      (JsonCtx *)arena_alloc(arena, sizeof(JsonCtx), ARENA_ALIGNOF(JsonCtx));
  if (!json_obj) {
    arena_free(arena);
    if (err)
      snprintf(err, JSON_ERROR_BUFFER_SIZE, "%s",
               "json_begin arena_alloc failed");
    return NULL;
  }

  json_obj->items = NULL;
  json_obj->capacity = 0;
  json_obj->count = 0;
  json_obj->source = json;
  json_obj->arena = arena;
  return json_obj;
}

JsonCtx *json_obj(JsonCtx *ctx, char *json_obj, char *err) {
  if (!ctx || !json_obj) {
    if (err)
      snprintf(err, JSON_ERROR_BUFFER_SIZE, "%s",
               "json_new_obj *JsonCtx or json_obj is not initialize");
    return NULL;
  }

  ctx->source = json_obj;
  return ctx;
}

JsonCtx *json_obj_key(JsonCtx *ctx, char *key, char *err) {
  if (!ctx) {
    if (err)
      snprintf(err, JSON_ERROR_BUFFER_SIZE, "%s",
               "json_new *JsonCtx is not initialize");
    return NULL;
  }

  char *source = (char *)json_get(ctx, key, err);
  if (!source)
    return NULL;

  JsonCtx *new_json = json_obj(ctx, source, err);
  return new_json;
}

int json_array_count(JsonCtx *ctx, char *err) {
  if (!ctx) {
    if (err)
      snprintf(err, JSON_ERROR_BUFFER_SIZE, "%s",
               "json_begin_array *JsonCtx is NULL or Empty");
    return -1;
  }
  return ctx->count;
}

char **json_array_key(JsonCtx *ctx, char *key, char *err) {
  if (!ctx) {
    if (err)
      snprintf(err, JSON_ERROR_BUFFER_SIZE, "%s",
               "json_new *JsonCtx is not initialize");
    return NULL;
  }

  char *source = (char *)json_get(ctx, key, err);
  if (!source)
    return NULL;

  char **new_json = json_array(ctx, source, err);
  return new_json;
}

char **json_array(JsonCtx *ctx, char *source, char *err) {
  if (!ctx || !source) {
    if (err)
      snprintf(err, JSON_ERROR_BUFFER_SIZE, "%s",
               "json_begin_array *JsonCtx or source is NULL or Empty");
    return NULL;
  }

  ctx->source = source;
  char *array_start = ctx->source;

  // Skip whitespace
  while (*array_start && isspace(*array_start))
    array_start++;

  // Verify it's an array
  if (*array_start != '[') {
    if (err)
      snprintf(err, JSON_ERROR_BUFFER_SIZE, "%s",
               "json_begin_array: value is not an array");
    return NULL;
  }

  // Start parsing after '['
  const char *cursor = array_start + 1;

  while (*cursor) {
    cursor = skip_whitespace(cursor);

    // Check for end of array
    if (*cursor == ']')
      break;

    // Skip commas
    if (*cursor == ',') {
      cursor++;
      continue;
    }

    // Found an object - extract from { to }
    if (*cursor == '{') {
      const char *obj_start = cursor;
      const char *obj_end = find_matching_bracket(cursor, '{', err);

      if (!obj_end)
        return NULL;

      // Extract and append the object
      size_t len = obj_end - obj_start + 1; // Include closing brace
      char *str = arena_alloc(ctx->arena, len + 1, ARENA_ALIGNOF(char));
      memcpy(str, obj_start, len);
      str[len] = '\0';
      da_append(ctx, str);

      cursor = obj_end + 1;
    } else {
      // Unexpected character
      if (err)
        snprintf(err, JSON_ERROR_BUFFER_SIZE,
                 "json_begin_array: expected '{' but found '%c'", *cursor);
      return NULL;
    }
  }

  return ctx->items;
}

void *json_get(JsonCtx *ctx, char *key, char *err) {
  if (!ctx || !ctx->source || !key) {
    if (err)
      snprintf(err, JSON_ERROR_BUFFER_SIZE, "%s",
               "json_get *JsonCtx or key is not initialize");
    return NULL;
  }

  size_t key_len = strlen(key);
  char *value = json_find_key(key, key_len, ctx->source, err);
  if (!value)
    return NULL;

  size_t value_len;
  char *result = json_extract_value(ctx->arena, value, &value_len, err);
  return (void *)result;
}

void json_end(JsonCtx *ctx, char *err) {
  if (!ctx) {
    if (err)
      snprintf(err, JSON_ERROR_BUFFER_SIZE, "%s",
               "json_end *JsonCtx is not initialize");
    return;
  }
  da_clear(ctx);
  arena_free(ctx->arena);
  ctx->source = NULL;
  ctx->arena = NULL;
}

#endif // LIBJSON_IMPLEMENTATION

#ifdef __cplusplus
}
#endif

#endif // LIBJSON_H
