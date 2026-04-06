#define LIBJSON_IMPLEMENTATION
#include "../libjson.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char name[64];
  long id;
  char email[128];
  char roles[4][32];
  size_t roles_count;
  bool active;
} User;

static bool slice_to_long(JsonSlice s, long *out) {
  char buf[32];
  char *end;

  if (!s.data || s.len == 0 || s.len >= sizeof(buf))
    return false;
  memcpy(buf, s.data, s.len);
  buf[s.len] = '\0';
  *out = strtol(buf, &end, 10);
  return end != buf && *end == '\0';
}

static bool slice_to_bool(JsonSlice s) {
  return s.data && s.len == 4 && strncmp(s.data, "true", 4) == 0;
}

static bool user_from_json(JsonSlice json, User *user) {
  JsonSlice val, roles;
  JsonArrayIter iter;
  JsonSlice item;

  memset(user, 0, sizeof(*user));

  if (!json_get(json, "name", &val))
    return false;
  json_slice_copy(val, user->name, sizeof(user->name));

  if (!json_get(json, "id", &val))
    return false;
  if (!slice_to_long(val, &user->id))
    return false;

  if (!json_get(json, "email", &val))
    return false;
  json_slice_copy(val, user->email, sizeof(user->email));

  if (!json_get(json, "active", &val))
    return false;
  user->active = slice_to_bool(val);

  if (!json_get(json, "roles", &roles))
    return false;

  json_array_iter_init(roles, &iter);
  while (json_array_iter_next(&iter, &item) && user->roles_count < 4) {
    json_slice_copy(item, user->roles[user->roles_count],
                    sizeof(user->roles[0]));
    user->roles_count++;
  }

  return true;
}

static void print_user(const User *user) {
  size_t i;

  printf("User {\n");
  printf("  name:   %s\n", user->name);
  printf("  id:     %ld\n", user->id);
  printf("  email:  %s\n", user->email);
  printf("  active: %s\n", user->active ? "true" : "false");
  printf("  roles:  [");
  for (i = 0; i < user->roles_count; i++) {
    if (i > 0)
      printf(", ");
    printf("%s", user->roles[i]);
  }
  printf("]\n}\n");
}

int main(void) {
  const char *json = "{"
                     "  \"name\": \"Alice\","
                     "  \"id\": 42,"
                     "  \"email\": \"alice@example.com\","
                     "  \"active\": true,"
                     "  \"roles\": [\"admin\", \"editor\"]"
                     "}";

  const char *json_array =
      "["
      "  {\"name\":\"Alice\",\"id\":1,\"email\":\"alice@example.com\","
      "   \"active\":true,\"roles\":[\"admin\"]},"
      "  {\"name\":\"Bob\",\"id\":2,\"email\":\"bob@example.com\","
      "   \"active\":false,\"roles\":[\"viewer\",\"editor\"]}"
      "]";

  JsonSlice root;
  User user;
  JsonArrayIter iter;
  JsonSlice item;

  /* Deserialize a single user */
  printf("=== Single User ===\n");
  root = json_from_cstr(json);
  if (user_from_json(root, &user)) {
    print_user(&user);
  } else {
    fprintf(stderr, "Failed to deserialize user\n");
    return 1;
  }

  /* Deserialize an array of users */
  printf("\n=== User Array ===\n");
  root = json_from_cstr(json_array);
  json_array_iter_init(root, &iter);
  while (json_array_iter_next(&iter, &item)) {
    if (user_from_json(item, &user)) {
      print_user(&user);
    }
  }

  return 0;
}
