package main

import "fmt"

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
	var results []AstNode

	for stream.HasNext() && !isEOF(stream) {
		expr, err := parseExpression(stream)
		if err != nil {
			return nil, err
		}

		results = append(results, expr)
	}
	return results, nil

}

// parseExpression handles all expression types for one expression
func parseExpression(stream *TokenStream) (AstNode, error) {
	// qoutation abbreviation handle
	if isToken(stream, "ABBREV") {
		stream.popAny()
		datum, err := parseDatum(stream)
		if err != nil {
			return nil, err
		}
		return datum, nil

	}
	if isConstant(stream) || isToken(stream, "IDENTIFIER") {
		return parseConstantOrIdentifier(stream)
	}
	// Everything else should start with lparen
	if !isToken(stream, "LPAREN") {
		return nil, formatParseError(stream)
	}
	stream.PopAny() //consume lparen
	if isToken(stream, "DEFINE") {
		return parseDefine(stream)
	} else if isToken(stream, "QUOTE") {
		stream.PopAny()
		datum, err := parseDatum(stream)
		if err != nil {
			return nil, err
		}
		if err := popToken(stream, "RPAREN"); err != nil {
			return nil, err
		}
		return &QuoteNode{datum: datum}, nil
		//handle rest of keywords
	} else if isToken(stream, "LAMBDA") {
		return parseLambda(stream)
	} else if isToken(stream, "IF") {
		return parseIf(stream)
	} else if isToken(stream, "SET") {
		return parseSet(stream)
	} else if isToken(stream, "AND") {
		return parseAnd(stream)
	} else if isToken(stream, "OR") {
		return parseOr(stream)
	} else if isToken(stream, "BEGIN") {
		return parseBegin(stream)
	} else if isToken(stream, "COND") {
		return parseCond(stream)
	} else if isToken(stream, "APPLY") {
		return parseApply(stream)
	} else if isToken(stream, "LET") {
		return parseLet(stream)
	} else {
		// <call> --> LPAREN <expression>+ RPAREN
		return parseCall(stream)
	}
}

// consumes one literal or identifier and constructs its AST node
func parseConstantOrIdentifier(stream *TokenStream) (AstNode, error) {
	tok := stream.Peek()

	switch tok.Type {
	case TOK_IDENTIFIER:
		stream.PopAny()
		return &IdentifierNode{id: tok.Text}, nil
	case TOK_INT:
		stream.PopAny()
		return &IntNode{val: tok.literal.(IntLit).val}, nil
	case TOK_DBL:
		stream.PopAny()
		return &DblNode{val: tok.literal.(DblLit).val}, nil
	case TOK_BOOL:
		stream.PopAny()
		if tok.literal.(BoolLit).val {
			return &BoolTrueNode{}, nil
		}
		return &BoolFalseNode{}, nil
	case TOK_STR:
		stream.PopAny()
		return &StrNode{val: tok.literal.(StrLit).val}, nil
	case TOK_CHAR:
		stream.PopAny()
		return &CharNode{val: tok.literal.(CharLit).val}, nil
	default:
		return nil, ParseError{msg: "Invalid constant"}
	}

}

// distinguishe variable definitions from function shorthand 
func parseDefine(stream *TokenStream) (AstNode, error) {
	if err := popToken(stream, "DEFINE"); err != nil {
		return nil, err
	}

	// Check if function shorthand or simple define
	if isToken(stream, "LPAREN") {
		// Function shorthand: (DEFINE (name arg1 arg2...) body...)
		stream.PopAny() // consume LPAREN

		var names []IdentifierNode
		if isToken(stream, "RPAREN") {
			return nil, formatParseError(stream)
		}
		for !isToken(stream, "RPAREN") {
			tok := stream.Peek()
			if tok.Type != TOK_IDENTIFIER {
				return nil, formatParseError(stream)
			}
			stream.PopAny()
			names = append(names, IdentifierNode{id: tok.Text})
		}
		if err := popToken(stream, "RPAREN"); err != nil {
			return nil, err
		}

		// Parse body expressions , shorthand reqs body
		var body []AstNode
		if isToken(stream, "RPAREN") {
			return nil, formatParseError(stream)
		}
		for !isToken(stream, "RPAREN") {
			expr, err := parseExpression(stream)
			if err != nil {
				return nil, err
			}
			body = append(body, expr)
		}

		if err := popToken(stream, "RPAREN"); err != nil {
			return nil, err
		}
		return &DefineFuncNode{ids: names, body: body}, nil
	}

	// Simple define: (DEFINE name value)
	tok := stream.Peek()
	if tok.Type != TOK_IDENTIFIER {
		return nil, formatParseError(stream)
	}
	stream.PopAny()

	value, err := parseExpression(stream)
	if err != nil {
		return nil, err
	}

	if err := popToken(stream, "RPAREN"); err != nil {
		return nil, err
	}

	return &DefineVarNode{
		identifier: IdentifierNode{id: tok.Text},
		expression: value,
	}, nil
}

// parseLambda parses parameter list followed by body expressions Every formal must be an identifier
func parseLambda(stream *TokenStream) (AstNode, error) {
	if err := popToken(stream, "LAMBDA"); err != nil {
		return nil, err
	}

	// Parse formals: (id* )
	if err := popToken(stream, "LPAREN"); err != nil {
		return nil, err
	}

	var params []IdentifierNode
	for !isToken(stream, "RPAREN") {
		tok := stream.Peek()
		if tok.Type != TOK_IDENTIFIER {
			return nil, formatParseError(stream)
		}
		stream.PopAny()
		params = append(params, IdentifierNode{id: tok.Text})
	}

	if err := popToken(stream, "RPAREN"); err != nil {
		return nil, err
	}

	// Parse body
	var body []AstNode
	if isToken(stream, "RPAREN") {
		return nil, formatParseError(stream)
	}
	for !isToken(stream, "RPAREN") {
		expr, err := parseExpression(stream)
		if err != nil {
			return nil, err
		}
		body = append(body, expr)
	}

	if err := popToken(stream, "RPAREN"); err != nil {
		return nil, err
	}

	return &LambdaDefNode{formals: params, body: body}, nil

}

// parses the if sequence: a test, consequent, and alternate
func parseIf(stream *TokenStream) (AstNode, error) {
	if err := popToken(stream, "IF"); err != nil {
		return nil, err
	}

	test, err := parseExpression(stream)
	if err != nil {
		return nil, err
	}

	consequent, err := parseExpression(stream)
	if err != nil {
		return nil, err
	}

	alternate, err := parseExpression(stream)
	if err != nil {
		return nil, err
	}

	if err := popToken(stream, "RPAREN"); err != nil {
		return nil, err
	}

	return &IfNode{cond: test, if_true: consequent, if_false: alternate}, nil

}

//parses SET's required identifier and the expression assigned to it
func parseSet(stream *TokenStream) (AstNode, error) {
	if err := popToken(stream, "SET"); err != nil {
		return nil, err
	}

	tok := stream.Peek()
	if tok.Type != TOK_IDENTIFIER {
		return nil, formatParseError(stream)
	}
	stream.PopAny()

	value, err := parseExpression(stream)
	if err != nil {
		return nil, err
	}

	if err := popToken(stream, "RPAREN"); err != nil {
		return nil, err
	}

	return &SetNode{
		identifier: IdentifierNode{id: tok.Text},
		expression: value,
	}, nil

}

//parses one or more expressions through the form's closing parenthesis
func parseAnd(stream *TokenStream) (AstNode, error) {
	if err := popToken(stream, "AND"); err != nil {
		return nil, err
	}

	var exprs []AstNode
	if isToken(stream, "RPAREN") {
		return nil, formatParseError(stream)
	}
	for !isToken(stream, "RPAREN") {
		expr, err := parseExpression(stream)
		if err != nil {
			return nil, err
		}
		exprs = append(exprs, expr)
	}

	if err := popToken(stream, "RPAREN"); err != nil {
		return nil, err
	}

	return &AndNode{exprs: exprs}, nil

}

// parses one or more expressions through the forms closing parenthesis
func parseOr(stream *TokenStream) (AstNode, error) {
	if err := popToken(stream, "OR"); err != nil {
		return nil, err
	}

	var exprs []AstNode
	if isToken(stream, "RPAREN") {
		return nil, formatParseError(stream)
	}
	for !isToken(stream, "RPAREN") {
		expr, err := parseExpression(stream)
		if err != nil {
			return nil, err
		}
		exprs = append(exprs, expr)
	}

	if err := popToken(stream, "RPAREN"); err != nil {
		return nil, err
	}

	return &OrNode{exprs: exprs}, nil

}

//parses one or more expressions into a BeginNode.
func parseBegin(stream *TokenStream) (AstNode, error) {
	if err := popToken(stream, "BEGIN"); err != nil {
		return nil, err
	}

	var exprs []AstNode
	if isToken(stream, "RPAREN") {
		return nil, formatParseError(stream)
	}
	for !isToken(stream, "RPAREN") {
		expr, err := parseExpression(stream)
		if err != nil {
			return nil, err
		}
		exprs = append(exprs, expr)
	}

	if err := popToken(stream, "RPAREN"); err != nil {
		return nil, err
	}

	return &BeginNode{exprs: exprs}, nil
}

// parses one or more clauses w/parenthesis
func parseCond(stream *TokenStream) (AstNode, error) {
	if err := popToken(stream, "COND"); err != nil {
		return nil, err
	}

	var conditions []Condition
	if isToken(stream, "RPAREN") {
		return nil, formatParseError(stream)
	}
	for !isToken(stream, "RPAREN") {
		if err := popToken(stream, "LPAREN"); err != nil {
			return nil, err
		}

		test, err := parseExpression(stream)
		if err != nil {
			return nil, err
		}

		var exprs []AstNode
		for !isToken(stream, "RPAREN") {
			expr, err := parseExpression(stream)
			if err != nil {
				return nil, err
			}
			exprs = append(exprs, expr)
		}

		if err := popToken(stream, "RPAREN"); err != nil {
			return nil, err
		}

		conditions = append(conditions, Condition{test: test, exprs: exprs})
	}

	if err := popToken(stream, "RPAREN"); err != nil {
		return nil, err
	}

	return &CondNode{conditions: conditions}, nil

}

// parses one or more bindings followed by  body expressions

func parseLet(stream *TokenStream) (AstNode, error) {
	if err := popToken(stream, "LET"); err != nil {
		return nil, err
	}

	// Parse bindings
	if err := popToken(stream, "LPAREN"); err != nil {
		return nil, err
	}

	var bindings []LetDef
	if isToken(stream, "RPAREN") {
		return nil, formatParseError(stream)
	}
	for !isToken(stream, "RPAREN") {
		if err := popToken(stream, "LPAREN"); err != nil {
			return nil, err
		}

		tok := stream.Peek()
		if tok.Type != TOK_IDENTIFIER {
			return nil, formatParseError(stream)
		}
		stream.PopAny()

		value, err := parseExpression(stream)
		if err != nil {
			return nil, err
		}

		if err := popToken(stream, "RPAREN"); err != nil {
			return nil, err
		}

		bindings = append(bindings, LetDef{
			id:  IdentifierNode{id: tok.Text},
			val: value,
		})
	}

	if err := popToken(stream, "RPAREN"); err != nil {
		return nil, err
	}

	// Parse body
	var body []AstNode
	if isToken(stream, "RPAREN") {
		return nil, formatParseError(stream)
	}
	for !isToken(stream, "RPAREN") {
		expr, err := parseExpression(stream)
		if err != nil {
			return nil, err
		}
		body = append(body, expr)
	}

	if err := popToken(stream, "RPAREN"); err != nil {
		return nil, err
	}

	return &LetNode{vars: bindings, body: body}, nil

}

// parses a function expression and one expression containing its
// argument list, then consumes the closing parenthesi
func parseApply(stream *TokenStream) (AstNode, error) {
	if err := popToken(stream, "APPLY"); err != nil {
		return nil, err
	}

	fn, err := parseExpression(stream)
	if err != nil {
		return nil, err
	}

	args, err := parseExpression(stream)
	if err != nil {
		return nil, err
	}

	if err := popToken(stream, "RPAREN"); err != nil {
		return nil, err
	}

	return &ApplyNode{function: fn, args: args}, nil

}

// parses the default application form. 1st expression is operator, rest are operands
func parseCall(stream *TokenStream) (AstNode, error) {
	var exprs []AstNode
	if isToken(stream, "RPAREN") {
		return nil, formatParseError(stream)
	}
	for !isToken(stream, "RPAREN") {
		expr, err := parseExpression(stream)
		if err != nil {
			return nil, err
		}
		exprs = append(exprs, expr)
	}

	if err := popToken(stream, "RPAREN"); err != nil {
		return nil, err
	}

	return &CallNode{exprs: exprs}, nil

}

// parses quoted data  
func parseDatum(stream *TokenStream) (AstNode, error) {
	// Constants keep their ordinary value nodes when used as data
	if isConstant(stream) {
		return parseConstant(stream)
	}

	// In datum position, an identifier names a symbol rather than a variable
	if isToken(stream, "IDENTIFIER") {
		tok := stream.Peek()
		stream.PopAny()
		return &SymbolNode{name: tok.Text}, nil
	}

	// Parenthesized data is empty, a dotted pair, or a  list
	if isToken(stream, "LPAREN") {
		stream.PopAny()

		if isToken(stream, "RPAREN") {
			stream.PopAny()
			return &EmptyConsNode{}, nil
		}

		first, err := parseDatum(stream)
		if err != nil {
			return nil, err
		}

		if isToken(stream, "DOT") {
			// <cons> LPAREN <datum> DOT <datum> RPAREN
			stream.PopAny()
			cdr, err := parseDatum(stream)
			if err != nil {
				return nil, err
			}
			if err := popToken(stream, "RPAREN"); err != nil {
				return nil, err
			}
			return &ConsNode{car: first, cdr: cdr}, nil
		}

		// get the proper list before converting it to linked cons cells.
		var datums []AstNode
		datums = append(datums, first)

		for !isToken(stream, "RPAREN") {
			datum, err := parseDatum(stream)
			if err != nil {
				return nil, err
			}
			datums = append(datums, datum)
		}

		if err := popToken(stream, "RPAREN"); err != nil {
			return nil, err
		}

		result, err := makeCons(datums)
		if err != nil {
			return nil, err
		}
		return result, nil
	}

	// Vector elements are datums and continue until the closing parenthesis
	if isToken(stream, "VEC") {
		stream.PopAny()
		var items []AstNode
		for !isToken(stream, "RPAREN") {
			datum, err := parseDatum(stream)
			if err != nil {
				return nil, err
			}
			items = append(items, datum)
		}
		if err := popToken(stream, "RPAREN"); err != nil {
			return nil, err
		}
		return &VecNode{items: items}, nil
	}

	return nil, formatParseError(stream)

}

// delegate node construction to the shared  parser
func parseConstant(stream *TokenStream) (AstNode, error) {
	return parseConstantOrIdentifier(stream)
}

// check if next token is literal value
func isConstant(stream *TokenStream) bool {
	tok := stream.Peek()
	return tok.Type == TOK_INT || tok.Type == TOK_DBL || tok.Type == TOK_BOOL ||
		tok.Type == TOK_STR || tok.Type == TOK_CHAR
}

// compare next token to readable token name
func isToken(stream *TokenStream, tokenType string) bool {
	if !stream.HasNext() {
		return false
	}

	tokenTypes := map[string]int{
		"ABBREV": TOK_ABBREV, "AND": TOK_AND, "APPLY": TOK_APPLY,
		"BEGIN": TOK_BEGIN, "BOOL": TOK_BOOL, "CHAR": TOK_CHAR,
		"COND": TOK_COND, "DBL": TOK_DBL, "DEFINE": TOK_DEFINE,
		"DOT": TOK_DOT, "EOF": TOK_EOF, "IDENTIFIER": TOK_IDENTIFIER,
		"IF": TOK_IF, "INT": TOK_INT, "LAMBDA": TOK_LAMBDA,
		"LPAREN": TOK_LEFT_PAREN, "LET": TOK_LET, "OR": TOK_OR,
		"QUOTE": TOK_QUOTE, "RPAREN": TOK_RIGHT_PAREN, "SET": TOK_SET,
		"STR": TOK_STR, "VEC": TOK_VECTOR,
	}
	typeID, ok := tokenTypes[tokenType]
	return ok && stream.Peek().Type == typeID
}

// check for eof
func isEOF(stream *TokenStream) bool {
	return isToken(stream, "EOF")
}

// consume token
func popToken(stream *TokenStream, expected string) error {
	if !isToken(stream, expected) {
		return formatParseError(stream)
	}
	stream.PopAny()
	return nil
}

// error formatting to match test cases
func formatParseError(stream *TokenStream) error {
	if stream.HasNext() {
		tok := stream.Peek()
		return ParseError{msg: fmt.Sprintf("Parse Error: line %d, col %d", tok.Line, tok.Col)}
	}
	return ParseError{msg: "Parse Error: EOF"}
}

// check for unread token
func (t *TokenStream) HasNext() bool {
	return t.next < len(t.tokens)
}
//return current token
func (t *TokenStream) Peek() Token {
	return t.nextToken()
}

//consumes the current token without checking its type.
func (t *TokenStream) PopAny() {
	t.popAny()
}
