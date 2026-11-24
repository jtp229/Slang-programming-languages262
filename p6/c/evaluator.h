#pragma once

#include "ast_nodes.h"
#include "env.h"
#include "values.h"

/// A global error message to display if the evaluator cannot evaluate an
/// expression.
extern char *EVAL_ERROR_MSG;

/// Evaluate an AST node in the given environment, returning the resulting
/// runtime Value. This function implements tail-call optimization by using
/// an iterative approach for recursive calls in tail position.
///
/// @param node   The node to evaluate
/// @param env    The current environment
///
/// @return The result of evaluation, or NULL if an error occurred (check
///         EVAL_ERROR_MSG)
struct Value *evaluate(struct AstNode *node, struct Env *env);

/// Convert a datum AstNode (identified by index in the arena) to a Value.  This
/// only works with BOOL, INT, DBL, CHAR, STR, SYMBOL, CONS, VEC, and EMPTY_CONS
/// types.
///
/// @param node The node to evaluate
///
/// @return The resulting Value
struct Value *ast_to_value(struct AstNode *node);

/// Allocate a new Value
///
/// @param type The type of value to allocate
///
/// @return A pointer to the newly allocated Value
struct Value *alloc_value(ValueType type);