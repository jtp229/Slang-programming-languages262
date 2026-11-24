// You should not need to edit this file

#include <math.h>
#include <stdlib.h>

#include "lib_helpers.h"
#include "lib_math.h"

/// A helper for getting a double value from an Int or Dbl value
///
/// @param v The value to convert
///
/// @return The double representation of the value
static double get_double(struct Value *v) {
  return (v->type == INT_VAL) ? (double)v->data.int_val : v->data.dbl_val;
}

/// A helper for implementing variable-arity math functions
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
/// @param name     The name of the function, for error reporting
/// @param opd      The operation to perform on doubles or mixed types
/// @param opi      The operation to perform on integers
///
/// @return A new value representing the result of the operation, or NULL on
/// error
static struct Value *math_op_var(struct Value **args, int num_args,
                                 struct Env *env, const char *name,
                                 double (*opd)(double, double),
                                 int (*opi)(int, int)) {
  ensure_arg_count_at_least(num_args, 1, name);
  if (EVAL_ERROR_MSG)
    return NULL;

  bool any_dbl = false;
  for (int i = 0; i < num_args; ++i) {
    ensure_arg_type_in2(args[i], INT_VAL, DBL_VAL,
                        "Run-time Error: Argument is not a number");
    if (EVAL_ERROR_MSG)
      return NULL;
    if (args[i]->type == DBL_VAL)
      any_dbl = true;
  }

  if (any_dbl) {
    double res = get_double(args[0]);
    for (int i = 1; i < num_args; ++i) {
      res = opd(res, get_double(args[i]));
    }
    struct Value *v = alloc_value(DBL_VAL);
    v->data.dbl_val = res;
    return v;
  } else {
    int res = args[0]->data.int_val;
    for (int i = 1; i < num_args; ++i) {
      res = opi(res, args[i]->data.int_val);
    }
    struct Value *v = alloc_value(INT_VAL);
    v->data.int_val = res;
    return v;
  }
}

/// A function for adding two doubles
///
/// @param a The first operand
/// @param b The second operand
/// @return The sum of a and b
static double op_add(double a, double b) { return a + b; }

/// A function for adding two integers
///
/// @param a The first operand
/// @param b The second operand
/// @return The sum of a and b
static int opi_add(int a, int b) { return a + b; }

/// A function for subtracting two doubles
///
/// @param a The first operand (minuend)
/// @param b The second operand (subtrahend)
/// @return The difference of a minus b
static double op_sub(double a, double b) { return a - b; }

/// A function for subtracting two integers
///
/// @param a The first operand (minuend)
/// @param b The second operand (subtrahend)
/// @return The difference of a minus b
static int opi_sub(int a, int b) { return a - b; }

/// A function for multiplying two doubles
///
/// @param a The first operand
/// @param b The second operand
/// @return The product of a and b
static double op_mul(double a, double b) { return a * b; }

/// A function for multiplying two integers
///
/// @param a The first operand
/// @param b The second operand
/// @return The product of a and b
static int opi_mul(int a, int b) { return a * b; }

/// A function for dividing two doubles
///
/// @param a The dividend
/// @param b The divisor
/// @return The quotient of a divided by b
static double op_div(double a, double b) { return a / b; }

/// A function for dividing two integers
///
/// @param a The dividend
/// @param b The divisor
/// @return The quotient of a divided by b
static int opi_div(int a, int b) { return a / b; }

/// A built-in function for adding numbers
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return A new value representing the sum, or NULL on error
static struct Value *builtin_add(struct Value **args, int num_args,
                                 struct Env *env) {
  return math_op_var(args, num_args, env, "+", op_add, opi_add);
}

/// A built-in function for subtracting numbers
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return A new value representing the difference, or NULL on error
static struct Value *builtin_sub(struct Value **args, int num_args,
                                 struct Env *env) {
  return math_op_var(args, num_args, env, "-", op_sub, opi_sub);
}

/// A built-in function for multiplying numbers
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return A new value representing the product, or NULL on error
static struct Value *builtin_mul(struct Value **args, int num_args,
                                 struct Env *env) {
  return math_op_var(args, num_args, env, "*", op_mul, opi_mul);
}

/// A built-in function for dividing numbers
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return A new value representing the quotient, or NULL on error
static struct Value *builtin_div(struct Value **args, int num_args,
                                 struct Env *env) {
  return math_op_var(args, num_args, env, "/", op_div, opi_div);
}

/// A built-in function for calculating the remainder of integer division
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return A new value representing the remainder, or NULL on error
static struct Value *builtin_mod(struct Value **args, int num_args,
                                 struct Env *env) {
  check_arg_count(num_args, 2, "%");
  if (EVAL_ERROR_MSG)
    return NULL;
  ensure_arg_type_in2(args[0], INT_VAL, DBL_VAL,
                      "% requires numeric arguments");
  ensure_arg_type_in2(args[1], INT_VAL, DBL_VAL,
                      "% requires numeric arguments");
  if (EVAL_ERROR_MSG)
    return NULL;

  bool any_dbl = args[0]->type == DBL_VAL || args[1]->type == DBL_VAL;

  int a = (args[0]->type == INT_VAL) ? args[0]->data.int_val
                                     : (int)args[0]->data.dbl_val;
  int b = (args[1]->type == INT_VAL) ? args[1]->data.int_val
                                     : (int)args[1]->data.dbl_val;

  int res = a % b;

  if (any_dbl) {
    struct Value *v = alloc_value(DBL_VAL);
    v->data.dbl_val = (double)res;
    return v;
  } else {
    struct Value *v = alloc_value(INT_VAL);
    v->data.int_val = res;
    return v;
  }
}

/// A helper for implementing variable-arity comparison functions
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
/// @param name     The name of the function, for error reporting
/// @param compd    The comparison to perform on doubles or mixed types
/// @param compi    The comparison to perform on integers
///
/// @return A value holding true or false, or NULL on error
static struct Value *math_comp_var(struct Value **args, int num_args,
                                   struct Env *env, const char *name,
                                   bool (*compd)(double, double),
                                   bool (*compi)(int, int)) {
  ensure_arg_count_at_least(num_args, 1, name);
  if (EVAL_ERROR_MSG)
    return NULL;

  bool any_dbl = false;
  for (int i = 0; i < num_args; ++i) {
    ensure_arg_type_in2(args[i], INT_VAL, DBL_VAL,
                        "numerical comparison requires numbers");
    if (EVAL_ERROR_MSG)
      return NULL;
    if (args[i]->type == DBL_VAL)
      any_dbl = true;
  }

  if (any_dbl) {
    for (int i = 0; i < num_args - 1; ++i) {
      if (!compd(get_double(args[i]), get_double(args[i + 1]))) {
        return alloc_value(FALSE_VAL);
      }
    }
  } else {
    for (int i = 0; i < num_args - 1; ++i) {
      if (!compi(args[i]->data.int_val, args[i + 1]->data.int_val)) {
        return alloc_value(FALSE_VAL);
      }
    }
  }
  return alloc_value(TRUE_VAL);
}

/// A function for checking if two doubles are equal
///
/// @param a  The first value to compare
/// @param b  The second value to compare
///
/// @return True if a and b are equal, false otherwise
static bool comp_eq(double a, double b) { return a == b; }

/// A function for checking if two integers are equal
///
/// @param a  The first value to compare
/// @param b  The second value to compare
///
/// @return True if a and b are equal, false otherwise
static bool compi_eq(int a, int b) { return a == b; }

/// A function for checking if one double is less than another
///
/// @param a  The first value to compare
/// @param b  The second value to compare
///
/// @return True if a is less than b, false otherwise
static bool comp_lt(double a, double b) { return a < b; }

/// A function for checking if one integer is less than another
///
/// @param a  The first value to compare
/// @param b  The second value to compare
///
/// @return True if a is less than b, false otherwise
static bool compi_lt(int a, int b) { return a < b; }

/// A function for checking if one double is less than or equal to another
///
/// @param a  The first value to compare
/// @param b  The second value to compare
///
/// @return True if a is less than or equal to b, false otherwise
static bool comp_le(double a, double b) { return a <= b; }

/// A function for checking if one integer is less than or equal to another
///
/// @param a  The first value to compare
/// @param b  The second value to compare
///
/// @return True if a is less than or equal to b, false otherwise
static bool compi_le(int a, int b) { return a <= b; }

/// A function for checking if one double is greater than another
///
/// @param a  The first value to compare
/// @param b  The second value to compare
///
/// @return True if a is greater than b, false otherwise
static bool comp_gt(double a, double b) { return a > b; }

/// A function for checking if one integer is greater than another
///
/// @param a  The first value to compare
/// @param b  The second value to compare
///
/// @return True if a is greater than b, false otherwise
static bool compi_gt(int a, int b) { return a > b; }

/// A function for checking if one double is greater than or equal to another
///
/// @param a  The first value to compare
/// @param b  The second value to compare
///
/// @return True if a is greater than or equal to b, false otherwise
static bool comp_ge(double a, double b) { return a >= b; }

/// A function for checking if one integer is greater than or equal to another
///
/// @param a  The first value to compare
/// @param b  The second value to compare
///
/// @return True if a is greater than or equal to b, false otherwise
static bool compi_ge(int a, int b) { return a >= b; }

/// A built-in function for checking if numbers are equal
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return A value holding true or false, or NULL on error
static struct Value *builtin_eq(struct Value **args, int num_args,
                                struct Env *env) {
  return math_comp_var(args, num_args, env, "=", comp_eq, compi_eq);
}

/// A built-in function for checking if numbers are strictly increasing
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return A value holding true or false, or NULL on error
static struct Value *builtin_lt(struct Value **args, int num_args,
                                struct Env *env) {
  return math_comp_var(args, num_args, env, "<", comp_lt, compi_lt);
}

/// A built-in function for checking if numbers are non-decreasing
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return A value holding true or false, or NULL on error
static struct Value *builtin_le(struct Value **args, int num_args,
                                struct Env *env) {
  return math_comp_var(args, num_args, env, "<=", comp_le, compi_le);
}

/// A built-in function for checking if numbers are strictly decreasing
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return A value holding true or false, or NULL on error
static struct Value *builtin_gt(struct Value **args, int num_args,
                                struct Env *env) {
  return math_comp_var(args, num_args, env, ">", comp_gt, compi_gt);
}

/// A built-in function for checking if numbers are non-increasing
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return A value holding true or false, or NULL on error
static struct Value *builtin_ge(struct Value **args, int num_args,
                                struct Env *env) {
  return math_comp_var(args, num_args, env, ">=", comp_ge, compi_ge);
}

/// A built-in function for calculating the absolute value of a number
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return A new value representing the absolute value, or NULL on error
static struct Value *builtin_abs(struct Value **args, int num_args,
                                 struct Env *env) {
  check_arg_count(num_args, 1, "abs");
  if (EVAL_ERROR_MSG)
    return NULL;
  if (args[0]->type == INT_VAL) {
    struct Value *v = alloc_value(INT_VAL);
    v->data.int_val = abs(args[0]->data.int_val);
    return v;
  } else if (args[0]->type == DBL_VAL) {
    struct Value *v = alloc_value(DBL_VAL);
    v->data.dbl_val = fabs(args[0]->data.dbl_val);
    return v;
  }
  EVAL_ERROR_MSG = strdup("Run-time Error: Argument is not a number");
  return NULL;
}

/// A built-in function for logical negation
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return A value holding true or false, or NULL on error
static struct Value *builtin_not(struct Value **args, int num_args,
                                 struct Env *env) {
  if (num_args != 1) {
    EVAL_ERROR_MSG = strdup("not requires 1 argument");
    return NULL;
  }
  return alloc_value(args[0]->type == FALSE_VAL ? TRUE_VAL : FALSE_VAL);
}

/// A built-in function that prints a value's textual representation (no
/// trailing newline), and returns the value itself.
///
/// @param args     The arguments to the function
/// @param num_args The size of the args array
/// @param env      The environment in which to evaluate the function
///
/// @return The displayed value, unchanged
static struct Value *builtin_display(struct Value **args, int num_args,
                                      struct Env *env) {
  if (num_args != 1) {
    EVAL_ERROR_MSG = strdup("display requires 1 argument");
    return NULL;
  }
  char *s = value_to_string(args[0]);
  printf("%s", s);
  free(s);
  return args[0];
}

/// Populate the environment with math built-in functions
///
/// @param env The environment to populate
void make_math_funcs(struct Env *env) {
  register_builtin(env, "+", builtin_add);
  register_builtin(env, "-", builtin_sub);
  register_builtin(env, "*", builtin_mul);
  register_builtin(env, "/", builtin_div);
  register_builtin(env, "%", builtin_mod);
  register_builtin(env, "=", builtin_eq);
  register_builtin(env, "<", builtin_lt);
  register_builtin(env, "<=", builtin_le);
  register_builtin(env, ">", builtin_gt);
  register_builtin(env, ">=", builtin_ge);
  register_builtin(env, "abs", builtin_abs);
  register_builtin(env, "not", builtin_not);
  register_builtin(env, "display", builtin_display);
}
