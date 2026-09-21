package main

// Literal is an interface that represents a literal value
type Literal interface{ isLiteral() bool }

// BoolLit represents a boolean literal
type BoolLit struct{ val bool }

// CharLit represents a character literal
type CharLit struct{ val byte }

// IntLit represents an integer literal
type IntLit struct{ val int }

// DblLit represents a double literal
type DblLit struct{ val float64 }

// StrLit represents a string literal
type StrLit struct{ val string }

// Return true if the literal is a boolean
func (BoolLit) isLiteral() bool { return true }

// Return true if the literal is a character
func (CharLit) isLiteral() bool { return true }

// Return true if the literal is an int
func (IntLit) isLiteral() bool { return true }

// Return true if the literal is a double
func (DblLit) isLiteral() bool { return true }

// Return true if the literal is a string
func (StrLit) isLiteral() bool { return true }

const (
	TOK_ABBREV      = iota // Quote with zero or more chars following
	TOK_AND                // and
	TOK_APPLY              // apply
	TOK_BEGIN              // begin
	TOK_BOOL               // boolean literal
	TOK_CHAR               // character literal
	TOK_COND               // cond
	TOK_DBL                // double literal
	TOK_DEFINE             // define
	TOK_DOT                // dot
	TOK_EOF                // end of file
	TOK_IDENTIFIER         // identifier
	TOK_IF                 // if
	TOK_INT                // int
	TOK_LAMBDA             // lambda
	TOK_LEFT_PAREN         // left parenthesis
	TOK_LET                // let
	TOK_OR                 // or
	TOK_QUOTE              // quote
	TOK_RIGHT_PAREN        // right parenthesis
	TOK_SET                // set!
	TOK_STR                // string literal
	TOK_VECTOR             // vector
)

// Token represents any individual item we get from the scanner
type Token struct {
	// The text that we saw in the input string
	Text string
	// The line number where the token occurred
	Line int
	// The column where the token started
	Col int
	// Which token is it (uses the "TOK" enum)
	Type int
	// The value associated with this token, if there is one
	literal Literal
}

// Append an extra EOF to the list of tokens, to make parsing easier
//
// @param tokens The list of tokens to update
//
// @return The updated list of tokens
func append_EOF(tokens []Token) []Token {
	tokens = append(tokens, Token{"", -1, -1, TOK_EOF, nil})
	return tokens
}
