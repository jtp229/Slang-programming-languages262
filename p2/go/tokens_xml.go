package main

import (
	"bytes"
	"encoding/xml"
	"fmt"
	"io"
	"strconv"
	"strings"
)

// Escape a string for XML
//
// @param s The string to escape
//
// @return A new string with special characters escaped
func xmlEscape(s string) string {
	var builder strings.Builder
	for i := 0; i < len(s); i++ {
		c := s[i]
		if c == '\\' {
			builder.WriteString("\\\\")
		} else if c == '\t' {
			builder.WriteString("\\t")
		} else if c == '\n' {
			builder.WriteString("\\n")
		} else if c == '\'' {
			builder.WriteString("\\'")
		} else if c == '&' {
			builder.WriteString("&amp;")
		} else if c == '"' {
			builder.WriteString("&quot;")
		} else if c == '>' {
			builder.WriteString("&gt;")
		} else if c == '<' {
			builder.WriteString("&lt;")
		} else {
			builder.WriteByte(c)
		}
	}
	return builder.String()
}

// Unescape a string from XML
//
// @param s The string to unescape
//
// @return A new string with escape sequences replaced, or an error
func xmlUnescape(s string) (string, error) {
	var builder strings.Builder
	i := 0
	for i < len(s) {
		if len(s) > i+1 && s[i:i+2] == "\\\\" {
			builder.WriteByte('\\')
			i += 2
		} else if len(s) > i+1 && s[i:i+2] == "\\t" {
			builder.WriteByte('\t')
			i += 2
		} else if len(s) > i+1 && s[i:i+2] == "\\n" {
			builder.WriteByte('\n')
			i += 2
		} else if len(s) > i+1 && s[i:i+2] == "\\'" {
			builder.WriteByte('\'')
			i += 2
		} else if s[i] == '\\' {
			return "", fmt.Errorf("Invalid string?!? 1")
		} else if len(s) > i+5 && s[i:i+5] == "&amp;" {
			builder.WriteByte('&')
			i += 5
		} else if len(s) > i+6 && s[i:i+5] == "&quot;" {
			builder.WriteByte('"')
			i += 6
		} else if len(s) > i+4 && s[i:i+5] == "&gt;" {
			builder.WriteByte('>')
			i += 4
		} else if len(s) > i+4 && s[i:i+5] == "&lt;" {
			builder.WriteByte('<')
			i += 4
		} else if s[i] == '&' {
			return "", fmt.Errorf("Invalid string?!? 2")
		} else {
			builder.WriteByte(s[i])
			i += 1
		}
	}
	return builder.String(), nil
}

// Read tokens from an XML string
//
// @param x The XML string to parse
//
// @return A list of Tokens reconstituted from the XML, or an error
func readTokensFromXml(x string) ([]Token, error) {
	b := bytes.NewBufferString(x)
	d := xml.NewDecoder(b)
	res := make([]Token, 0)

	// It's essentially a "while" loop to get tokens
	for {
		t, err := d.Token() // next token...
		if t == nil && err == io.EOF {
			break
		}
		if err != nil {
			return nil, err
		}

		if elt, ok := t.(xml.StartElement); ok {
			// Just try to get the line, column, and val string tags
			line, col, vs := 0, 0, ""
			for _, a := range elt.Attr {
				if a.Name.Local == "line" {
					line, err = strconv.Atoi(a.Value)
					if err != nil {
						return nil, err
					}
				} else if a.Name.Local == "col" {
					col, err = strconv.Atoi(a.Value)
					if err != nil {
						return nil, err
					}
				} else if a.Name.Local == "val" {
					vs = a.Value
				}
			}
			// Now use the name to decide what token to make
			switch elt.Name.Local {
			case "AbbrevToken":
				res = append(res, Token{"'", line, col, TOK_ABBREV, nil})
			case "AndToken":
				res = append(res, Token{"'", line, col, TOK_AND, nil})
			case "ApplyToken":
				res = append(res, Token{"'", line, col, TOK_APPLY, nil})
			case "BeginToken":
				res = append(res, Token{"'", line, col, TOK_BEGIN, nil})
			case "BoolToken":
				val := BoolLit{false}
				if vs == "true" {
					val.val = true
				} else if vs != "false" {
					return nil, fmt.Errorf("Invalid XML tag")
				}
				res = append(res, Token{"'", line, col, TOK_BOOL, val})
			case "CharToken":
				val, err := xmlUnescape(vs)
				if err != nil {
					return nil, err
				}
				res = append(res, Token{"'", line, col, TOK_CHAR, CharLit{val[0]}})
			case "CondToken":
				res = append(res, Token{"'", line, col, TOK_COND, nil})
			case "DblToken":
				val, err := strconv.ParseFloat(vs, 64)
				if err != nil {
					return nil, err
				}
				res = append(res, Token{"'", line, col, TOK_DBL, DblLit{val}})
			case "DefineToken":
				res = append(res, Token{"'", line, col, TOK_DEFINE, nil})
			case "DotToken":
				res = append(res, Token{"'", line, col, TOK_DOT, nil})
			case "EofToken":
				res = append(res, Token{"'", line, col, TOK_EOF, nil})
			case "IdentifierToken":
				res = append(res, Token{vs, line, col, TOK_IDENTIFIER, StrLit{vs}})
			case "IfToken":
				res = append(res, Token{"'", line, col, TOK_IF, nil})
			case "IntToken":
				val, err := strconv.ParseInt(vs, 10, 32)
				if err != nil {
					return nil, err
				}
				res = append(res, Token{"'", line, col, TOK_INT, IntLit{int(val)}})
			case "LambdaToken":
				res = append(res, Token{"'", line, col, TOK_LAMBDA, nil})
			case "LeftParenToken":
				res = append(res, Token{"'", line, col, TOK_LEFT_PAREN, nil})
			case "LetToken":
				res = append(res, Token{"'", line, col, TOK_LET, nil})
			case "OrToken":
				res = append(res, Token{"'", line, col, TOK_OR, nil})
			case "QuoteToken":
				res = append(res, Token{"'", line, col, TOK_QUOTE, nil})
			case "RightParenToken":
				res = append(res, Token{"'", line, col, TOK_RIGHT_PAREN, nil})
			case "SetToken":
				res = append(res, Token{"'", line, col, TOK_SET, nil})
			case "StrToken":
				val, err := xmlUnescape(vs)
				if err != nil {
					return nil, err
				}
				res = append(res, Token{"'", line, col, TOK_STR, StrLit{val}})
			case "VecToken":
				res = append(res, Token{"'", line, col, TOK_VECTOR, nil})
			}
		}
	}

	return res, nil
}
