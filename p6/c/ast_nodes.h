#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gc.h"

/// Format a double using the shortest representation that reads back as the
/// same value (like Java, Python, Rust, etc.), rather than printf's default of
/// six significant digits.
///
/// @param buf  The buffer to write into
/// @param len  The size of buf (at least 32)
/// @param d    The value to format
static inline void format_double(char *buf, size_t len, double d) {
  for (int prec = 1; prec <= 17; ++prec) {
    snprintf(buf, len, "%.*g", prec, d);
    if (strtod(buf, NULL) == d)
      return;
  }
}

/// An enum for the types that an AstNode can be
typedef enum {
  // Datum Types
  BOOL_FALSE_DATUM,
  BOOL_TRUE_DATUM,
  CHAR_DATUM,
  CONS_DATUM,
  EMPTY_CONS_DATUM,
  DBL_DATUM,
  INT_DATUM,
  STR_DATUM,
  SYMBOL_DATUM,
  VEC_DATUM,
  // Non-Datum Nodes
  AND_NODE,
  CALL_NODE,
  BEGIN_NODE,
  COND_NODE,
  DEFINE_VAR_NODE,
  DEFINE_FUNC_NODE,
  IDENTIFIER_NODE,
  IF_NODE,
  LAMBDA_DEF_NODE,
  OR_NODE,
  QUOTE_NODE,
  SET_NODE,
  TICK_NODE,
  LET_NODE,
  // Types that aren't really AstNodes, but that we want to be able to store in
  // the AstNode arena
  CONDITION_OBJ,
  LET_DEF_OBJ,
} NodeType;

struct AstNode;
/// An array of AstNode* elements
struct AstNodeVec {
  /// The actual AstNode* array
  struct AstNode **elements;
  /// The number of valid entries in the array
  int count;
  /// The max size of the array
  int capacity;
};

/// Push an AstNode* into an AstNodeVec.  This handles growing the vector when
/// it's full
///
/// @param v    The vector into which a value should be pushed
/// @param node The pointer to push into the vector
inline static void AstNodeVec_push(struct AstNodeVec *v, struct AstNode *node) {
  if (v->count == v->capacity) {
    v->capacity *= 2;
    v->elements = (struct AstNode **)realloc(
        v->elements, v->capacity * sizeof(struct AstNode *));
  }
  v->elements[v->count++] = node;
}

/// An enum for the types that can be assigned to an AstNode
typedef enum {
  TYPE_RUNTIME,
  TYPE_INT,
  TYPE_DBL,
  TYPE_BOOL,
  TYPE_CHAR,
  TYPE_STR,
  TYPE_SYMBOL,
  TYPE_CONS,
  TYPE_VEC,
  TYPE_LAMBDA,
  TYPE_VOID
} TypeDecoration;

/// AstNode is a "type-safe" union of all the possible nodes that can go in the
/// AST.
struct AstNode {
  /// The "type safety" of this union comes from using this type to know what is
  /// stored in this struct.
  NodeType type;

  /// The static type of the node
  TypeDecoration datum_type;

  /// A union of all possible sub-types
  ///
  /// NB: Not all types require fields in this union.  We simply don't use the
  ///     union when `type` is one of those (BOOL_FALSE, BOOL_TRUE, EMPTY_CONS)
  union {
    /// CHAR_NODE is just a char
    char char_node;

    /// CONS_NODE is a pair of AstNodes
    struct {
      /// The index of the `car` field of the cons cell.  Should only be of
      /// Datum types.
      struct AstNode *car;
      /// The index of the `cdr` field of the cons cell.  Should only be of
      /// Datum types.
      struct AstNode *cdr;
    } cons_node;

    /// DBL_NODE is just a double
    double dbl_node;

    /// INT_NODE is just an int
    int int_node;

    /// STR_NODE is just a string
    char *str_node;

    /// SYMBOL_NODE is just a string
    char *symbol_node;

    /// VEC_NODE is an array of AstNodes (actually only Datum are allowed)
    struct AstNodeVec vec_node;

    /// AND_NODE is an single array of expressions
    struct AstNodeVec and_node;

    /// BEGIN_NODE is an single array of expressions
    struct AstNodeVec begin_node;

    /// CALL_NODE is an single array of expressions
    struct AstNodeVec call_node;

    /// OR_NODE is an single array of expressions
    struct AstNodeVec or_node;

    /// Cond nodes are just vectors of conditions
    struct AstNodeVec cond_node;

    /// DEFINE_VAR_NODE needs an identifier and an expression
    struct {
      /// The identifier (actually only IDENTIFIER_NODE is allowed)
      struct AstNode *id;
      /// The expression to bind to that identifier
      struct AstNode *expr;
    } define_var_node;

    /// DEFINE_FUNC_NODE has an array of identifiers (formal arguments) and a
    /// body of expressions to evaluate
    struct {
      /// The identifiers (actually only IDENTIFIER_NODE is allowed)
      struct AstNodeVec ids;
      /// The expressions that make up the body of the function
      struct AstNodeVec body;
    } define_func_node;

    /// IDENTIFIER_NODE is just a string name
    char *identifier_node;

    /// IF_NODE is just three AstNodes
    struct {
      /// The test condition
      struct AstNode *cond;
      /// The expression if the condition is not false
      struct AstNode *if_true;
      /// The expression if the condition is false
      struct AstNode *if_false;
    } if_node;

    /// LAMBDA_DEF_NODE is a bunch of identifiers and then a body
    struct {
      /// The arguments (just identifiers)
      struct AstNodeVec formals;
      /// The body to evaluate
      struct AstNodeVec body;
    } lambda_def_node;

    /// QUOTE_NODE should only ever hold a datum
    struct AstNode *quote_node;

    /// LET_NODE has a vector of name/value bindings and a body of expressions
    /// to evaluate.
    struct {
      /// Name/value bindings (only LetDefObj allowed)
      struct AstNodeVec vars;
      /// Expressions to evaluate
      struct AstNodeVec body;
    } let_node;


    /// SET_NODE has an identifier and an expression that produces a value
    struct {
      /// The identifier whose value should be updated
      struct AstNode *id;
      /// The expression that produces a value
      struct AstNode *expr;
    } set_node;

    /// TICK_NODE should only ever hold a datum
    struct AstNode *tick_node;

    /// A CONDITION_OBJ consists of a test and a bunch of expressions to
    /// evaluate if the test evaluates to true.
    struct {
      /// The expression to evaluate in order to decide if this Condition should
      /// run
      struct AstNode *test;
      /// The expressions to evaluate if `test` produces `true`
      struct AstNodeVec exprs;
    } condition_obj;

    /// A LET_DEF_OBJ is an identifier and a value that get associated with each
    /// other
    struct {
      /// The identifier.  Needs to be an IDENTIFIER_NODE.
      struct AstNode *id;
      /// The expression for producing the value to bind to the id
      struct AstNode *val;
    } let_def_obj;

    /// ERROR_OBJ is just an error message
    char *error_obj;
  } data;
};

/// Allocate a new AstNode and set its type
///
/// @param type   The desired type for the node
///
/// @return The newly allocated node
static inline struct AstNode *alloc_astnode(NodeType type) {
  struct AstNode *res = gc_alloc(GC_OBJ_ASTNODE, sizeof(struct AstNode));
  res->type = type;
  res->datum_type = TYPE_RUNTIME;
  return res;
}

/// Create a boolean (Datum) AstNode that expresses the value `b`
///
/// @param b  The boolean value to store in the node
///
/// @return A pointer to the newly allocated node
static inline struct AstNode *make_bool_node(bool b) {
  return alloc_astnode(b ? BOOL_TRUE_DATUM : BOOL_FALSE_DATUM);
}

/// Create a char (Datum) AstNode with value `c`
///
/// @param c  The character to store in the node
///
/// @return A pointer to the newly allocated node
static inline struct AstNode *make_char_node(char c) {
  struct AstNode *n = alloc_astnode(CHAR_DATUM);
  n->data.char_node = c;
  return n;
}

/// Create an int (Datum) AstNode with value `i`
///
/// @param i  The integer to store in the node
///
/// @return A pointer to the newly allocated node
static inline struct AstNode *make_int_node(int i) {
  struct AstNode *n = alloc_astnode(INT_DATUM);
  n->data.int_node = i;
  return n;
}

/// Create a double (Datum) AstNode with value `d`
///
/// @param d  The double to store in the node
///
/// @return A pointer to the newly allocated node
static inline struct AstNode *make_dbl_node(double d) {
  struct AstNode *n = alloc_astnode(DBL_DATUM);
  n->data.dbl_node = d;
  return n;
}

/// Create a string (Datum) AstNode with value `s`
///
/// @param s  The string to duplicate and store in the node
///
/// @return A pointer to the newly allocated node
static inline struct AstNode *make_str_node(const char *s) {
  struct AstNode *n = alloc_astnode(STR_DATUM);
  n->data.str_node = strdup(s);
  return n;
}

/// Create a symbol (Datum) AstNode with value `s`
///
/// @param s  The symbol name to duplicate and store in the node
///
/// @return A pointer to the newly allocated node
static inline struct AstNode *make_symbol_node(const char *s) {
  struct AstNode *n = alloc_astnode(SYMBOL_DATUM);
  n->data.symbol_node = strdup(s);
  return n;
}

/// Create an empty cons (Datum) AstNode
///
/// @return A pointer to the newly allocated node
static inline struct AstNode *make_empty_cons_node() {
  return alloc_astnode(EMPTY_CONS_DATUM);
}

/// Create a cons (Datum) AstNode with the given car and cdr
///
/// @param car  The car (first element) of the cons cell
/// @param cdr  The cdr (rest of the list or second element) of the cons cell
///
/// @return A pointer to the newly allocated node
static inline struct AstNode *make_cons_node(struct AstNode *car,
                                             struct AstNode *cdr) {
  struct AstNode *n = alloc_astnode(CONS_DATUM);
  n->data.cons_node.car = car;
  n->data.cons_node.cdr = cdr;
  return n;
}
