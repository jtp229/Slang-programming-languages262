#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "evaluator.h"
#include "gc.h"
#include "lib_helpers.h"

char *EVAL_ERROR_MSG = NULL;

/// Convert a datum AstNode (identified by index in the arena) to a Value.  This
/// only works with BOOL, INT, DBL, CHAR, STR, SYMBOL, CONS, VEC, and EMPTY_CONS
/// types.
///
/// @param node The node to evaluate
///
/// @return The resulting Value
struct Value *ast_to_value(struct AstNode *node) {
  switch (node->type) {
  case BOOL_TRUE_DATUM: {
    return alloc_value(TRUE_VAL);
  }
  case BOOL_FALSE_DATUM: {
    return alloc_value(FALSE_VAL);
  }
  case INT_DATUM: {
    struct Value *res = alloc_value(INT_VAL);
    res->data.int_val = node->data.int_node;
    return res;
  }
  case DBL_DATUM: {
    struct Value *res = alloc_value(DBL_VAL);
    res->data.dbl_val = node->data.dbl_node;
    return res;
  }
  case CHAR_DATUM: {
    struct Value *res = alloc_value(CHAR_VAL);
    res->data.char_val = node->data.char_node;
    return res;
  }
  case STR_DATUM: {
    struct Value *res = alloc_value(STR_VAL);
    res->data.str_val = strdup(node->data.str_node);
    return res;
  }
  case SYMBOL_DATUM: {
    struct Value *res = alloc_value(SYMBOL_VAL);
    res->data.symbol_name = strdup(node->data.symbol_node);
    return res;
  }
  case EMPTY_CONS_DATUM: {
    return alloc_value(EMPTY_VAL);
  }
  case CONS_DATUM: {
    struct Value *res = alloc_value(CONS_VAL);
    res->data.cons.car = ast_to_value(node->data.cons_node.car);
    res->data.cons.cdr = ast_to_value(node->data.cons_node.cdr);
    return res;
  }
  case VEC_DATUM: {
    struct Value *res = alloc_value(VEC_VAL);
    res->data.vec.count = node->data.vec_node.count;
    res->data.vec.items = calloc(res->data.vec.count, sizeof(struct Value *));
    for (int i = 0; i < res->data.vec.count; ++i) {
      res->data.vec.items[i] = ast_to_value(node->data.vec_node.elements[i]);
    }
    return res;
  }
  default: {
    EVAL_ERROR_MSG = strdup("Cannot convert this AST type to value");
    return NULL;
  }
  }
}

/// Evaluate an AST node in the given environment, returning the resulting
/// runtime Value.
///
/// @param expr The node to evaluate
/// @param env  The current environment
///
/// @return The result of evaluation, or NULL if an error occurred (check
///         EVAL_ERROR_MSG)
struct Value *evaluate(struct AstNode *expr, struct Env *env) {
  // The result of evaluating `expr`
  struct Value *result = NULL;

  switch (expr->type) {
  case IDENTIFIER_NODE: {
    result = env_get(env, expr->data.identifier_node);
    break;
  }
  case DEFINE_VAR_NODE: {
    result = evaluate(expr->data.define_var_node.expr, env);
    if (result == NULL)
      break;
    const char *name = expr->data.define_var_node.id->data.identifier_node;
    env_put(env, name, result);
    break;
  }
  case SET_NODE: {
    result = evaluate(expr->data.set_node.expr, env);
    if (result == NULL)
      break;
    const char *name = expr->data.set_node.id->data.identifier_node;
    if (!env_update(env, name, result)) {
      EVAL_ERROR_MSG = strdup("Cannot set unbound variable");
      result = NULL;
    }
    break;
  }
  case BOOL_TRUE_DATUM:
  case BOOL_FALSE_DATUM:
  case INT_DATUM:
  case DBL_DATUM:
  case CHAR_DATUM:
  case STR_DATUM:
  case SYMBOL_DATUM:
  case EMPTY_CONS_DATUM:
  case CONS_DATUM:
  case VEC_DATUM: {
    result = ast_to_value(expr);
    break;
  }
  case IF_NODE: {
    struct Value *cond = evaluate(expr->data.if_node.cond, env);
    if (cond == NULL)
      break;
    if (cond->type != FALSE_VAL)
      result = evaluate(expr->data.if_node.if_true, env);
    else
      result = evaluate(expr->data.if_node.if_false, env);
    break;
  }
  case AND_NODE: {
    struct AstNodeVec *exprs = &expr->data.and_node;
    if (exprs->count == 0) {
      result = alloc_value(TRUE_VAL);
      break;
    }
    for (int i = 0; i < exprs->count; ++i) {
      result = evaluate(exprs->elements[i], env);
      // NB: Short-circuit evaluation when false
      if (result == NULL || result->type == FALSE_VAL)
        break;
    }
    break;
  }
  case OR_NODE: {
    struct AstNodeVec *exprs = &expr->data.or_node;
    if (exprs->count == 0) {
      result = alloc_value(FALSE_VAL);
      break;
    }
    for (int i = 0; i < exprs->count; ++i) {
      result = evaluate(exprs->elements[i], env);
      // NB: Short-circuit evaluation when not false
      if (result == NULL || result->type != FALSE_VAL)
        break;
    }
    break;
  }
  case BEGIN_NODE: {
    struct AstNodeVec *exprs = &expr->data.begin_node;
    if (exprs->count == 0) {
      result = alloc_value(EMPTY_VAL);
      break;
    }
    for (int i = 0; i < exprs->count; ++i) {
      result = evaluate(exprs->elements[i], env);
      if (result == NULL)
        break;
    }
    break;
  }
  case CALL_NODE: {
    // Evaluation of a function call: (func arg1 arg2 ...)
    struct AstNodeVec *exprs = &expr->data.call_node;
    // Make sure the callee is valid
    struct Value *func = evaluate(exprs->elements[0], env);
    if (func == NULL)
      break;
    if (func->type != BUILT_IN_FUNC_VAL && func->type != LAMBDA_VAL) {
      EVAL_ERROR_MSG = strdup("Operator is not a procedure");
      break;
    }

    // Evaluate operands
    int arg_count = exprs->count - 1;
    struct Value **args = calloc(exprs->count, sizeof(struct Value *));
    bool ok = true;
    for (int i = 0; i < arg_count; ++i) {
      args[i] = evaluate(exprs->elements[i + 1], env);
      if (args[i] == NULL) {
        ok = false;
        break;
      }
    }

    if (ok && func->type == BUILT_IN_FUNC_VAL) {
      // Evaluate a built-in function
      result = func->data.built_in.func(args, arg_count, env);
    } else if (ok) {
      // Bind arguments in a new environment and then recurse to interpret a
      // lambda body
      struct AstNode *def = func->data.lambda.lambda_def;
      struct AstNodeVec *formals = &def->data.lambda_def_node.formals;
      if (arg_count != formals->count) {
        EVAL_ERROR_MSG = strdup("Wrong number of arguments to lambda");
      } else {
        struct Env *inner = make_env(func->data.lambda.env);
        for (int i = 0; i < arg_count; ++i) {
          const char *pname = formals->elements[i]->data.identifier_node;
          env_put(inner, pname, args[i]);
        }

        struct AstNodeVec *body = &def->data.lambda_def_node.body;
        if (body->count == 0)
          result = alloc_value(EMPTY_VAL);
        for (int i = 0; i < body->count; ++i) {
          result = evaluate(body->elements[i], inner);
          if (result == NULL)
            break;
        }
      }
    }
    free(args);
    break;
  }
  case DEFINE_FUNC_NODE: {
    // The AstNodes for the ids, including the function name
    struct AstNodeVec *ids = &expr->data.define_func_node.ids;
    // The AstNodes for the body of the function
    struct AstNodeVec *body = &expr->data.define_func_node.body;
    // The function name
    const char *name_str = ids->elements[0]->data.identifier_node;

    // We need to make a LAMBDA_DEF_NODE!
    struct AstNode *lambda_node = alloc_astnode(LAMBDA_DEF_NODE);
    int formal_count = ids->count - 1;
    // Copy the Ids (but not the name) to the lambda's formals
    lambda_node->data.lambda_def_node.formals.count = formal_count;
    lambda_node->data.lambda_def_node.formals.capacity = formal_count;
    lambda_node->data.lambda_def_node.formals.elements =
        malloc(formal_count * sizeof(struct AstNode));
    if (formal_count > 0) {
      memcpy(lambda_node->data.lambda_def_node.formals.elements,
             &ids->elements[1], formal_count * sizeof(struct AstNode *));
    }
    // Copy the body to the lambda's body
    lambda_node->data.lambda_def_node.body.count = body->count;
    lambda_node->data.lambda_def_node.body.capacity = body->count;
    lambda_node->data.lambda_def_node.body.elements =
        malloc(body->count * sizeof(struct AstNode *));
    if (body->count > 0) {
      memcpy(lambda_node->data.lambda_def_node.body.elements, body->elements,
             body->count * sizeof(struct AstNode *));
    }


    // Now make the lambda value and put it in the environment
    result = alloc_value(LAMBDA_VAL);
    result->data.lambda.env = make_env(env);
    result->data.lambda.lambda_def = lambda_node;
    env_put(env, name_str, result);
    break;
  }
  case COND_NODE: {
    struct AstNodeVec *conditions = &expr->data.cond_node;
    bool matched = false;
    for (int i = 0; i < conditions->count && !matched; ++i) {
      struct AstNode *cond_obj = conditions->elements[i];
      struct Value *test_val = evaluate(cond_obj->data.condition_obj.test, env);
      if (test_val == NULL)
        break;
      if (test_val->type != FALSE_VAL) {
        matched = true;
        struct AstNodeVec *exprs = &cond_obj->data.condition_obj.exprs;
        if (exprs->count == 0)
          result = alloc_value(EMPTY_VAL);
        for (int j = 0; j < exprs->count; ++j) {
          result = evaluate(exprs->elements[j], env);
          if (result == NULL)
            break;
        }
      }
    }
    if (!matched && EVAL_ERROR_MSG == NULL)
      EVAL_ERROR_MSG = strdup("All cond arms evaluated false");
    break;
  }
  case LET_NODE: {
    struct Env *inner = make_env(env);
    bool ok = true;
    for (int i = 0; i < expr->data.let_node.vars.count; ++i) {
      struct AstNode *let_def = expr->data.let_node.vars.elements[i];
      struct Value *v = evaluate(let_def->data.let_def_obj.val, env);
      if (v == NULL) {
        ok = false;
        break;
      }
      const char *name = let_def->data.let_def_obj.id->data.identifier_node;
      env_put(inner, name, v);
    }
    if (ok && expr->data.let_node.body.count == 0)
      result = alloc_value(EMPTY_VAL);
    for (int i = 0; ok && i < expr->data.let_node.body.count; ++i) {
      result = evaluate(expr->data.let_node.body.elements[i], inner);
      if (result == NULL)
        break;
    }
    break;
  }
  case LAMBDA_DEF_NODE: {
    result = alloc_value(LAMBDA_VAL);
    result->data.lambda.env = make_env(env);
    result->data.lambda.lambda_def = expr;
    break;
  }
  case QUOTE_NODE:
    result = evaluate(expr->data.quote_node, env);
    break;
  case TICK_NODE:
    result = evaluate(expr->data.tick_node, env);
    break;
  default:
    EVAL_ERROR_MSG = strdup("Unknown AST node type");
    break;
  }
  return result;
}

/// Allocate a new garbage collected Value
///
/// @param type The type of value to allocate
///
/// @return A pointer to the newly allocated Value
struct Value *alloc_value(ValueType type) {
  struct Value *v = gc_alloc(GC_OBJ_VALUE, sizeof(struct Value));
  v->type = type;
  return v;
}
