package main

import (
	"bytes"
	"encoding/xml"
	"fmt"
	"io"
	"strconv"
	"strings"
)

const TAG_ACTIONS = "Actions"
const TAG_AND = "And"
const TAG_APPLY = "Apply"
const TAG_BEGIN = "Begin"
const TAG_BINDING = "Binding"
const TAG_BINDINGS = "Bindings"
const TAG_BOOL = "Bool"
const TAG_CALL = "Call"
const TAG_CHAR = "Char"
const TAG_COND = "Cond"
const TAG_CONDITION = "Condition"
const TAG_CONS = "Cons"
const TAG_DBL = "Dbl"
const TAG_DEFINE_FUNC = "DefineFunc"
const TAG_DEFINE_VAR = "DefineVar"
const TAG_EMPTY = "Empty"
const TAG_EXPRESSIONS = "Expressions"
const TAG_FORMALS = "Formals"
const TAG_IDENTIFIER = "Identifier"
const TAG_IDENTIFIERS = "Identifiers"
const TAG_IF = "If"
const TAG_INT = "Int"
const TAG_LAMBDA = "Lambda"
const TAG_LET = "Let"
const TAG_OR = "Or"
const TAG_QUOTE = "Quote"
const TAG_SET = "Set"
const TAG_STR = "Str"
const TAG_SYMBOL = "Symbol"
const TAG_TEST = "Test"
const TAG_TICK = "Tick"
const TAG_VECTOR = "Vector"

// Serialize an AstNode to a StringBuilder as XML.  `indent` indicates the
// amount of indentation for this element.
//
// @param builder The strings.Builder to write the XML to
// @param node    The AstNode to serialize
// @param indent  The indentation string
//
// @return An error if serialization fails, else nil
func nodeToXml(builder *strings.Builder, node AstNode, indent string) error {
	if _, ok := node.(*BoolTrueNode); ok {
		builder.WriteString(indent + "<" + TAG_BOOL + " val=\"true\"/>\n")
	} else if _, ok := node.(*BoolFalseNode); ok {
		builder.WriteString(indent + "<" + TAG_BOOL + " val=\"false\"/>\n")
	} else if n, ok := node.(*CharNode); ok {
		builder.WriteString(indent + "<" + TAG_CHAR + " val=\"" + xmlEscape(string(n.val)) + "\"/>\n")
	} else if n, ok := node.(*ConsNode); ok {
		builder.WriteString(indent + "<" + TAG_CONS + ">\n")
		nodeToXml(builder, n.car, indent+" ")
		nodeToXml(builder, n.cdr, indent+" ")
		builder.WriteString(indent + "</" + TAG_CONS + ">\n")
	} else if _, ok := node.(*EmptyConsNode); ok {
		builder.WriteString(indent + "<" + TAG_EMPTY + "/>\n")
	} else if n, ok := node.(*DblNode); ok {
		builder.WriteString(indent + "<" + TAG_DBL + " val=\"")
		builder.WriteString(strconv.FormatFloat(n.val, 'f', -1, 64))
		builder.WriteString("\"/>\n")
	} else if n, ok := node.(*IntNode); ok {
		builder.WriteString(indent + "<" + TAG_INT + " val=\"")
		builder.WriteString(strconv.Itoa(n.val))
		builder.WriteString("\"/>\n")
	} else if n, ok := node.(*StrNode); ok {
		builder.WriteString(indent + "<" + TAG_STR + " val=\"" + xmlEscape(n.val) + "\"/>\n")
	} else if n, ok := node.(*SymbolNode); ok {
		builder.WriteString(indent + "<" + TAG_SYMBOL + " val=\"" + xmlEscape(n.name) + "\"/>\n")
	} else if n, ok := node.(*VecNode); ok {
		if len(n.items) == 0 {
			builder.WriteString(indent + "<" + TAG_VECTOR + "/>\n")
		} else {
			builder.WriteString(indent + "<" + TAG_VECTOR + ">\n")
			for _, d := range n.items {
				nodeToXml(builder, d, indent+" ")
			}
			builder.WriteString(indent + "</" + TAG_VECTOR + ">\n")
		}
	} else if n, ok := node.(*AndNode); ok {
		builder.WriteString(indent + "<" + TAG_AND + ">\n")
		for _, e := range n.exprs {
			nodeToXml(builder, e, indent+" ")
		}
		builder.WriteString(indent + "</" + TAG_AND + ">\n")
	} else if n, ok := node.(*CallNode); ok {
		builder.WriteString(indent + "<" + TAG_CALL + ">\n")
		for _, e := range n.exprs {
			nodeToXml(builder, e, indent+" ")
		}
		builder.WriteString(indent + "</" + TAG_CALL + ">\n")
	} else if n, ok := node.(*BeginNode); ok {
		builder.WriteString(indent + "<" + TAG_BEGIN + ">\n")
		for _, e := range n.exprs {
			nodeToXml(builder, e, indent+" ")
		}
		builder.WriteString(indent + "</" + TAG_BEGIN + ">\n")
	} else if n, ok := node.(*Condition); ok {
		builder.WriteString(indent + "<" + TAG_CONDITION + ">\n")
		builder.WriteString(indent + " <" + TAG_TEST + ">\n")
		nodeToXml(builder, n.test, "  "+indent)
		builder.WriteString(indent + " </" + TAG_TEST + ">\n")
		if len(n.exprs) == 0 {
			builder.WriteString(indent + " <" + TAG_ACTIONS + "/>\n")
		} else {
			builder.WriteString(indent + " <" + TAG_ACTIONS + ">\n")
			for _, e := range n.exprs {
				nodeToXml(builder, e, "  "+indent)
			}
			builder.WriteString(indent + " </" + TAG_ACTIONS + ">\n")
		}
		builder.WriteString(indent + "</" + TAG_CONDITION + ">\n")
	} else if n, ok := node.(*CondNode); ok {
		builder.WriteString(indent + "<" + TAG_COND + ">\n")
		for _, e := range n.conditions {
			nodeToXml(builder, &e, indent+" ")
		}
		builder.WriteString(indent + "</" + TAG_COND + ">\n")
	} else if n, ok := node.(*DefineVarNode); ok {
		builder.WriteString(indent + "<" + TAG_DEFINE_VAR + ">\n")
		nodeToXml(builder, &n.identifier, indent+" ")
		nodeToXml(builder, n.expression, indent+" ")
		builder.WriteString(indent + "</" + TAG_DEFINE_VAR + ">\n")
	} else if n, ok := node.(*DefineFuncNode); ok {
		builder.WriteString(indent + "<" + TAG_DEFINE_FUNC + ">\n")
		builder.WriteString(indent + " <" + TAG_IDENTIFIERS + ">\n")
		for _, i := range n.ids {
			nodeToXml(builder, &i, "  "+indent)
		}
		builder.WriteString(indent + " </" + TAG_IDENTIFIERS + ">\n")
		builder.WriteString(indent + " <" + TAG_EXPRESSIONS + ">\n")
		for _, e := range n.body {
			nodeToXml(builder, e, "  "+indent)
		}
		builder.WriteString(indent + " </" + TAG_EXPRESSIONS + ">\n")
		builder.WriteString(indent + "</" + TAG_DEFINE_FUNC + ">\n")
	} else if n, ok := node.(*IdentifierNode); ok {
		builder.WriteString(indent + "<" + TAG_IDENTIFIER + " val=\"" + xmlEscape(n.id) + "\"/>\n")
	} else if n, ok := node.(*IfNode); ok {
		builder.WriteString(indent + "<" + TAG_IF + ">\n")
		nodeToXml(builder, n.cond, indent+" ")
		nodeToXml(builder, n.if_true, indent+" ")
		nodeToXml(builder, n.if_false, indent+" ")
		builder.WriteString(indent + "</" + TAG_IF + ">\n")
	} else if n, ok := node.(*LambdaDefNode); ok {
		builder.WriteString(indent + "<" + TAG_LAMBDA + ">\n")
		if len(n.formals) == 0 {
			builder.WriteString(indent + " <" + TAG_FORMALS + "/>\n")

		} else {
			builder.WriteString(indent + " <" + TAG_FORMALS + ">\n")
			for _, f := range n.formals {
				nodeToXml(builder, &f, indent+"  ")
			}
			builder.WriteString(indent + " </" + TAG_FORMALS + ">\n")
		}
		builder.WriteString(indent + " <" + TAG_EXPRESSIONS + ">\n")
		for _, e := range n.body {
			nodeToXml(builder, e, indent+"  ")
		}
		builder.WriteString(indent + " </" + TAG_EXPRESSIONS + ">\n")
		builder.WriteString(indent + "</" + TAG_LAMBDA + ">\n")
	} else if n, ok := node.(*OrNode); ok {
		builder.WriteString(indent + "<" + TAG_OR + ">\n")
		for _, e := range n.exprs {
			nodeToXml(builder, e, indent+" ")
		}
		builder.WriteString(indent + "</" + TAG_OR + ">\n")
	} else if n, ok := node.(*QuoteNode); ok {
		builder.WriteString(indent + "<" + TAG_QUOTE + ">\n")
		nodeToXml(builder, n.datum, indent+" ")
		builder.WriteString(indent + "</" + TAG_QUOTE + ">\n")
	} else if n, ok := node.(*SetNode); ok {
		builder.WriteString(indent + "<" + TAG_SET + ">\n")
		nodeToXml(builder, &n.identifier, indent+" ")
		nodeToXml(builder, n.expression, indent+" ")
		builder.WriteString(indent + "</" + TAG_SET + ">\n")
	} else if n, ok := node.(*TickNode); ok {
		builder.WriteString(indent + "<" + TAG_TICK + ">\n")
		nodeToXml(builder, n.datum, indent+" ")
		builder.WriteString(indent + "</" + TAG_TICK + ">\n")
	} else if n, ok := node.(*LetDef); ok {
		builder.WriteString(indent + "<" + TAG_BINDING + ">\n")
		nodeToXml(builder, &n.id, indent+" ")
		nodeToXml(builder, n.val, indent+" ")
		builder.WriteString(indent + "</" + TAG_BINDING + ">\n")
	} else if n, ok := node.(*LetNode); ok {
		builder.WriteString(indent + "<" + TAG_LET + ">\n")
		builder.WriteString(indent + " <" + TAG_BINDINGS + ">\n")
		for _, v := range n.vars {
			nodeToXml(builder, &v, "  "+indent)
		}
		builder.WriteString(indent + " </" + TAG_BINDINGS + ">\n")
		builder.WriteString(indent + " <" + TAG_ACTIONS + ">\n")
		for _, v := range n.body {
			nodeToXml(builder, v, "  "+indent)
		}
		builder.WriteString(indent + " </" + TAG_ACTIONS + ">\n")
		builder.WriteString(indent + "</" + TAG_LET + ">\n")
	} else if n, ok := node.(*ApplyNode); ok {
		builder.WriteString(indent + "<" + TAG_APPLY + ">\n")
		nodeToXml(builder, n.function, indent+" ")
		nodeToXml(builder, n.args, indent+" ")
		builder.WriteString(indent + "</" + TAG_APPLY + ">\n")
	} else {
		return fmt.Errorf("XML Error")
	}
	return nil
}

// Serialize a forest of AstNodes into an XML-formatted string
//
// @param forest The list of AstNodes to serialize
//
// @return An XML string representation of the forest, or an error
func astToXml(forest []AstNode) (string, error) {
	var builder strings.Builder
	builder.WriteString("<Ast xmlns=\"ast\">\n")
	for _, e := range forest {
		err := nodeToXml(&builder, e, " ")
		if err != nil {
			return "", err
		}
	}
	builder.WriteString("</Ast>")
	return builder.String(), nil
}

// Get 0 or more expressions until a specific XML end tag is encountered
//
// @param decoder The xml.Decoder to read from
// @param end_tag The XML tag name that terminates the sequence
//
// @return A list of AstNodes, or an error
func get_exprs_until(decoder *xml.Decoder, end_tag string) ([]AstNode, error) {
	items := make([]AstNode, 0)
	for {
		t, err := decoder.Token()
		if err != nil {
			return nil, err
		}
		if e, ok := t.(xml.StartElement); ok {
			expr, err := xmlToNode(&e, decoder)
			if err != nil {
				return nil, err
			}
			items = append(items, expr)
		} else if _, ok := t.(xml.CharData); ok {
			// ignore!
		} else if end, ok := t.(xml.EndElement); ok {
			if end.Name.Local == end_tag {
				return items, nil
			}
			return nil, fmt.Errorf("XML Error")
		} else {
			return nil, fmt.Errorf("XML Error")
		}
	}
}

// Consume the expected closing XML tag, skipping any intervening content.
//
// @param decoder The xml.Decoder to read from
// @param tag     The XML tag name to consume
//
// @return An error if the tag is not found, else nil
func closeTag(decoder *xml.Decoder, tag string) error {
	t, err := decoder.Token()
	for {
		if err != nil {
			return err
		}
		if e, ok := t.(xml.EndElement); ok {
			if e.Name.Local != tag {
				return fmt.Errorf("XML error")
			}
			return nil
		}
		if _, ok := t.(xml.CharData); !ok {
			return fmt.Errorf("XML Error")
		}
		t, err = decoder.Token()
	}
}

// Open the next XML tag
//
// @param decoder The xml.Decoder to read from
//
// @return The next XML start element, or an error
func openTag(decoder *xml.Decoder) (*xml.StartElement, error) {
	t, err := decoder.Token()
	for {
		if err != nil {
			return nil, err
		}
		if e, ok := t.(xml.StartElement); ok {
			return &e, nil
		}
		if _, ok := t.(xml.CharData); !ok {
			return nil, fmt.Errorf("XML Error")
		}
		t, err = decoder.Token()
	}
}

// Convert an XML start element into its corresponding AST node
//
// @param node    The current XML start element
// @param decoder The xml.Decoder to read from
//
// @return An AstNode reconstituted from the XML, or an error
func xmlToNode(node *xml.StartElement, decoder *xml.Decoder) (AstNode, error) {
	name := node.Name.Local
	// The first 7 are self-closing, the rest aren't
	switch name {
	case TAG_IDENTIFIER:
		if len(node.Attr) != 1 || node.Attr[0].Name.Local != "val" {
			return nil, fmt.Errorf("XML error")
		}
		val := node.Attr[0].Value
		if err := closeTag(decoder, TAG_IDENTIFIER); err != nil {
			return nil, err
		}
		return &IdentifierNode{id: val}, nil
	case TAG_BOOL:
		if len(node.Attr) != 1 || node.Attr[0].Name.Local != "val" {
			return nil, fmt.Errorf("XML error")
		}
		val := node.Attr[0].Value
		if err := closeTag(decoder, TAG_BOOL); err != nil {
			return nil, err
		}
		if val == "true" {
			return &BoolTrueNode{}, nil
		} else if val == "false" {
			return &BoolFalseNode{}, nil
		} else {
			return nil, fmt.Errorf("XML Error")
		}
	case TAG_INT:
		if len(node.Attr) != 1 || node.Attr[0].Name.Local != "val" {
			return nil, fmt.Errorf("XML error")
		}
		val, err := strconv.Atoi(node.Attr[0].Value)
		if err != nil {
			return nil, err
		}
		err = closeTag(decoder, TAG_INT)
		if err != nil {
			return nil, err
		}
		return &IntNode{val: val}, nil
	case TAG_DBL:
		if len(node.Attr) != 1 || node.Attr[0].Name.Local != "val" {
			return nil, fmt.Errorf("XML error")
		}
		val, err := strconv.ParseFloat(node.Attr[0].Value, 64)
		if err != nil {
			return nil, err
		}
		err = closeTag(decoder, TAG_DBL)
		if err != nil {
			return nil, err
		}
		return &DblNode{val: val}, nil
	case TAG_CHAR:
		if len(node.Attr) != 1 || node.Attr[0].Name.Local != "val" {
			return nil, fmt.Errorf("XML error")
		}
		val, err := xmlUnescape(node.Attr[0].Value)
		if err != nil {
			return nil, err
		}
		if len(val) != 1 {
			return nil, fmt.Errorf("XML Error")
		}
		err = closeTag(decoder, TAG_CHAR)
		if err != nil {
			return nil, err
		}
		return &CharNode{val: val[0]}, nil
	case TAG_STR:
		if len(node.Attr) != 1 || node.Attr[0].Name.Local != "val" {
			return nil, fmt.Errorf("XML error")
		}
		val, err := xmlUnescape(node.Attr[0].Value)
		if err != nil {
			return nil, err
		}
		err = closeTag(decoder, TAG_STR)
		if err != nil {
			return nil, err
		}
		return &StrNode{val: val}, nil
	case TAG_SYMBOL:
		if len(node.Attr) != 1 || node.Attr[0].Name.Local != "val" {
			return nil, fmt.Errorf("XML error")
		}
		val := node.Attr[0].Value
		err := closeTag(decoder, TAG_SYMBOL)
		if err != nil {
			return nil, err
		}
		return &SymbolNode{name: val}, nil
	case TAG_DEFINE_VAR:
		t, err := openTag(decoder)
		if err != nil {
			return nil, err
		}
		id, err := xmlToNode(t, decoder)
		if err != nil {
			return nil, err
		}
		if _, ok := id.(*IdentifierNode); !ok {
			return nil, fmt.Errorf("XML Error")
		}
		t, err = openTag(decoder)
		if err != nil {
			return nil, err
		}
		expr, err := xmlToNode(t, decoder)
		if err != nil {
			return nil, err
		}
		if err = closeTag(decoder, TAG_DEFINE_VAR); err != nil {
			return nil, err
		}
		return &DefineVarNode{identifier: *id.(*IdentifierNode), expression: expr}, nil
	case TAG_IF:
		t, err := openTag(decoder)
		if err != nil {
			return nil, err
		}
		test, err := xmlToNode(t, decoder)
		if err != nil {
			return nil, err
		}
		t, err = openTag(decoder)
		if err != nil {
			return nil, err
		}
		ifTrue, err := xmlToNode(t, decoder)
		if err != nil {
			return nil, err
		}
		t, err = openTag(decoder)
		if err != nil {
			return nil, err
		}
		ifFalse, err := xmlToNode(t, decoder)
		if err != nil {
			return nil, err
		}
		err = closeTag(decoder, TAG_IF)
		if err != nil {
			return nil, err
		}
		return &IfNode{cond: test, if_true: ifTrue, if_false: ifFalse}, nil
	case TAG_SET:
		t, err := openTag(decoder)
		if err != nil {
			return nil, err
		}
		id, err := xmlToNode(t, decoder)
		if err != nil {
			return nil, err
		}
		if _, ok := id.(*IdentifierNode); !ok {
			return nil, fmt.Errorf("XML Error")
		}
		t, err = openTag(decoder)
		if err != nil {
			return nil, err
		}
		expr, err := xmlToNode(t, decoder)
		if err != nil {
			return nil, err
		}
		if err = closeTag(decoder, TAG_SET); err != nil {
			return nil, err
		}
		return &SetNode{identifier: *id.(*IdentifierNode), expression: expr}, nil
	case TAG_QUOTE:
		t, err := openTag(decoder)
		if err != nil {
			return nil, err
		}
		datum, err := xmlToNode(t, decoder)
		if err != nil {
			return nil, err
		}
		// [mfs] TODO: Ensure it's a datum
		if err = closeTag(decoder, TAG_QUOTE); err != nil {
			return nil, err
		}
		return &QuoteNode{datum: datum}, nil
	case TAG_TICK:
		t, err := openTag(decoder)
		if err != nil {
			return nil, err
		}
		datum, err := xmlToNode(t, decoder)
		if err != nil {
			return nil, err
		}
		// [mfs] TODO: Ensure it's a datum
		if err = closeTag(decoder, TAG_TICK); err != nil {
			return nil, err
		}
		return &TickNode{datum: datum}, nil
	case TAG_LAMBDA:
		f, err := openTag(decoder) // Formals
		if err != nil {
			return nil, err
		}
		if f.Name.Local != TAG_FORMALS {
			return nil, fmt.Errorf("XML Error")
		}

		items, err := get_exprs_until(decoder, TAG_FORMALS)
		if err != nil {
			return nil, err
		}

		// Convert to Identifiers
		formals := make([]IdentifierNode, 0)
		for _, item := range items {
			if id, ok := item.(*IdentifierNode); ok {
				formals = append(formals, *id)
			} else {
				return nil, fmt.Errorf("XML Error")
			}
		}

		b, err := openTag(decoder)
		if err != nil {
			return nil, err
		}
		if b.Name.Local != TAG_EXPRESSIONS {
			return nil, fmt.Errorf("XML Error")
		}

		body, err := get_exprs_until(decoder, TAG_EXPRESSIONS)
		if err != nil {
			return nil, err
		}

		if err = closeTag(decoder, TAG_LAMBDA); err != nil {
			return nil, err
		}
		return &LambdaDefNode{formals: formals, body: body}, nil
	case TAG_AND:
		items, err := get_exprs_until(decoder, TAG_AND)
		if err != nil {
			return nil, err
		}
		return &AndNode{exprs: items}, nil
	case TAG_CALL:
		items, err := get_exprs_until(decoder, TAG_CALL)
		if err != nil {
			return nil, err
		}
		return &CallNode{exprs: items}, nil
	case TAG_OR:
		items, err := get_exprs_until(decoder, TAG_OR)
		if err != nil {
			return nil, err
		}
		return &OrNode{exprs: items}, nil
	case TAG_BEGIN:
		items, err := get_exprs_until(decoder, TAG_BEGIN)
		if err != nil {
			return nil, err
		}
		return &BeginNode{exprs: items}, nil
	case TAG_VECTOR:
		items, err := get_exprs_until(decoder, TAG_VECTOR)
		if err != nil {
			return nil, err
		}
		// [mfs] Need to ensure they're Datum?
		return &VecNode{items: items}, nil
	case TAG_COND:
		conditions := make([]Condition, 0)
		for {
			t, err := decoder.Token()
			if err != nil {
				return nil, err
			}
			if c, ok := t.(xml.StartElement); ok {
				if c.Name.Local != TAG_CONDITION {
					return nil, fmt.Errorf("XML Error")
				}
				condition, err := xmlToNode(&c, decoder)
				if err != nil {
					return nil, err
				}
				conditions = append(conditions, *condition.(*Condition))
			} else if _, ok := t.(xml.CharData); ok {
				// ignore
			} else if end, ok := t.(xml.EndElement); ok {
				if end.Name.Local == TAG_COND {
					break
				}
				return nil, fmt.Errorf("XML Error")
			} else {
				return nil, fmt.Errorf("XML Error")
			}
		}
		return &CondNode{conditions: conditions}, nil
	case TAG_CONDITION:
		t, err := openTag(decoder)
		if err != nil {
			return nil, err
		}
		if t.Name.Local != TAG_TEST {
			return nil, fmt.Errorf("XML Error")
		}
		t, err = openTag(decoder)
		if err != nil {
			return nil, err
		}
		test, err := xmlToNode(t, decoder)
		if err != nil {
			return nil, err
		}
		if err = closeTag(decoder, TAG_TEST); err != nil {
			return nil, err
		}
		a, err := openTag(decoder)
		if err != nil {
			return nil, err
		}
		if a.Name.Local != TAG_ACTIONS {
			return nil, fmt.Errorf("XML Error")
		}
		actions, err := get_exprs_until(decoder, TAG_ACTIONS)
		if err != nil {
			return nil, err
		}
		if err := closeTag(decoder, TAG_CONDITION); err != nil {
			return nil, err
		}
		return &Condition{test: test, exprs: actions}, nil
	case TAG_CONS:
		t, err := openTag(decoder)
		if err != nil {
			return nil, err
		}
		car, err := xmlToNode(t, decoder)
		if err != nil {
			return nil, err
		}
		t, err = openTag(decoder)
		if err != nil {
			return nil, err
		}
		cdr, err := xmlToNode(t, decoder)
		if err != nil {
			return nil, err
		}
		if err = closeTag(decoder, TAG_CONS); err != nil {
			return nil, err
		}
		return &ConsNode{car: car, cdr: cdr}, nil
	case TAG_EMPTY:
		if err := closeTag(decoder, TAG_EMPTY); err != nil {
			return nil, err
		}
		return &EmptyConsNode{}, nil
	case TAG_APPLY:
		t, err := openTag(decoder)
		if err != nil {
			return nil, err
		}
		function, err := xmlToNode(t, decoder)
		if err != nil {
			return nil, err
		}
		t, err = openTag(decoder)
		if err != nil {
			return nil, err
		}
		args, err := xmlToNode(t, decoder)
		if err != nil {
			return nil, err
		}
		if err = closeTag(decoder, TAG_APPLY); err != nil {
			return nil, err
		}
		return &ApplyNode{function: function, args: args}, nil
	case TAG_DEFINE_FUNC:
		// identifiers and expressions are the children
		i, err := openTag(decoder)
		if err != nil {
			return nil, err
		}
		if i.Name.Local != TAG_IDENTIFIERS {
			return nil, fmt.Errorf("XML Error")
		}
		items, err := get_exprs_until(decoder, TAG_IDENTIFIERS)
		if err != nil {
			return nil, err
		}

		// Convert to identifiers
		ids := make([]IdentifierNode, 0)
		for _, item := range items {
			if id, ok := item.(*IdentifierNode); ok {
				ids = append(ids, *id)
			} else {
				return nil, fmt.Errorf("XML Error")
			}
		}

		e, err := openTag(decoder)
		if err != nil {
			return nil, err
		}
		if e.Name.Local != TAG_EXPRESSIONS {
			return nil, fmt.Errorf("XML Error")
		}
		exprs, err := get_exprs_until(decoder, TAG_EXPRESSIONS)
		if err != nil {
			return nil, err
		}
		if err = closeTag(decoder, TAG_DEFINE_FUNC); err != nil {
			return nil, err
		}
		return &DefineFuncNode{ids: ids, body: exprs}, nil
	case TAG_LET:
		b, err := openTag(decoder)
		if err != nil {
			return nil, err
		}
		if b.Name.Local != TAG_BINDINGS {
			return nil, fmt.Errorf("XML Error")
		}
		bindings := make([]LetDef, 0)
		for {
			bb, err := decoder.Token()
			if err != nil {
				return nil, err
			}
			if binding, ok := bb.(xml.StartElement); ok {
				if binding.Name.Local != TAG_BINDING {
					return nil, fmt.Errorf("XML Error")
				}
				bb, err := openTag(decoder)
				if err != nil {
					return nil, err
				}
				ii, err := xmlToNode(bb, decoder)
				if err != nil {
					return nil, err
				}
				if _, ok := ii.(*IdentifierNode); !ok {
					return nil, fmt.Errorf("XML Error")
				}

				bb, err = openTag(decoder)
				if err != nil {
					return nil, err
				}
				vv, err := xmlToNode(bb, decoder)
				if err != nil {
					return nil, err
				}
				if err = closeTag(decoder, TAG_BINDING); err != nil {
					return nil, err
				}
				bindings = append(bindings, LetDef{id: *ii.(*IdentifierNode), val: vv})
			} else if _, ok := bb.(xml.CharData); ok {
				// ignore
			} else if end, ok := bb.(xml.EndElement); ok {
				if end.Name.Local == TAG_BINDINGS {
					break
				}
			} else {
				return nil, fmt.Errorf("XML Error")
			}
		}
		e, err := openTag(decoder)
		if err != nil {
			return nil, err
		}
		if e.Name.Local != TAG_ACTIONS {
			return nil, fmt.Errorf("XML Error")
		}
		exprs, err := get_exprs_until(decoder, TAG_ACTIONS)
		if err != nil {
			return nil, err
		}
		if err = closeTag(decoder, TAG_LET); err != nil {
			return nil, err
		}
		return &LetNode{vars: bindings, body: exprs}, nil
	default:
		return nil, fmt.Errorf("Unexpected XML Tag name " + name)
	}
}

// Read a sequence of XML-encoded AstNodes from the given string.  If the
// XML is not well-formed, the behavior is undefined.
//
// @param x The string containing the XML-encoded AstNodes
//
// @return A slice of AstNodes, or an error if the XML is malformed
func readNodesFromXml(x string) ([]AstNode, error) {
	b := bytes.NewBufferString(x)
	d := xml.NewDecoder(b)
	forest := make([]AstNode, 0)
	// Get <Ast>
	t, err := d.Token()
	if t == nil && err == io.EOF {
		return nil, err
	}
	if err != nil {
		return nil, err
	}
	if elt, ok := t.(xml.StartElement); ok {
		name := elt.Name.Local
		if name != "Ast" {
			return nil, fmt.Errorf("Invalid start tag " + name)
		}
	}

	for {
		t, err := d.Token()
		if t == nil && err == io.EOF {
			break
		}
		if err != nil {
			return nil, err
		}
		// check for end of stream
		if _, ok := t.(xml.EndElement); ok {
			// Token() ensures proper matching.  We should check for EOF before
			// exiting, but we'll just exit.
			break
		}
		// Ok, it's a start tag, so go ahead and process it
		if elt, ok := t.(xml.StartElement); ok {
			n, err := xmlToNode(&elt, d)
			if err != nil {
				return nil, err
			}
			forest = append(forest, n)
		}
	}
	return forest, nil
}

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
