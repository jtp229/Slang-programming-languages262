package main

import (
	"fmt"
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
