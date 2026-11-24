// You should not need to edit this file

#include <stdlib.h>
#include <string.h>

#include "lib_helpers.h"
#include "lib_vector.h"

/// A function for reporting the length of a vector
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return A value holding the vector's length, or NULL on error
static struct Value *builtin_vector_length(struct Value **args, int num_args,
                                           struct Env *env) {
  check_arg_count(num_args, 1, "vector-length");
  if (EVAL_ERROR_MSG)
    return NULL;
  if (args[0]->type != VEC_VAL) {
    EVAL_ERROR_MSG = strdup("vector-length requires a vector");
    return NULL;
  }

  struct Value *v = alloc_value(INT_VAL);
  v->data.int_val = args[0]->data.vec.count;
  return v;
}

/// A function for getting an element from a vector
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return The value in the requested position, or NULL on error
static struct Value *builtin_vector_get(struct Value **args, int num_args,
                                        struct Env *env) {
  check_arg_count(num_args, 2, "vector-get");
  if (EVAL_ERROR_MSG)
    return NULL;
  if (args[0]->type != VEC_VAL || args[1]->type != INT_VAL) {
    EVAL_ERROR_MSG = strdup("Run-time Error: Argument is not a vector");
    return NULL;
  }

  struct Value *vec = args[0];
  int idx = args[1]->data.int_val;
  if (idx < 0 || idx >= vec->data.vec.count) {
    EVAL_ERROR_MSG = strdup("vector-get: out of bounds");
    return NULL;
  }

  return vec->data.vec.items[idx];
}

/// A function for setting an element in a vector
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return The value that was written, or NULL on error
static struct Value *builtin_vector_set(struct Value **args, int num_args,
                                        struct Env *env) {
  check_arg_count(num_args, 3, "vector-set!");
  if (EVAL_ERROR_MSG)
    return NULL;
  if (args[0]->type != VEC_VAL || args[1]->type != INT_VAL) {
    EVAL_ERROR_MSG = strdup("vector-set! requires (vector, int, value)");
    return NULL;
  }

  struct Value *vec = args[0];
  int idx = args[1]->data.int_val;
  if (idx < 0 || idx >= vec->data.vec.count) {
    EVAL_ERROR_MSG = strdup("vector-set!: out of bounds");
    return NULL;
  }

  vec->data.vec.items[idx] = args[2];
  return args[2];
}

/// A function for constructing a vector from a bunch of values
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return A value representing the vector, or NULL on error
static struct Value *builtin_vector(struct Value **args, int num_args,
                                    struct Env *env) {
  struct Value *v = alloc_value(VEC_VAL);
  v->data.vec.count = num_args;
  v->data.vec.items =
      (struct Value **)malloc(sizeof(struct Value *) * num_args);
  for (int i = 0; i < num_args; ++i) {
    v->data.vec.items[i] = args[i];
  }
  return v;
}

/// A function for checking if a value is a vector
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return A value holding true or false, or NULL on error
static struct Value *builtin_vector_p(struct Value **args, int num_args,
                                      struct Env *env) {
  if (num_args != 1) {
    EVAL_ERROR_MSG = strdup("vector? requires 1 argument");
    return NULL;
  }

  return alloc_value(args[0]->type == VEC_VAL ? TRUE_VAL : FALSE_VAL);
}

/// Populate the environment with vector-processing built-in functions
///
/// @param env The environment to populate
void make_vector_funcs(struct Env *env) {
  register_builtin(env, "vector-length", builtin_vector_length);
  register_builtin(env, "vector-get", builtin_vector_get);
  register_builtin(env, "vector-set!", builtin_vector_set);
  register_builtin(env, "vector", builtin_vector);
  register_builtin(env, "vector?", builtin_vector_p);
}
