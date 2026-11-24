#include <stdlib.h>

#include "lib_helpers.h"
#include "lib_lists.h"

/// A function for getting the first element of a list
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return The first element of the list, or NULL on error
static struct Value *builtin_car(struct Value **args, int num_args,
                                 struct Env *env) {
  check_arg_count(num_args, 1, "car");
  if (EVAL_ERROR_MSG)
    return NULL;
  if (args[0]->type == EMPTY_VAL) {
    EVAL_ERROR_MSG = strdup("car: Pair expected");
    return NULL;
  }
  if (args[0]->type != CONS_VAL) {
    EVAL_ERROR_MSG = strdup("Run-time Error: Argument is not a cons cell");
    return NULL;
  }
  return args[0]->data.cons.car;
}

/// A function for getting the rest of a list
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return The rest of the list, or NULL on error
static struct Value *builtin_cdr(struct Value **args, int num_args,
                                 struct Env *env) {
  check_arg_count(num_args, 1, "cdr");
  if (EVAL_ERROR_MSG)
    return NULL;
  if (args[0]->type == EMPTY_VAL) {
    EVAL_ERROR_MSG = strdup("cdr: Pair expected");
    return NULL;
  }
  if (args[0]->type != CONS_VAL) {
    EVAL_ERROR_MSG = strdup("cdr: Argument is not a Cons cell");
    return NULL;
  }
  return args[0]->data.cons.cdr;
}

/// A function for constructing a new list cell
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return A new value representing the list cell, or NULL on error
static struct Value *builtin_cons(struct Value **args, int num_args,
                                  struct Env *env) {
  check_arg_count(num_args, 2, "cons");
  if (EVAL_ERROR_MSG)
    return NULL;
  struct Value *v = alloc_value(CONS_VAL);
  v->data.cons.car = args[0];
  v->data.cons.cdr = args[1];
  return v;
}

/// A helper function for recursively constructing a list from an array of
/// values
///
/// @param items The array of values
/// @param count The number of values in the array
/// @param start The index of the first value to process
///
/// @return A new value representing the list
static struct Value *make_cons_list_recursive(struct Value **items, int count,
                                              int start) {
  if (start >= count)
    return alloc_value(EMPTY_VAL);

  // Build the tail first, then the cell that points to it
  struct Value *tail = make_cons_list_recursive(items, count, start + 1);
  struct Value *head = alloc_value(CONS_VAL);
  head->data.cons.car = items[start];
  head->data.cons.cdr = tail;
  return head;
}

/// A function for constructing a list from a bunch of values
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return A value representing the list, or NULL on error
static struct Value *builtin_list(struct Value **args, int num_args,
                                  struct Env *env) {
  return make_cons_list_recursive(args, num_args, 0);
}

/// A function for checking if a value is a list
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return A value holding true or false, or NULL on error
static struct Value *builtin_list_p(struct Value **args, int num_args,
                                    struct Env *env) {
  check_arg_count(num_args, 1, "list?");
  if (EVAL_ERROR_MSG)
    return NULL;
  struct Value *arg = args[0];
  if (arg->type == EMPTY_VAL)
    return alloc_value(TRUE_VAL);
  if (arg->type != CONS_VAL)
    return alloc_value(FALSE_VAL);
  struct Value *cdr = arg->data.cons.cdr;
  if (cdr->type == EMPTY_VAL || cdr->type == CONS_VAL)
    return alloc_value(TRUE_VAL);
  return alloc_value(FALSE_VAL);
}

/// A function for checking if a value is the empty list
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return A value holding true or false, or NULL on error
static struct Value *builtin_null_p(struct Value **args, int num_args,
                                    struct Env *env) {
  if (num_args != 1) {
    EVAL_ERROR_MSG = strdup("null? requires 1 argument");
    return NULL;
  }
  return alloc_value(args[0]->type == EMPTY_VAL ? TRUE_VAL : FALSE_VAL);
}

/// A function for setting the first element of a list
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return The value that was written, or NULL on error
static struct Value *builtin_set_car_bang(struct Value **args, int num_args,
                                          struct Env *env) {
  check_arg_count(num_args, 2, "set-car!");
  if (EVAL_ERROR_MSG)
    return NULL;
  if (args[0]->type != CONS_VAL) {
    EVAL_ERROR_MSG = strdup("set-car!: Argument is not a Cons cell");
    return NULL;
  }
  args[0]->data.cons.car = args[1];
  return args[1];
}

/// A function for setting the rest of a list
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return The value that was written, or NULL on error
static struct Value *builtin_set_cdr_bang(struct Value **args, int num_args,
                                          struct Env *env) {
  check_arg_count(num_args, 2, "set-cdr!");
  if (EVAL_ERROR_MSG)
    return NULL;
  if (args[0]->type != CONS_VAL) {
    EVAL_ERROR_MSG = strdup("set-cdr!: Argument is not a Cons cell");
    return NULL;
  }
  args[0]->data.cons.cdr = args[1];
  return args[1];
}

/// Populate the environment with list-processing built-in functions
///
/// @param env The environment to populate
void make_list_funcs(struct Env *env) {
  register_builtin(env, "car", builtin_car);
  register_builtin(env, "cdr", builtin_cdr);
  register_builtin(env, "cons", builtin_cons);
  register_builtin(env, "list", builtin_list);
  register_builtin(env, "list?", builtin_list_p);
  register_builtin(env, "set-car!", builtin_set_car_bang);
  register_builtin(env, "set-cdr!", builtin_set_cdr_bang);
  register_builtin(env, "null?", builtin_null_p);
}
