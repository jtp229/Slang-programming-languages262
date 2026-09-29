package main

// ParseError is *not* an AstNode.  We use it to report errors.
type ParseError struct {
	msg string // A description of the error
}

func (e ParseError) Error() string { return e.msg }

// AstNode provides us with a generic name for all of the different node types,
// so that we don't have to use `interface{}` throughout the code.
type AstNode interface {
	isAstNode() bool
}

// astNode is embedded into every concrete AST node type so that each one
// automatically satisfies AstNode, without repeating a one-line isAstNode()
// method for every type.
type astNode struct{}

func (astNode) isAstNode() bool { return true }

// A Bool node representing "true"
type BoolTrueNode struct {
	astNode
}

// A Bool node representing "false"
type BoolFalseNode struct {
	astNode
}

// A character datum
type CharNode struct {
	astNode
	val byte // The character value in this node
}

// A pair (cons cell)
type ConsNode struct {
	astNode
	car AstNode // The first value of the pair
	cdr AstNode // The second value of the pair
}

// An empty cons cell
type EmptyConsNode struct {
	astNode
}

// A double-precision floating point datum
type DblNode struct {
	astNode
	val float64 // The float64 value in this node
}

// An integer datum
type IntNode struct {
	astNode
	val int // The int value in this node
}

// A string datum
type StrNode struct {
	astNode
	val string // The string value in this node
}

// A symbol datum (e.g., 'a)
type SymbolNode struct {
	astNode
	name string // The name of this symbol
}

// A vector of datum elements
type VecNode struct {
	astNode
	items []AstNode // The vector of items
}

// A node representing the AND special form
type AndNode struct {
	astNode
	exprs []AstNode // The expressions to evaluate when computing `and`
}

// A function call node (the "default" form)
type CallNode struct {
	astNode
	exprs []AstNode // The set of expressions that comprise this call
}

// A node representing the BEGIN special form
type BeginNode struct {
	astNode
	exprs []AstNode // The set of expressions to interpret
}

// An arm of a CondNode
type Condition struct {
	astNode
	test  AstNode   // The boolean expression to evaluate
	exprs []AstNode // The expressions to evaluate if `test` is true
}

// A node representing the COND special form
type CondNode struct {
	astNode
	conditions []Condition // The set of conditions
}

// A node representing the DEFINE_VAR special form
type DefineVarNode struct {
	astNode
	identifier IdentifierNode // The identifier whose value is being defined
	expression AstNode        // The expression to evaluate to get a value
}

// A node representing the DEFINE_FUNC special form
type DefineFuncNode struct {
	astNode
	ids  []IdentifierNode // The arguments to the function
	body []AstNode        // The body of the function
}

// A node representing an identifier (a name that is bound to a value)
type IdentifierNode struct {
	astNode
	id string // The name associated with this identifier
}

// A node representing the IF special form
type IfNode struct {
	astNode
	cond     AstNode // The expression to evaluate to true or false
	if_true  AstNode // The expression to evaluate if true
	if_false AstNode // The expression to evaluate if false
}

// A node representing the LAMBDA special form
type LambdaDefNode struct {
	astNode

	// The identifiers that are the formal arguments to the function
	formals []IdentifierNode

	// The body of the function
	body []AstNode
}

// A node representing the OR special form
type OrNode struct {
	astNode
	exprs []AstNode // The expressions to evaluate when computing `or`
}

// A node representing the QUOTE special form
type QuoteNode struct {
	astNode
	datum AstNode // The value that is being quoted
}

// A node representing the SET special form
type SetNode struct {
	astNode
	identifier IdentifierNode // The identifier whose value is being set
	expression AstNode        // The expression that produces the value to set
}

// A node representing the ABBREV special form
type TickNode struct {
	astNode
	datum AstNode // The value that is being quoted
}

// A single binding within a LetNode
type LetDef struct {
	astNode
	id  IdentifierNode // The identifier for this binding
	val AstNode        // The val to associate with the id
}

// A node representing the LET special form
type LetNode struct {
	astNode
	vars []LetDef  // The variable bindings
	body []AstNode // The expressions to evaluate
}

// A node representing the APPLY special form
type ApplyNode struct {
	astNode
	function AstNode // The function to call
	args     AstNode // A list with the arguments to the function
}

// Construct a `cons` node from a vector of items. This can produce a linked
// list of Cons cells.
//
// @param items The list of AstNodes to turn into a cons-cell list
//
// @return A list of ConsNodes terminated by an EmptyConsNode
func makeCons(items []AstNode) (AstNode, error) {
	if len(items) == 0 {
		return nil, ParseError{"Cannot construct Cons from empty list"}
	}
	if len(items) == 1 {
		return &ConsNode{car: items[0], cdr: &EmptyConsNode{}}, nil
	}
	tail, err := makeCons(items[1:])
	if err != nil {
		return nil, err
	}
	return &ConsNode{car: items[0], cdr: tail}, nil
}

// [CSE 262] This is the only go file that you need to edit in this assignment.
//
// The reference solution has ~628 lines of code (plus 31 blank lines and ~92
// lines of comments). Your code may be longer or shorter... the
// line-of-code count is just a reference.
//
// You are allowed to add functions and types to this file.

// Transform a TokenStream into a forest of AstNodes.  It is assumed that the
// TokenStream has an extra EOF at the end.  This is really just a single
// transition: <program> --> <expression>*.
//
// In terms of implementation, this is a recursive descent parser.  Scheme's
// syntax makes the whole affair quite easy.
func parse_program(stream *TokenStream) ([]AstNode, error) {
	var []results AstNode

	for stream.HasNext() && !isEOF(stream){
		expr, err := parseExpression(stream)
		if != nil{
		return nil, err
		}
	
	results = append(results,expr)
}
return results, nil

}

//parseExpression handles all expression types
func parseExpression(stream *TokenStream)(AstNode, error){
//datum
if isToken(stream, "ABBREV"){
	stream.popAny()
	datum, err := parseDatum(stream)
	if err!=nil{
		return nil, err
	}
	return &TickNode {datum: datum}, nil

}
if isConstant(stream) || isToken(stream, "IDENTIFIER"){
	return parseConstantOrIdentifier(stream)
}
//Everything else should start with lparen
if !isToken(stream, "LPAREN"){
	return nil, formatParseError(stream)
}
stream.PopAny() //consume lparen
if isToken(stream, "DEFINE"){
	return parseDefine(stream)
} else if isToken(stream, "QUOTE"){
	stream.PopAny()
	datum, err := parseDatum(stream)
	if err != nil{
		return nil, err
	}
	if err := popToken(stream, "RPAREN"); err != nil {
		return nil, err
	}
	return &QuoteNode{datum: datum}, nil
	//do rest of keywords
} else if{

}
}


func parseConstantOrIdentifier(stream *TokenStream) (AstNode, error){
	tok := stream.Peek()


	switch tok.Type {
		case "IDENTIFIER":
		stream.PopAny()
		return &IdentifierNode{id: tok.Text}, nil
	case "INT":
		stream.PopAny()
		return &IntNode{val: tok.Value.(int)}, nil
	case "DBL":
		stream.PopAny()
		return &DblNode{val: tok.Value.(float64)}, nil
	case "BOOL":
		stream.PopAny()
		if tok.Value.(bool) {
			return &BoolTrueNode{}, nil
		}
		return &BoolFalseNode{}, nil
	case "STR":
		stream.PopAny()
		return &StrNode{val: tok.Value.(string)}, nil
	case "CHAR":
		stream.PopAny()
		return &CharNode{val: tok.Value.(byte)}, nil
	default:
		return nil, ParseError{msg: "Invalid constant"}
	}

}

func parseDefine(stream *TokenStream)(AstNode, error){
	if err := popToken(stream, "DEFINE"); err != nil {
		return nil, err
	}
}

func parseLambda(stream *TokenStream)(AstNode, error){

}

func parseIf(stream *TokenStream)(AstNode, error){

}

func parseSet(stream *TokenStream)(AstNode, error){

}

func parseAnd(stream *TokenStream)(AstNode, error){

}

func parseOr(stream *TokenStream)(AstNode, error){

}

func parseBegin(stream *TokenStream)(AstNode, error){

}

func parseCond(stream *TokenStream)(AstNode, error){

}

func parseLet(stream *TokenStream)(AstNode, error){

}

func parseApply(strean *TokenStream)(AstNode, error){

}
func parseCall(stream *TokenStream) (AstNode, error) {
	
}

func parseDatum(stream *TokenStream) (AstNode, error) {

}

func parseConstant(stream *TokenStream) (AstNode, error) {
}




func isConstant(stream *TokenStream) bool {
	tok := stream.Peek()
	return tok.Type == "INT" || tok.Type == "DBL" || tok.Type == "BOOL" ||
		tok.Type == "STR" || tok.Type == "CHAR"
}

func isToken(stream *TokenStream, tokenType string) bool {
	return stream.HasNext() && stream.Peek().Type == tokenType
}

func isEOF(stream *TokenStream) bool {
	return isToken(stream, "EOF")
}

func popToken(stream *TokenStream, expected string) error {
	if !isToken(stream, expected) {
		return formatParseError(stream)
	}
	stream.PopAny()
	return nil
}

func formatParseError(stream *TokenStream) error {
	if stream.HasNext() {
		tok := stream.Peek()
		return ParseError{msg: fmt.Sprintf("Parse Error: line %d, col %d", tok.Line, tok.Col)}
	}
	return ParseError{msg: "Parse Error: EOF"}
}



