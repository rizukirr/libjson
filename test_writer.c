#define LIBJSON_IMPLEMENTATION
#include "libjson.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_simple_object(void) {
  char buf[256];
  JsonWriter w;
  json_writer_init(&w, buf, sizeof(buf));

  json_write_object_begin(&w);
  json_write_key(&w, "name");
  json_write_str(&w, "sensor-01");
  json_write_key(&w, "temp");
  json_write_int(&w, 2350);
  json_write_key(&w, "active");
  json_write_bool(&w, true);
  json_write_object_end(&w);

  assert(json_writer_ok(&w));
  assert(strcmp(buf, "{\"name\":\"sensor-01\",\"temp\":2350,\"active\":true}") == 0);
  printf("PASS: simple object\n");
}

static void test_nested_array(void) {
  char buf[256];
  JsonWriter w;
  json_writer_init(&w, buf, sizeof(buf));

  json_write_object_begin(&w);
  json_write_key(&w, "readings");
  json_write_array_begin(&w);
  json_write_int(&w, 100);
  json_write_int(&w, 200);
  json_write_int(&w, 300);
  json_write_array_end(&w);
  json_write_object_end(&w);

  assert(json_writer_ok(&w));
  assert(strcmp(buf, "{\"readings\":[100,200,300]}") == 0);
  printf("PASS: nested array\n");
}

static void test_escape(void) {
  char buf[256];
  JsonWriter w;
  json_writer_init(&w, buf, sizeof(buf));

  json_write_str(&w, "hello\t\"world\"\n");

  assert(json_writer_ok(&w));
  assert(strcmp(buf, "\"hello\\t\\\"world\\\"\\n\"") == 0);
  printf("PASS: string escaping\n");
}

static void test_overflow(void) {
  char buf[8];
  JsonWriter w;
  json_writer_init(&w, buf, sizeof(buf));

  json_write_str(&w, "this string is way too long for the buffer");

  assert(!json_writer_ok(&w));
  printf("PASS: overflow detection\n");
}

static void test_null_and_bool(void) {
  char buf[64];
  JsonWriter w;
  json_writer_init(&w, buf, sizeof(buf));

  json_write_array_begin(&w);
  json_write_null(&w);
  json_write_bool(&w, false);
  json_write_bool(&w, true);
  json_write_array_end(&w);

  assert(json_writer_ok(&w));
  assert(strcmp(buf, "[null,false,true]") == 0);
  printf("PASS: null and bool\n");
}

static void test_negative_int(void) {
  char buf[64];
  JsonWriter w;
  json_writer_init(&w, buf, sizeof(buf));

  json_write_int(&w, -42);

  assert(json_writer_ok(&w));
  assert(strcmp(buf, "-42") == 0);
  printf("PASS: negative int\n");
}

static void test_raw(void) {
  char buf[64];
  JsonWriter w;
  json_writer_init(&w, buf, sizeof(buf));

  json_write_object_begin(&w);
  json_write_key(&w, "pi");
  json_write_raw(&w, "3.14", 4);
  json_write_object_end(&w);

  assert(json_writer_ok(&w));
  assert(strcmp(buf, "{\"pi\":3.14}") == 0);
  printf("PASS: raw value\n");
}

static void test_empty_containers(void) {
  char buf[64];
  JsonWriter w;
  json_writer_init(&w, buf, sizeof(buf));

  json_write_object_begin(&w);
  json_write_key(&w, "obj");
  json_write_object_begin(&w);
  json_write_object_end(&w);
  json_write_key(&w, "arr");
  json_write_array_begin(&w);
  json_write_array_end(&w);
  json_write_object_end(&w);

  assert(json_writer_ok(&w));
  assert(strcmp(buf, "{\"obj\":{},\"arr\":[]}") == 0);
  printf("PASS: empty containers\n");
}

static void test_control_char_escape(void) {
  char buf[64];
  JsonWriter w;
  json_writer_init(&w, buf, sizeof(buf));

  json_write_str(&w, "\x01\x1f");

  assert(json_writer_ok(&w));
  assert(strcmp(buf, "\"\\u0001\\u001f\"") == 0);
  printf("PASS: control char \\uXXXX escaping\n");
}

static void test_uint(void) {
  char buf[64];
  JsonWriter w;
  json_writer_init(&w, buf, sizeof(buf));

  json_write_uint(&w, 4294967295UL);

  assert(json_writer_ok(&w));
  printf("PASS: uint (%s)\n", buf);
}

static void test_null_str(void) {
  char buf[64];
  JsonWriter w;
  json_writer_init(&w, buf, sizeof(buf));

  json_write_str(&w, NULL);

  assert(json_writer_ok(&w));
  assert(strcmp(buf, "null") == 0);
  printf("PASS: NULL string -> null\n");
}

int main(void) {
  test_simple_object();
  test_nested_array();
  test_escape();
  test_overflow();
  test_null_and_bool();
  test_negative_int();
  test_raw();
  test_empty_containers();
  test_control_char_escape();
  test_uint();
  test_null_str();
  printf("\nAll writer tests passed.\n");
  return 0;
}
