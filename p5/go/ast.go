package main

// The static type of an AST node.  The type checker stores one in every node it
// checks.  RUNTIME is the zero value, so a node that hasn't been checked (or
// that the optimizer just created) is RUNTIME.
type TypeDecoration int

// The possible values of a TypeDecoration.  The order must match the names in
// String().
const (
	RUNTIME TypeDecoration = iota // Unknown until the program runs
	INT                           // An Int
	DBL                           // A Dbl
	BOOL                          // #t or #f
	CHAR                          // A Char
	STR                           // A Str
	SYMBOL                        // A Symbol
	CONS                          // A Cons, or the empty list
	VEC                           // A Vec
	LAMBDA                        // A function
)

// String returns the name of a type, as it appears in warning messages
//
// @return The type's name (e.g., "INT")
func (t TypeDecoration) String() string {
	// NB: The Go compiler is smart enough not to re-allocate on every call
	return [...]string{"RUNTIME", "INT", "DBL", "BOOL", "CHAR", "STR", "SYMBOL", "CONS", "VEC", "LAMBDA"}[t]
}

// AstNode provides us with a generic name for all of the different node types,
// so that we don't have to use `interface{}` throughout the code.
type AstNode interface {
	isAstNode() bool        // Standard "marker" idiom in Go
	Type() TypeDecoration   // Get the node's static type (initially RUNTIME)
	SetType(TypeDecoration) // Set the node's static type
}

// baseNode is embedded in every node type to ensure it satisfies the AstNode
// interface
type baseNode struct {
	td TypeDecoration // The static type of this node
}

func (n *baseNode) isAstNode() bool          { return true }
func (n *baseNode) Type() TypeDecoration     { return n.td }
func (n *baseNode) SetType(t TypeDecoration) { n.td = t }

// A Bool node representing "true"
type BoolTrueNode struct {
	baseNode
}

// A Bool node representing "false"
type BoolFalseNode struct {
	baseNode
}

// A character datum
type CharNode struct {
	baseNode
	val byte // The character value in this node
}

// A pair (cons cell)
type ConsNode struct {
	baseNode
	car AstNode // The first value of the pair
	cdr AstNode // The second value of the pair
}

// An empty cons cell
type EmptyConsNode struct {
	baseNode
}

// A double-precision floating point datum
type DblNode struct {
	baseNode
	val float64 // The float64 value in this node
}

// An integer datum
type IntNode struct {
	baseNode
	val int // The int value in this node
}

// A string datum
type StrNode struct {
	baseNode
	val string // The string value in this node
}

// A symbol datum (e.g., 'a)
type SymbolNode struct {
	baseNode
	name string // The name of this symbol
}

// A vector of datum elements
type VecNode struct {
	baseNode
	items []AstNode // The vector of items
}

// A node representing the AND special form
type AndNode struct {
	baseNode
	exprs []AstNode // The expressions to evaluate when computing `and`
}

// A function call node (the "default" form)
type CallNode struct {
	baseNode
	exprs []AstNode // The set of expressions that comprise this call
}

// A node representing the BEGIN special form
type BeginNode struct {
	baseNode
	exprs []AstNode // The set of expressions to interpret
}

// An arm of a CondNode
type Condition struct {
	baseNode
	test  AstNode   // The boolean expression to evaluate
	exprs []AstNode // The expressions to evaluate if `test` is true
}

// A node representing the COND special form
type CondNode struct {
	baseNode
	conditions []Condition // The set of conditions
}

// A node representing the DEFINE_VAR special form
type DefineVarNode struct {
	baseNode
	identifier IdentifierNode // The identifier whose value is being defined
	expression AstNode        // The expression to evaluate to get a value
}

// A node representing the DEFINE_FUNC special form
type DefineFuncNode struct {
	baseNode
	ids  []IdentifierNode // The arguments to the function
	body []AstNode        // The body of the function
}

// A node representing an identifier (a name that is bound to a value)
type IdentifierNode struct {
	baseNode
	id string // The name associated with this identifier
}

// A node representing the IF special form
type IfNode struct {
	baseNode
	cond     AstNode // The expression to evaluate to true or false
	if_true  AstNode // The expression to evaluate if true
	if_false AstNode // The expression to evaluate if false
}

// A node representing the LAMBDA special form
type LambdaDefNode struct {
	baseNode
	// The identifiers that are the formal arguments to the function
	formals []IdentifierNode

	// The body of the function
	body []AstNode
}

// A node representing the OR special form
type OrNode struct {
	baseNode
	exprs []AstNode // The expressions to evaluate when computing `or`
}

// A node representing the QUOTE special form
type QuoteNode struct {
	baseNode
	datum AstNode // The value that is being quoted
}

// A node representing the SET special form
type SetNode struct {
	baseNode
	identifier IdentifierNode // The identifier whose value is being set
	expression AstNode        // The expression that produces the value to set
}

// A node representing the ABBREV special form (e.g., 'x, which is short for
// (quote x))
type TickNode struct {
	baseNode
	datum AstNode // The value that is being quoted
}

// A single binding within a LetNode
type LetDef struct {
	baseNode
	id  IdentifierNode // The identifier for this binding
	val AstNode        // The val to associate with the id
}

// A node representing the LET special form
type LetNode struct {
	baseNode
	vars []LetDef  // The variable bindings
	body []AstNode // The expressions to evaluate
}

// A node representing the APPLY special form
type ApplyNode struct {
	baseNode
	function AstNode // The function to call
	args     AstNode // A list with the arguments to the function
}
