#include <stdlib.h>
#include <string.h>

#include "env.h"
#include "gc.h"
#include "lib_lists.h"
#include "lib_math.h"
#include "lib_string.h"
#include "lib_vector.h"

extern char *EVAL_ERROR_MSG;

/// Hash a string using the djb2 algorithm
///
/// @param str The string to hash
///
/// @return The hash of the string
static unsigned long hash(const char *str) {
  unsigned long hash = 5381;
  int c;
  while ((c = *str++))
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  return hash;
}

/// Create a new environment
///
/// @param outer The outer environment
///
/// @return A pointer to the new environment
struct Env *make_env(struct Env *outer) {
  struct Env *env = gc_alloc(GC_OBJ_ENV, sizeof(struct Env));
  env->outer = outer;
  return env;
}

/// Create a default environment and populate it with built-in functions and
/// constants.
///
/// @return A pointer to the default environment
struct Env *make_default_env() {
  struct Env *env = make_env(NULL);
  make_math_funcs(env);
  make_list_funcs(env);
  make_string_funcs(env);
  make_vector_funcs(env);
  return env;
}

/// Put a key-value pair into an environment.  Updates it if the key already
/// exists.  Never recurses to an outer environment.
///
/// @param env The environment
/// @param key The key
/// @param val The value
void env_put(struct Env *env, const char *key, struct Value *val) {
  unsigned long h = hash(key) % ENV_BUCKETS;
  struct EnvEntry *entry = env->buckets[h];
  // Traverse the list in this bucket
  while (entry) {
    if (strcmp(entry->key, key) == 0) {
      entry->val = val;
      return;
    }
    entry = entry->next;
  }
  // Not found: push a new entry to the front of this bucket
  entry = (struct EnvEntry *)malloc(sizeof(struct EnvEntry));
  entry->key = strdup(key);
  entry->val = val;
  entry->next = env->buckets[h];
  env->buckets[h] = entry;
}

/// Get a value from an environment.  Recurses to parent environments as needed.
/// Populates EVAL_ERROR_MSG if the key is not found.
///
/// @param env The environment
/// @param key The key
///
/// @return The value associated with the key
struct Value *env_get(struct Env *env, const char *key) {
  if (!env) {
    EVAL_ERROR_MSG = strdup("Run-time Error: Invalid identifier");
    return NULL;
  }
  struct EnvEntry *entry = env->buckets[hash(key) % ENV_BUCKETS];
  while (entry) {
    if (strcmp(entry->key, key) == 0) {
      return entry->val;
    }
    entry = entry->next;
  }
  return env_get(env->outer, key);
}

/// Update a value in an environment.  Recurses to parent environments as
/// needed.  Never creates new entries.
///
/// @param env The environment
/// @param key The key
/// @param val The value
///
/// @return True if the value was updated, false otherwise
bool env_update(struct Env *env, const char *key, struct Value *val) {
  if (!env)
    return false;
  struct EnvEntry *entry = env->buckets[hash(key) % ENV_BUCKETS];
  while (entry) {
    if (strcmp(entry->key, key) == 0) {
      entry->val = val;
      return true;
    }
    entry = entry->next;
  }
  return env_update(env->outer, key, val);
}
