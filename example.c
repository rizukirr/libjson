#define LIBJSON_IMPLEMENTATION
#include "libjson.h"
#include <stdio.h>
#include <stdlib.h>

struct Example {
  char *id;
  char *model;
  struct ExampleOutput *output;
};

struct ExampleOutput {
  char *id;
  struct ExampleOutputContent *content;
};

struct ExampleOutputContent {
  char *type;
  char *text;
};

int main(void) {
  FILE *fp = fopen("example.json", "r");
  if (!fp) {
    fprintf(stderr, "Failed to open example.json\n");
    return 1;
  }

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  rewind(fp);

  char *json_string = malloc(sizeof(char) * (size + 1));
  if (!json_string) {
    fprintf(stderr, "Failed to allocate memory\n");
    fclose(fp);
    return 1;
  }

  fread(json_string, size, 1, fp);
  json_string[size] = '\0';
  fclose(fp);

  JsonContext *ctx = json_begin();
  if (!ctx) {
    fprintf(stderr, "json_begin failed\n");
    return 1;
  }

  char *id = get_value(ctx, "id", json_string);
  char *model = get_value(ctx, "model", json_string);
  printf("{\n");
  printf("  \"id\": \"%s\",\n", id);
  printf("  \"model\": \"%s\",\n", model);
  printf("  \"output\": [\n");

  // get raw json array
  char *output_raw = get_value(ctx, "output", json_string);
  // get array of raw json objects
  char **output_arr = (char **)get_array(ctx, "output", output_raw);
  size_t output_count = json_array_count(ctx);
  for (size_t i = 0; i < output_count; i++) {
    // get raw json object
    char *output_obj = output_arr[i];
    char *id = get_value(ctx, "id", output_obj);

    // get raw content json object
    char *content_obj = get_value(ctx, "content", output_obj);
    char *type = get_value(ctx, "type", content_obj);
    char *text = get_value(ctx, "text", content_obj);

    printf("    {\n");
    printf("      \"id\": \"%s\",\n", id);
    printf("      \"content\": {\n");
    printf("        \"type\": \"%s\",\n", type);
    printf("        \"text\": \"%s\"\n", text);
    printf("      }\n");
    if (i < output_count - 1) {
      printf("    },\n");
    } else {
      printf("    }\n");
    }
  }
  printf("  ]\n");
  printf("}\n");

  json_end(ctx);
  free(json_string);
  return 0;
}
