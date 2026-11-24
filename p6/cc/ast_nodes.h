#pragma once

#include <string>
#include <variant>
#include <vector>

#include "util.h"

/// An enum for disambiguating all of the Ast node types
enum NodeType {
  BOOL_FALSE_NODE, ///< A node representing boolean false
  BOOL_TRUE_NODE,  ///< A node representing boolean true
  CHAR_NODE,       ///< A node representing a character
  CONS_NODE,       ///< A node representing a cons cell
  EMPTY_CONS_NODE, ///< A node representing an empty list
  DBL_NODE,        ///< A node representing a double
  INT_NODE,        ///< A node representing an integer
  STR_NODE,        ///< A node representing a string
  SYMBOL_NODE,     ///< A node representing a symbol
  VEC_NODE,        ///< A node representing a vector
  AND_NODE,        ///< A node representing an AND expression
  CALL_NODE,       ///< A node representing a procedure call
  BEGIN_NODE,      ///< A node representing a BEGIN expression
  COND_NODE,       ///< A node representing a COND expression
  DEFINE_VAR_NODE, ///< A node representing a variable definition
  DEFINE_FUNC_NODE,///< A node representing a function definition
  IDENTIFIER_NODE, ///< A node representing an identifier
  IF_NODE,         ///< A node representing an IF expression
  LAMBDA_DEF_NODE, ///< A node representing a lambda definition
  OR_NODE,         ///< A node representing an OR expression
  QUOTE_NODE,      ///< A node representing a QUOTE expression
  SET_NODE,        ///< A node representing a SET expression
  TICK_NODE,       ///< A node representing a tick expression
  LET_NODE         ///< A node representing a LET expression
};

/// AstNode is the base class for all nodes in our abstract syntax tree
struct AstNode {
  NodeType type; // The type of the node
  /// Construct an AstNode by indicating its type
  AstNode(NodeType t) : type(t) {}
  /// Virtual destructor: required because derived node types are destroyed
  /// polymorphically through AstNode * -- without this, deleting
  /// through the base pointer invokes only ~AstNode() with sizeof(AstNode),
  /// never the derived destructor, corrupting the heap.
  virtual ~AstNode() = default;
};

/// Datum is a shadow over AstNode, so we can be more precise about certain
/// kinds of children of other AstNodes
struct Datum : AstNode {
  Datum(NodeType t) : AstNode(t) {}
};

/// A Datum representing a boolean false value
struct BoolFalseNode : Datum {
  /// Construct a BoolFalseNode
  BoolFalseNode() : Datum(BOOL_FALSE_NODE) {}
};

/// A Datum representing a boolean true value
struct BoolTrueNode : Datum {
  /// Construct a BoolTrueNode
  BoolTrueNode() : Datum(BOOL_TRUE_NODE) {}
};

/// A Datum holding a character value
struct CharNode : Datum {
  char val; // The value
  /// Construct a CharNode with a value
  CharNode(char c) : Datum(CHAR_NODE), val(c) {}
};

/// A Datum holding a pair, where each field is a datum
struct ConsNode : Datum {
  Datum *car; // The first value of the pair
  Datum *cdr; // The second value of the pair
  /// Construct a Pair with two values
  ConsNode(Datum *&&a, Datum *&&b)
      : Datum(CONS_NODE), car(std::move(a)), cdr(std::move(b)) {}
};

/// A Node representing an empty list
struct EmptyConsNode : Datum {
  /// Construct an empty cons node
  EmptyConsNode() : Datum(EMPTY_CONS_NODE) {}
};

/// DblNode is for double values
struct DblNode : Datum {
  double val; // The value
  /// Construct a DblNode with a value
  DblNode(double d) : Datum(DBL_NODE), val(d) {}
};

/// IntNode is for integer values
struct IntNode : Datum {
  int val; // The value
  /// Construct an IntNode with a value
  IntNode(int i) : Datum(INT_NODE), val(i) {}
};

/// StrNode is for string values
struct StrNode : Datum {
  std::string val; // The value
  /// Construct a StringNode with a value
  StrNode(std::string_view s) : Datum(STR_NODE), val(s) {}
};

/// SymbolNode is for Scheme Symbols, which are values
struct SymbolNode : Datum {
  std::string name; // The symbol name
  /// Construct a SymbolNode from the name of the symbol
  SymbolNode(std::string_view n) : Datum(SYMBOL_NODE), name(n) {}
};

/// VecNode is for vector values
struct VecNode : Datum {
  std::vector<Datum *> items; // The values
  /// Construct a VecNode from a vector of values
  VecNode(std::vector<Datum *> &&v)
      : Datum(VEC_NODE), items(std::move(v)) {}
};

/// AndNode is for the AND special form
struct AndNode : AstNode {
  std::vector<AstNode *> exprs; // The expressions
  /// Construct an AndNode from a vector of expressions
  AndNode(std::vector<AstNode *> &&e)
      : AstNode(AND_NODE), exprs(std::move(e)) {}
};

/// CallNode is the default form
struct CallNode : AstNode {
  std::vector<AstNode *> exprs; // The expressions
  /// Construct a CallNode from a vector of expressions
  CallNode(std::vector<AstNode *> &&e)
      : AstNode(CALL_NODE), exprs(std::move(e)) {}
};

/// BeginNode is for the BEGIN special form
struct BeginNode : AstNode {
  std::vector<AstNode *> exprs; // The expressions
  /// Construct a BeginNode from a vector of expressions
  BeginNode(std::vector<AstNode *> &&e)
      : AstNode(BEGIN_NODE), exprs(std::move(e)) {}
};

/// Condition is one of the "arms" of a CondNode
struct Condition {
  AstNode *test; // The test expression
  std::vector<AstNode *>
      exprs; // The expressions to run if test is true
  /// Construct a ConditionNode from a test expression and a vector of
  /// expressions to execute if the test evaluates to true
  Condition(AstNode *t,
            std::vector<AstNode *> &&e)
      : test(std::move(t)), exprs(std::move(e)) {}
};

/// CondNode is for the COND special form
struct CondNode : AstNode {
  std::vector<Condition> conditions; // The conditions
  /// Construct a CondNode from a vector of conditions
  CondNode(std::vector<Condition> &&c)
      : AstNode(COND_NODE), conditions(std::move(c)) {}
};

/// IdentifierNode is for identifiers (names that are bound to values)
struct IdentifierNode : AstNode {
  std::string id; // The name associated with this identifier
  /// Construct an IdentifierNode from its name
  IdentifierNode(std::string &i) : AstNode(IDENTIFIER_NODE), id(i) {}
};

/// DefineVarNode is for the DEFINE_VAR special form
struct DefineVarNode : AstNode {
  IdentifierNode *id; // The id whose value is being defined
  AstNode *expr; // The expression to evaluate to get a value
  /// Construct a DefineVarNode from an identifier and an expression
  DefineVarNode(IdentifierNode *&&i,
                AstNode *&&e)
      : AstNode(DEFINE_VAR_NODE), id(std::move(i)), expr(std::move(e)) {}
};

/// DefineFuncNode is for the DEFINE_FUNC special form
struct DefineFuncNode : AstNode {
  std::vector<IdentifierNode *> ids; // The args to the function
  std::vector<AstNode *> body;       // The body of the function
  /// Construct a DefineFuncNode from a vector of ids and a vector of
  /// expressions
  DefineFuncNode(std::vector<IdentifierNode *> &&i,
                 std::vector<AstNode *> &&b)
      : AstNode(DEFINE_FUNC_NODE), ids(std::move(i)), body(std::move(b)) {}
};

/// IfNode is for the IF special form
struct IfNode : AstNode {
  AstNode *cond; // The expression to evaluate to true or false
  AstNode *if_true;  // The expression to evaluate if true
  AstNode *if_false; // The expression to evaluate if false
  /// Construct an IfNode from three expressions
  IfNode(AstNode *&&c, AstNode *&&t,
         AstNode *&&f)
      : AstNode(IF_NODE), cond(std::move(c)), if_true(std::move(t)),
        if_false(std::move(f)) {}
};

/// LambdaDefNode is for the LAMBDA special form
struct LambdaDefNode : AstNode {
  std::vector<IdentifierNode *> formals; // The formal arguments
  std::vector<AstNode *> body; // The body of the function
  /// Construct a LambdaDefNode from the formals and body
  LambdaDefNode(std::vector<IdentifierNode *> &&f,
                std::vector<AstNode *> &&b)
      : AstNode(LAMBDA_DEF_NODE), formals(std::move(f)), body(std::move(b)) {}
};

/// OrNode is for the OR special form
struct OrNode : AstNode {
  std::vector<AstNode *> exprs; // The expressions
  /// Construct an OrNode from a vector of expressions
  OrNode(std::vector<AstNode *> &&e)
      : AstNode(OR_NODE), exprs(std::move(e)) {}
};

/// QuoteNode is for the QUOTE special form
struct QuoteNode : AstNode {
  Datum *datum; // The value that is being quoted
  /// Construct a QuoteNode from a value
  QuoteNode(Datum *&&d)
      : AstNode(QUOTE_NODE), datum(std::move(d)) {}
};

/// SetNode is for the SET special form
struct SetNode : AstNode {
  IdentifierNode *id; // The identifier whose value is being set
  AstNode *
      expr; // The expression that produces the value to set
  /// Construct a SetNode from an identifier and an expression
  SetNode(IdentifierNode *&&i, AstNode *&&e)
      : AstNode(SET_NODE), id(std::move(i)), expr(std::move(e)) {}
};

/// TickNode is for the ABBREV special form
struct TickNode : AstNode {
  Datum *datum; // The value that is being quoted
  /// Construct a TickNode from a value
  TickNode(Datum *&&d)
      : AstNode(TICK_NODE), datum(std::move(d)) {}
};

/// LetDef is for a single binding within a LetNode
struct LetDef {
  IdentifierNode *id; // The identifier for this binding
  AstNode *val;       // The val to associate with the id
  /// Construct a LetDef from an identifier and a value
  LetDef(IdentifierNode *&&i, AstNode *&&v)
      : id(std::move(i)), val(std::move(v)) {}
};

/// LetNode is for the LET special form
struct LetNode : AstNode {
  std::vector<LetDef> vars;                   // The variable bindings
  std::vector<AstNode *> body; // The expressions to evaluate
  /// Construct a LetNode from a vector of let_defs and a vector of expressions
  LetNode(std::vector<LetDef> &&v, std::vector<AstNode *> &&b)
      : AstNode(LET_NODE), vars(std::move(v)), body(std::move(b)) {}
};
