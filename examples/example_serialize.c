#define LIBJSON_IMPLEMENTATION
#include "../libjson.h"

#include <stdio.h>
#include <string.h>

typedef struct {
  const char *name;
  long id;
  const char *email;
  const char *roles[4];
  size_t roles_count;
  bool active;
} User;

static bool user_to_json(const User *user, char *buf, size_t cap) {
  JsonWriter w;
  size_t i;

  json_writer_init(&w, buf, cap);

  json_write_object_begin(&w);
  json_write_str_kv(&w, "name", user->name);
  json_write_int_kv(&w, "id", user->id);
  json_write_str_kv(&w, "email", user->email);
  json_write_bool_kv(&w, "active", user->active);

  json_write_array_begin_k(&w, "roles");
  for (i = 0; i < user->roles_count; i++)
    json_write_str(&w, user->roles[i]);
  json_write_array_end(&w);

  json_write_object_end(&w);

  return json_writer_ok(&w);
}

static bool users_to_json(const User *users, size_t count, char *buf,
                          size_t cap) {
  JsonWriter w;
  size_t i, j;

  json_writer_init(&w, buf, cap);

  json_write_array_begin(&w);
  for (i = 0; i < count; i++) {
    json_write_object_begin(&w);
    json_write_str_kv(&w, "name", users[i].name);
    json_write_int_kv(&w, "id", users[i].id);
    json_write_str_kv(&w, "email", users[i].email);
    json_write_bool_kv(&w, "active", users[i].active);

    json_write_array_begin_k(&w, "roles");
    for (j = 0; j < users[i].roles_count; j++)
      json_write_str(&w, users[i].roles[j]);
    json_write_array_end(&w);

    json_write_object_end(&w);
  }
  json_write_array_end(&w);

  return json_writer_ok(&w);
}

int main(void) {
  char buf[512];

  /* Serialize a single user */
  User alice = {
      .name = "Alice",
      .id = 42,
      .email = "alice@example.com",
      .roles = {"admin", "editor"},
      .roles_count = 2,
      .active = true,
  };

  printf("=== Single User ===\n");
  if (user_to_json(&alice, buf, sizeof(buf))) {
    printf("%s\n", buf);
  } else {
    fprintf(stderr, "Buffer overflow\n");
    return 1;
  }

  /* Serialize an array of users */
  User users[] = {
      {.name = "Alice",
       .id = 1,
       .email = "alice@example.com",
       .roles = {"admin"},
       .roles_count = 1,
       .active = true},
      {.name = "Bob",
       .id = 2,
       .email = "bob@example.com",
       .roles = {"viewer", "editor"},
       .roles_count = 2,
       .active = false},
  };

  printf("\n=== User Array ===\n");
  if (users_to_json(users, 2, buf, sizeof(buf))) {
    printf("%s\n", buf);
  } else {
    fprintf(stderr, "Buffer overflow\n");
    return 1;
  }

  /* Demonstrate overflow detection */
  char tiny[16];
  printf("\n=== Overflow Test ===\n");
  if (!user_to_json(&alice, tiny, sizeof(tiny))) {
    printf("Correctly detected buffer overflow (buf too small)\n");
  }

  return 0;
}
