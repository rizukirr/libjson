#define LIBJSON_IMPLEMENTATION

#include "../libjson.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Simplified example demonstrating how to parse example.json
 */

// Struct definitions matching the simplified example.json structure

typedef struct {
  char *type;
  char *text;
} Content;

typedef struct {
  char *id;
  Content content;
} OutputMessage;

typedef struct {
  char *id;
  char *model;
  OutputMessage *output;
  int output_count;
} Response;

/**
 * Helper function to parse a single output message from JSON string
 */
OutputMessage parse_output_message(JsonCtx *ctx, char *output_json, char *err) {
  OutputMessage msg = {0};

  // Parse id
  char *id = json_get_string(ctx, "id", err);
  if (id)
    msg.id = strdup(id);

  // Parse content object
  char *content_json = json_get_string(ctx, "content", err);
  if (content_json) {
    char *type = json_get_string(ctx, "type", err);
    if (type)
      msg.content.type = strdup(type);

    char *text = json_get_string(ctx, "text", err);
    if (text)
      msg.content.text = strdup(text);
  }

  return msg;
}

/**
 * Main function demonstrating the parsing workflow
 */
int main(void) {
  // Step 1: Read the JSON file
  FILE *fp = fopen("example/example.json", "r");
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

  // Step 2: Initialize error buffer
  char err[256] = {0};

  // Step 3: Parse the root JSON object
  JsonCtx *root = json_begin(json_string, err);
  if (!root) {
    fprintf(stderr, "Failed to parse JSON: %s\n", err);
    free(json_string);
    return 1;
  }

  // Step 4: Create and populate the Response struct
  Response response = {0};

  // Parse top-level string fields
  response.id = json_get_string(root, "id", err);
  response.model = json_get_string(root, "model", err);

  // Parse array: output (array of message objects)
  char **output_items = json_array_key(root, "output", err);
  if (output_items) {
    int count = json_array_count(root, err);
    response.output_count = count > 0 ? count : 0;
    response.output = malloc(sizeof(OutputMessage) * count);
    for (int i = 0; i < count; i++) {
      JsonCtx *item = json_obj(root, output_items[i], err);
      response.output[i].id = json_get_string(item, "id", err);
      JsonCtx *content_item = json_obj_key(item, "content", err);
      response.output[i].content.type =
          json_get_string(content_item, "type", err);
      response.output[i].content.text =
          json_get_string(content_item, "text", err);
    }
  }

  // Step 5: Print the parsed data to verify
  printf("=== Parsed Response Data ===\n\n");
  printf("ID: %s\n", response.id ? response.id : "N/A");
  printf("Model: %s\n", response.model ? response.model : "N/A");
  printf("\n");

  printf("=== Output Messages (%d) ===\n", response.output_count);
  for (int i = 0; i < response.output_count; i++) {
    printf("\nMessage %d:\n", i + 1);
    printf("  ID: %s\n", response.output[i].id ? response.output[i].id : "N/A");
    printf("  Content:\n");
    printf("    Type: %s\n", response.output[i].content.type
                                 ? response.output[i].content.type
                                 : "N/A");
    printf("    Text: %s\n", response.output[i].content.text
                                 ? response.output[i].content.text
                                 : "N/A");
  }

  if (response.output) {
    free(response.output);
  }

  json_end(root, err);
  free(json_string);

  printf("\n=== Parsing Complete ===\n");
  return 0;
}
