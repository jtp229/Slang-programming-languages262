package main

import "fmt"

// TokenStream is a wrapper around a list of tokens. The reason for the wrapper
// is that our `Parser` can't just iterate through the tokens, because many of
// the grammar's productions require a little bit of look-ahead (one or two
// tokens). TokenStream provides an API that is easy enough to iterate through,
// but which is also friendly to the requirements of `Parser`.
type TokenStream struct {
	// A lightweight iterator mechanism: this int just tells which entry in
	// tokens to return next.  Initialize it to 0.
	next int

	// A collection of tokens, in source-code order
	tokens []Token
}

// Get the next token, via the iterator-like interface
func (t TokenStream) nextToken() Token { return t.tokens[t.next] }

// Get the token after the next token, via the iterator-like interface
func (t TokenStream) nextNextToken() Token { return t.tokens[t.next+1] }

// Pop a token off the stack without checking it
func (t *TokenStream) popAny() { t.next += 1 }

// Pop a token off the stack, but only if it matches an expected type (exp).
// Return "" if all is good, and an error string otherwise.
func (t *TokenStream) popToken(exp int) string {
	if t.tokens[t.next].Type != exp {
		return fmt.Sprintf("Parse Error: line %d, col %d", t.tokens[t.next].Line, t.tokens[t.next].Col)
	}
	t.next += 1
	return ""
}
