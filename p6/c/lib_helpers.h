#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "env.h"
#include "evaluator.h"
#include "values.h"

extern char *EVAL_ERROR_MSG;

/// Check if the number of arguments is exactly as expected, and set an error if
/// it's not.
///
/// @param count    Actual number of arguments
/// @param expected Expected number of arguments
/// @param name     Name of the function, for error reporting
static inline void check_arg_count(int count, int expected, const char *name) {
  if (count != expected) {
    EVAL_ERROR_MSG = strdup("Run-time Error: Wrong number of arguments");
  }
}

/// Check if the number of arguments is at least some value, and set an error if
/// it's not
///
/// @param count  The number of arguments
/// @param min    The minimum number of arguments
/// @param name   Name of the function, for error reporting
static inline void ensure_arg_count_at_least(int count, int min,
                                             const char *name) {
  if (count < min) {
    char buf[128];
    snprintf(buf, sizeof(buf), "%s requires at least %d argument(s)", name,
             min);
    EVAL_ERROR_MSG = strdup(buf);
  }
}

/// Check if a value's type matches the given type
///
/// @param v      The value whose type needs to be checked
/// @param type   The expected type
/// @param errmsg The error to generate on failure
static inline void ensure_arg_type(struct Value *v, ValueType type,
                                   const char *errmsg) {
  if (v->type != type)
    EVAL_ERROR_MSG = strdup(errmsg);
}

/// Check if a value's type is one of the two given types
///
/// @param v      The value whose type needs to be checked
/// @param t1     The first option for the type
/// @param t2     The second option for the type
/// @param errmsg The error to generate on failure
static inline void ensure_arg_type_in2(struct Value *v, ValueType t1,
                                       ValueType t2, const char *errmsg) {
  if (v->type != t1 && v->type != t2)
    EVAL_ERROR_MSG = strdup(errmsg);
}

/// Register a built-in function by putting it in the environment
///
/// @param env  The environment into which the function should be placed
/// @param name The desired name for the function
/// @param func The C function that does the work of the built-in function
static inline void register_builtin(struct Env *env, const char *name,
                                    struct Value *(*func)(struct Value **, int,
                                                          struct Env *)) {
  struct Value *v = alloc_value(BUILT_IN_FUNC_VAL);
  v->data.built_in.name = strdup(name);
  v->data.built_in.func = func;
  env_put(env, name, v);
}
