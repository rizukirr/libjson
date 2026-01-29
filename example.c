#define LIBJSON_IMPLEMENTATION
#include "libjson.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Read entire file into a string buffer
 * Returns allocated string or NULL on failure
 */
static char *read_file(const char *filename) {
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "Error: Failed to open '%s'\n", filename);
    return NULL;
  }

  // Get file size
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  rewind(fp);

  // Allocate buffer
  char *buffer = malloc(size + 1);
  if (!buffer) {
    fprintf(stderr, "Error: Failed to allocate memory for file\n");
    fclose(fp);
    return NULL;
  }

  // Read file contents
  size_t read_size = fread(buffer, 1, size, fp);
  buffer[read_size] = '\0';
  fclose(fp);

  return buffer;
}

/**
 * Print a single output item (pretty formatted)
 */
static void print_output_item(JsonContext *ctx, char *output_item, size_t index,
                              size_t total) {
  // Extract output fields
  char *output_id = get_value(ctx, "id", output_item);

  // Extract nested content object
  char *content_json = get_value(ctx, "content", output_item);
  char *content_type = get_value(ctx, "type", content_json);
  char *content_text = get_value(ctx, "text", content_json);

  // Print formatted output
  printf("    {\n");
  printf("      \"id\": \"%s\",\n", output_id);
  printf("      \"content\": {\n");
  printf("        \"type\": \"%s\",\n", content_type);
  printf("        \"text\": \"%s\"\n", content_text);
  printf("      }\n");

  // Add comma for all but last item
  if (index < total - 1) {
    printf("    },\n");
  } else {
    printf("    }\n");
  }
}

int main(void) {
  // Read JSON file
  char *json_data = read_file("example.json");
  if (!json_data) {
    return 1;
  }

  // Initialize JSON parser context
  JsonContext *ctx = json_begin();
  if (!ctx) {
    fprintf(stderr, "Error: Failed to initialize JSON context\n");
    free(json_data);
    return 1;
  }

  // Extract top-level fields
  char *response_id = get_value(ctx, "id", json_data);
  char *model_name = get_value(ctx, "model", json_data);

  // Print response header
  printf("{\n");
  printf("  \"id\": \"%s\",\n", response_id);
  printf("  \"model\": \"%s\",\n", model_name);
  printf("  \"output\": [\n");

  // Extract and parse output array
  char *output_array_json = get_value(ctx, "output", json_data);
  size_t output_count = 0;
  char **output_items =
      get_array(ctx, "output", output_array_json, &output_count);

  // Print each output item
  for (size_t i = 0; i < output_count; i++) {
    print_output_item(ctx, output_items[i], i, output_count);
  }

  // Print response footer
  printf("  ]\n");
  printf("}\n");

  // Cleanup
  json_end(ctx);
  free(json_data);

  return 0;
}
