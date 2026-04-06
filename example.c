#define LIBJSON_IMPLEMENTATION
#include "libjson.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
  size_t statuses_count;
  size_t total_text_bytes;
  JsonSlice metadata_count;
  JsonSlice first_id;
  JsonSlice first_screen_name;
  JsonSlice first_text;
} PayloadSummary;

static char *read_file(const char *filename) {
  FILE *fp = NULL;
  char *buffer;
  long size;
  size_t read_size;

  #ifdef _MSC_VER
  if (fopen_s(&fp, filename, "rb") != 0)
    fp = NULL;
  #else
  fp = fopen(filename, "rb");
  #endif

  if (!fp) {
    fprintf(stderr, "Error: failed to open '%s'\n", filename);
    return NULL;
  }

  if (fseek(fp, 0, SEEK_END) != 0) {
    fclose(fp);
    return NULL;
  }

  size = ftell(fp);
  if (size < 0) {
    fclose(fp);
    return NULL;
  }

  rewind(fp);
  buffer = (char *)malloc((size_t)size + 1);
  if (!buffer) {
    fclose(fp);
    return NULL;
  }

  read_size = fread(buffer, 1, (size_t)size, fp);
  buffer[read_size] = '\0';
  fclose(fp);
  return buffer;
}

static void print_slice_line(const char *label, JsonSlice value) {
  printf("%s: ", label);
  fwrite(value.data, 1, value.len, stdout);
  putchar('\n');
}

static int analyze_payload(JsonSlice root, PayloadSummary *summary) {
  JsonSlice statuses;
  JsonSlice metadata;
  JsonArrayIter status_iter;
  JsonSlice status;

  summary->statuses_count = 0;
  summary->total_text_bytes = 0;
  summary->metadata_count = json_from_parts(NULL, 0);
  summary->first_id = json_from_parts(NULL, 0);
  summary->first_screen_name = json_from_parts(NULL, 0);
  summary->first_text = json_from_parts(NULL, 0);

  if (!json_get(root, "statuses", &statuses) ||
      !json_get(root, "search_metadata", &metadata) ||
      !json_get(metadata, "count", &summary->metadata_count)) {
    fprintf(stderr, "Error: failed to read top-level twitter payload fields\n");
    return 0;
  }

  json_array_iter_init(statuses, &status_iter);
  while (json_array_iter_next(&status_iter, &status)) {
    JsonSlice text;

    if (!json_get(status, "text", &text)) {
      fprintf(stderr, "Error: failed to read status text\n");
      return 0;
    }

    summary->statuses_count++;
    summary->total_text_bytes += text.len;

    if (summary->statuses_count == 1) {
      JsonSlice user;

      if (!json_get(status, "id_str", &summary->first_id) ||
          !json_get(status, "user", &user) ||
          !json_get(user, "screen_name", &summary->first_screen_name)) {
        fprintf(stderr, "Error: failed to read first status metadata\n");
        return 0;
      }

      summary->first_text = text;
    }
  }

  return 1;
}

static void run_benchmark(JsonSlice root, size_t iterations) {
  PayloadSummary summary;
  clock_t start = clock();
  clock_t end;
  size_t i;
  double elapsed_ms;
  double elapsed_seconds;
  double total_bytes;
  double gib_per_second;

  for (i = 0; i < iterations; ++i) {
    if (!analyze_payload(root, &summary)) {
      fprintf(stderr, "Benchmark aborted during iteration %lu\n",
              (unsigned long)i);
      return;
    }
  }

  end = clock();
  elapsed_ms = ((double)(end - start) * 1000.0) / (double)CLOCKS_PER_SEC;
  elapsed_seconds = (double)(end - start) / (double)CLOCKS_PER_SEC;
  total_bytes = (double)root.len * (double)iterations;
  gib_per_second =
      elapsed_seconds > 0.0 ? total_bytes / elapsed_seconds / (1024.0 * 1024.0 * 1024.0)
                            : 0.0;

  printf("\nBenchmark\n");
  printf("iterations: %lu\n", (unsigned long)iterations);
  printf("elapsed_ms: %.3f\n", elapsed_ms);
  printf("avg_ms_per_iteration: %.6f\n", elapsed_ms / (double)iterations);
  printf("statuses_per_iteration: %lu\n", (unsigned long)summary.statuses_count);
  printf("throughput_gib_per_s: %.6f\n", gib_per_second);
}

int main(int argc, char **argv) {
  char *json_data = read_file("example.json");
  JsonSlice root;
  PayloadSummary summary;
  size_t iterations = 200;

  if (!json_data)
    return 1;

  if (argc > 1) {
    char *endptr = NULL;
    unsigned long parsed = strtoul(argv[1], &endptr, 10);
    if (endptr && *endptr == '\0' && parsed > 0)
      iterations = (size_t)parsed;
  }

  root = json_from_cstr(json_data);
  if (!analyze_payload(root, &summary)) {
    free(json_data);
    return 1;
  }

  printf("Twitter payload summary\n");
  print_slice_line("metadata.count", summary.metadata_count);
  printf("statuses.count: %lu\n", (unsigned long)summary.statuses_count);
  printf("statuses.total_text_bytes: %lu\n",
         (unsigned long)summary.total_text_bytes);
  print_slice_line("first_status.id_str", summary.first_id);
  print_slice_line("first_status.user.screen_name", summary.first_screen_name);
  print_slice_line("first_status.text", summary.first_text);

  run_benchmark(root, iterations);

  free(json_data);
  return 0;
}
