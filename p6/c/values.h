// You should not need to edit this file

#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast_nodes.h"

struct Env;

/// The node types, so that we can manually implement a tagged union
typedef enum {
  EMPTY_VAL,      ///< Empty list
  TRUE_VAL,       ///< True boolean
  FALSE_VAL,      ///< False boolean
  CHAR_VAL,       ///< Character
  DBL_VAL,        ///< Double
  INT_VAL,        ///< Integer
  STR_VAL,        ///< String
  SYMBOL_VAL,     ///< Symbol
  VEC_VAL,        ///< Vector
  BUILT_IN_FUNC_VAL, ///< Built-in function
  CONS_VAL,       ///< Cons cell
  LAMBDA_VAL,     ///< Lambda function
} ValueType;

/// A tagged union that stores any value type supported by slang
struct Value {
  /// A tag, for determining the true type of this Value
  ValueType type;

  union {
    /// The value when it's a character
    char char_val;
    /// The value when it's a double
    double dbl_val;
    /// The value when it's an integer
    int int_val;
    /// The value when it's a string
    char *str_val;
    /// The value when it's a symbol
    char *symbol_name;
    /// The value when it's a vector
    struct {
      /// An array of Values
      struct Value **items;
      /// The size of the array
      int count;
    } vec;
    /// The value when it's a built-in function
    struct {
      /// The name of the function
      char *name;
      /// A pointer to the function's code
      struct Value *(*func)(struct Value **args, int num_args, struct Env *env);
    } built_in;
    /// The value when it's a cons cell
    struct {
      /// The value of the first element (car)
      struct Value *car;
      /// The value of the second element (cdr)
      struct Value *cdr;
    } cons;
    /// The value when it's a lambda value
    struct {
      /// The captured environment
      struct Env *env;
      /// A LambdaDef node
      struct AstNode *lambda_def;
    } lambda;
  } data;
};

/// Create a string representation of a value
///
/// @param v The value to convert
///
/// @return An owned string representing the value
inline static char *value_to_string(struct Value *v) {
  if (!v)
    return strdup("NULL");

  char buffer[1024];
  switch (v->type) {
  case EMPTY_VAL:
    return strdup("()");
  case TRUE_VAL:
    return strdup("#t");
  case FALSE_VAL:
    return strdup("#f");
  case CHAR_VAL:
    if (v->data.char_val == ' ')
      return strdup("#\\space");
    if (v->data.char_val == '\n')
      return strdup("#\\newline");
    if (v->data.char_val == '\t')
      return strdup("#\\tab");
    snprintf(buffer, sizeof(buffer), "#\\%c", v->data.char_val);
    return strdup(buffer);
  case DBL_VAL:
    format_double(buffer, sizeof(buffer), v->data.dbl_val);
    return strdup(buffer);
  case INT_VAL:
    snprintf(buffer, sizeof(buffer), "%d", v->data.int_val);
    return strdup(buffer);
  case STR_VAL:
    return strdup(v->data.str_val);
  case SYMBOL_VAL:
    snprintf(buffer, sizeof(buffer), "'%s", v->data.symbol_name);
    return strdup(buffer);
  case VEC_VAL: {
    // Robust assuming at least one item or empty
    if (v->data.vec.count == 0)
      return strdup("#()");
    // This is a bit complex for a one-liner, but let's do simple concat
    char *res = strdup("#(");
    for (int i = 0; i < v->data.vec.count; ++i) {
      char *item_str = value_to_string(v->data.vec.items[i]);
      char *next_res = (char *)malloc(strlen(res) + strlen(item_str) + 2);
      sprintf(next_res, "%s%s%s", res, item_str,
              (i == v->data.vec.count - 1 ? "" : " "));
      free(res);
      free(item_str);
      res = next_res;
    }
    char *final_res = (char *)malloc(strlen(res) + 2);
    sprintf(final_res, "%s)", res);
    free(res);
    return final_res;
  }
  case BUILT_IN_FUNC_VAL:
    snprintf(buffer, sizeof(buffer), "Built-in Function: %s",
             v->data.built_in.name);
    return strdup(buffer);
  case CONS_VAL: {
    char *car_str = value_to_string(v->data.cons.car);
    char *cdr_str = value_to_string(v->data.cons.cdr);
    char *res = (char *)malloc(strlen(car_str) + strlen(cdr_str) + 10);
    sprintf(res, "(%s . %s)", car_str, cdr_str);
    free(car_str);
    free(cdr_str);
    return res;
  }
  case LAMBDA_VAL:
    return strdup("Lambda Function");
  default:
    return strdup("Unknown Value");
  }
}
