#define LIBJSON_IMPLEMENTATION
#include "libjson.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static bool slice_eq(JsonSlice s, const char *expected) {
  size_t len = strlen(expected);
  return s.data && s.len == len && strncmp(s.data, expected, len) == 0;
}

static void test_get_string(void) {
  JsonSlice root = json_from_cstr("{\"name\":\"alice\",\"city\":\"tokyo\"}");
  JsonSlice val;

  assert(json_get(root, "name", &val));
  assert(slice_eq(val, "alice"));

  assert(json_get(root, "city", &val));
  assert(slice_eq(val, "tokyo"));

  assert(!json_get(root, "missing", &val));

  printf("PASS: get string\n");
}

static void test_get_number(void) {
  JsonSlice root = json_from_cstr("{\"temp\":2350,\"neg\":-42}");
  JsonSlice val;

  assert(json_get(root, "temp", &val));
  assert(slice_eq(val, "2350"));

  assert(json_get(root, "neg", &val));
  assert(slice_eq(val, "-42"));

  printf("PASS: get number\n");
}

static void test_get_bool_and_null(void) {
  JsonSlice root = json_from_cstr("{\"on\":true,\"off\":false,\"x\":null}");
  JsonSlice val;

  assert(json_get(root, "on", &val));
  assert(slice_eq(val, "true"));

  assert(json_get(root, "off", &val));
  assert(slice_eq(val, "false"));

  assert(json_get(root, "x", &val));
  assert(slice_eq(val, "null"));

  printf("PASS: get bool and null\n");
}

static void test_get_nested_object(void) {
  JsonSlice root = json_from_cstr("{\"user\":{\"id\":1,\"name\":\"bob\"}}");
  JsonSlice user, val;

  assert(json_get(root, "user", &user));
  assert(json_get(user, "id", &val));
  assert(slice_eq(val, "1"));
  assert(json_get(user, "name", &val));
  assert(slice_eq(val, "bob"));

  printf("PASS: get nested object\n");
}

static void test_get_nested_array(void) {
  JsonSlice root = json_from_cstr("{\"tags\":[\"a\",\"b\",\"c\"]}");
  JsonSlice arr;

  assert(json_get(root, "tags", &arr));
  assert(arr.data[0] == '[');

  printf("PASS: get nested array\n");
}

static void test_getn(void) {
  JsonSlice root = json_from_cstr("{\"ab\":1,\"abc\":2}");
  JsonSlice val;

  assert(json_getn(root, "ab", 2, &val));
  assert(slice_eq(val, "1"));

  assert(json_getn(root, "abc", 3, &val));
  assert(slice_eq(val, "2"));

  printf("PASS: getn explicit length\n");
}

static void test_escaped_string_value(void) {
  JsonSlice root = json_from_cstr("{\"msg\":\"hello\\\"world\"}");
  JsonSlice val;

  assert(json_get(root, "msg", &val));
  assert(slice_eq(val, "hello\\\"world"));

  printf("PASS: escaped string value\n");
}

static void test_slice_copy(void) {
  JsonSlice root = json_from_cstr("{\"k\":\"value\"}");
  JsonSlice val;
  char buf[16];

  assert(json_get(root, "k", &val));
  assert(json_slice_copy(val, buf, sizeof(buf)));
  assert(strcmp(buf, "value") == 0);

  /* buffer too small */
  char tiny[3];
  assert(!json_slice_copy(val, tiny, sizeof(tiny)));

  printf("PASS: slice copy\n");
}

static void test_object_iter(void) {
  JsonSlice root = json_from_cstr("{\"a\":1,\"b\":2,\"c\":3}");
  JsonObjectIter iter;
  JsonSlice key, val;
  int count = 0;

  json_object_iter_init(root, &iter);
  while (json_object_iter_next(&iter, &key, &val))
    count++;

  assert(count == 3);
  printf("PASS: object iterator\n");
}

static void test_object_iter_values(void) {
  JsonSlice root = json_from_cstr("{\"x\":\"hello\",\"y\":42}");
  JsonObjectIter iter;
  JsonSlice key, val;

  json_object_iter_init(root, &iter);

  assert(json_object_iter_next(&iter, &key, &val));
  assert(slice_eq(key, "x"));
  assert(slice_eq(val, "hello"));

  assert(json_object_iter_next(&iter, &key, &val));
  assert(slice_eq(key, "y"));
  assert(slice_eq(val, "42"));

  assert(!json_object_iter_next(&iter, &key, &val));

  printf("PASS: object iterator values\n");
}

static void test_array_iter(void) {
  JsonSlice root = json_from_cstr("{\"items\":[10,20,30]}");
  JsonSlice arr, val;
  JsonArrayIter iter;
  int count = 0;

  assert(json_get(root, "items", &arr));
  json_array_iter_init(arr, &iter);
  while (json_array_iter_next(&iter, &val))
    count++;

  assert(count == 3);
  printf("PASS: array iterator\n");
}

static void test_array_iter_values(void) {
  JsonSlice arr = json_from_cstr("[\"one\",\"two\",\"three\"]");
  JsonArrayIter iter;
  JsonSlice val;

  json_array_iter_init(arr, &iter);

  assert(json_array_iter_next(&iter, &val));
  assert(slice_eq(val, "one"));

  assert(json_array_iter_next(&iter, &val));
  assert(slice_eq(val, "two"));

  assert(json_array_iter_next(&iter, &val));
  assert(slice_eq(val, "three"));

  assert(!json_array_iter_next(&iter, &val));

  printf("PASS: array iterator values\n");
}

static void test_array_mixed_types(void) {
  JsonSlice arr = json_from_cstr("[1,\"two\",true,null,{\"k\":\"v\"},[3]]");
  JsonArrayIter iter;
  JsonSlice val;

  json_array_iter_init(arr, &iter);

  assert(json_array_iter_next(&iter, &val));
  assert(slice_eq(val, "1"));

  assert(json_array_iter_next(&iter, &val));
  assert(slice_eq(val, "two"));

  assert(json_array_iter_next(&iter, &val));
  assert(slice_eq(val, "true"));

  assert(json_array_iter_next(&iter, &val));
  assert(slice_eq(val, "null"));

  assert(json_array_iter_next(&iter, &val));
  assert(val.data[0] == '{');

  assert(json_array_iter_next(&iter, &val));
  assert(val.data[0] == '[');

  assert(!json_array_iter_next(&iter, &val));

  printf("PASS: array mixed types\n");
}

static void test_empty_object(void) {
  JsonSlice root = json_from_cstr("{}");
  JsonObjectIter iter;
  JsonSlice key, val;

  json_object_iter_init(root, &iter);
  assert(!json_object_iter_next(&iter, &key, &val));

  printf("PASS: empty object\n");
}

static void test_empty_array(void) {
  JsonSlice arr = json_from_cstr("[]");
  JsonArrayIter iter;
  JsonSlice val;

  json_array_iter_init(arr, &iter);
  assert(!json_array_iter_next(&iter, &val));

  printf("PASS: empty array\n");
}

static void test_whitespace(void) {
  JsonSlice root = json_from_cstr("  { \"a\" : 1 , \"b\" : 2 }  ");
  JsonSlice val;

  assert(json_get(root, "a", &val));
  assert(slice_eq(val, "1"));

  assert(json_get(root, "b", &val));
  assert(slice_eq(val, "2"));

  printf("PASS: whitespace handling\n");
}

static void test_deeply_nested(void) {
  JsonSlice root = json_from_cstr("{\"a\":{\"b\":{\"c\":{\"d\":\"deep\"}}}}");
  JsonSlice a, b, c, val;

  assert(json_get(root, "a", &a));
  assert(json_get(a, "b", &b));
  assert(json_get(b, "c", &c));
  assert(json_get(c, "d", &val));
  assert(slice_eq(val, "deep"));

  printf("PASS: deeply nested\n");
}

static void test_invalid_inputs(void) {
  JsonSlice root = json_from_cstr("not json");
  JsonSlice val;

  assert(!json_get(root, "key", &val));

  JsonSlice empty = json_from_parts(NULL, 0);
  assert(!json_get(empty, "key", &val));

  printf("PASS: invalid inputs\n");
}

static void test_from_parts(void) {
  const char *data = "hello world";
  JsonSlice s = json_from_parts(data + 6, 5);

  assert(s.len == 5);
  assert(strncmp(s.data, "world", 5) == 0);

  printf("PASS: from_parts\n");
}

int main(void) {
  test_get_string();
  test_get_number();
  test_get_bool_and_null();
  test_get_nested_object();
  test_get_nested_array();
  test_getn();
  test_escaped_string_value();
  test_slice_copy();
  test_object_iter();
  test_object_iter_values();
  test_array_iter();
  test_array_iter_values();
  test_array_mixed_types();
  test_empty_object();
  test_empty_array();
  test_whitespace();
  test_deeply_nested();
  test_invalid_inputs();
  test_from_parts();
  printf("\nAll reader tests passed.\n");
  return 0;
}
