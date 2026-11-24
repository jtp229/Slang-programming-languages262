#pragma once

#include "values.h"

/// A single key-value pair in an environment
struct EnvEntry {
  char *key;
  struct Value *val;
  struct EnvEntry *next;
};

/// The number of buckets in the environment's hash table
#define ENV_BUCKETS 128

/// An environment consists of a hash table of k/v pairs (bindings), and a
/// pointer to an enclosing outer environment.
struct Env {
  /// A pointer to the enclosing environment
  struct Env *outer;
  /// The entries in this environment / scope
  struct EnvEntry *buckets[ENV_BUCKETS];
};

/// Create a new environment
///
/// @param outer The outer environment
///
/// @return A pointer to the new environment
struct Env *make_env(struct Env *outer);

/// Create a default environment and populate it with built-in functions and
/// constants.
///
/// @return A pointer to the default environment
struct Env *make_default_env();

/// Put a key-value pair into an environment.  Updates it if the key already
/// exists.  Never recurses to an outer environment.
///
/// @param env The environment
/// @param key The key
/// @param val The value
void env_put(struct Env *env, const char *key, struct Value *val);

/// Get a value from an environment.  Recurses to parent environments as needed.
/// Populates EVAL_ERROR_MSG if the key is not found.
///
/// @param env The environment
/// @param key The key
///
/// @return The value associated with the key
struct Value *env_get(struct Env *env, const char *key);

/// Update a value in an environment.  Recurses to parent environments as
/// needed.  Never creates new entries.
///
/// @param env The environment
/// @param key The key
/// @param val The value
///
/// @return True if the value was updated, false otherwise
bool env_update(struct Env *env, const char *key, struct Value *val);

