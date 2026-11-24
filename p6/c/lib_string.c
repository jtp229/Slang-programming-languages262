// You should not need to edit this file

#include <stdlib.h>
#include <string.h>

#include "lib_helpers.h"
#include "lib_string.h"

/// A function for appending zero or more strings
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return A new value representing the concatenated string, or NULL on error
static struct Value *builtin_string_append(struct Value **args, int num_args,
                                           struct Env *env) {
  for (int i = 0; i < num_args; ++i) {
    if (args[i]->type != STR_VAL) {
      EVAL_ERROR_MSG = strdup("Run-time Error: Argument is not a string");
      return NULL;
    }
  }

  size_t total_len = 0;
  for (int i = 0; i < num_args; ++i) {
    total_len += strlen(args[i]->data.str_val);
  }

  char *res = (char *)malloc(total_len + 1);
  res[0] = '\0';
  for (int i = 0; i < num_args; ++i) {
    strcat(res, args[i]->data.str_val);
  }

  struct Value *v = alloc_value(STR_VAL);
  v->data.str_val = res;
  return v;
}

/// A function for reporting the length of a string
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return A value holding the string's length, or NULL on error
static struct Value *builtin_string_length(struct Value **args, int num_args,
                                           struct Env *env) {
  check_arg_count(num_args, 1, "string-length");
  if (EVAL_ERROR_MSG)
    return NULL;
  if (args[0]->type != STR_VAL) {
    EVAL_ERROR_MSG = strdup("string-length requires a string");
    return NULL;
  }

  struct Value *v = alloc_value(INT_VAL);
  v->data.int_val = (int)strlen(args[0]->data.str_val);
  return v;
}

/// A function for extracting a substring from a string
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return A new value representing the substring, or NULL on error
static struct Value *builtin_substring(struct Value **args, int num_args,
                                       struct Env *env) {
  check_arg_count(num_args, 3, "substring");
  if (EVAL_ERROR_MSG)
    return NULL;
  if (args[0]->type != STR_VAL || args[1]->type != INT_VAL ||
      args[2]->type != INT_VAL) {
    EVAL_ERROR_MSG = strdup("substring requires (string, int, int)");
    return NULL;
  }

  char *s = args[0]->data.str_val;
  int start = args[1]->data.int_val;
  int end = args[2]->data.int_val;
  int len = (int)strlen(s);

  if (start < 0 || start > len || end < start || end > len) {
    EVAL_ERROR_MSG = strdup("substring: out of bounds");
    return NULL;
  }

  int sublen = end - start;
  char *res = (char *)malloc(sublen + 1);
  strncpy(res, s + start, sublen);
  res[sublen] = '\0';

  struct Value *v = alloc_value(STR_VAL);
  v->data.str_val = res;
  return v;
}

/// A function for checking if a value is a string
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return A value holding true or false, or NULL on error
static struct Value *builtin_string_p(struct Value **args, int num_args,
                                      struct Env *env) {
  if (num_args != 1) {
    EVAL_ERROR_MSG = strdup("string? requires 1 argument");
    return NULL;
  }

  return alloc_value(args[0]->type == STR_VAL ? TRUE_VAL : FALSE_VAL);
}

/// Populate the environment with string-processing built-in functions
///
/// @param env The environment to populate
void make_string_funcs(struct Env *env) {
  register_builtin(env, "string-append", builtin_string_append);
  register_builtin(env, "string-length", builtin_string_length);
  register_builtin(env, "substring", builtin_substring);
  register_builtin(env, "string?", builtin_string_p);
}
