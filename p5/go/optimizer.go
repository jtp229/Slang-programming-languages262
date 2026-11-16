package main

import (
	"math"
	"unicode/utf8"
)

// This file implements the -optimize mode for slang.  It applies four passes --
// constant folding, constant propagation, strength reduction, and dead-code
// elimination -- repeatedly to a fixed point (capped at 50 iterations).
//
// The program has already passed the type checker.  Every node is tagged with
// its type, and the type checker is never re-run, because every rule replaces
// an expression with one that has the same type.  The only nodes without a
// correct tag are the ones the optimizer creates, so it tags each one as it
// creates it.

const maxOptimizeIterations = 50

// Make a new BoolTrueNode or BoolFalseNode, for a Go bool, tagged BOOL
func boolNode(b bool) AstNode {
	var n AstNode
	if b {
		n = &BoolTrueNode{}
	} else {
		n = &BoolFalseNode{}
	}
	n.SetType(BOOL)
	return n
}

// Make a new IntNode, tagged INT
func intLit(v int) AstNode {
	n := &IntNode{val: v}
	n.SetType(INT)
	return n
}

// Make a new DblNode, tagged DBL
func dblLit(v float64) AstNode {
	n := &DblNode{val: v}
	n.SetType(DBL)
	return n
}

// Make a new StrNode, tagged STR
func strLit(v string) AstNode {
	n := &StrNode{val: v}
	n.SetType(STR)
	return n
}

// Report if every node in args is an IntNode or a DblNode.  (It is true for an
// empty list.)
func allLiteralNumeric(args []AstNode) bool {
	for _, a := range args {
		switch a.(type) {
		case *IntNode, *DblNode:
			continue
		default:
			return false
		}
	}
	return true
}

// Return the value of an IntNode or DblNode, as a float64
func numAsFloat(n AstNode) float64 {
	if d, ok := n.(*DblNode); ok {
		return d.val
	}
	return float64(n.(*IntNode).val)
}

// Return the value of an IntNode.  It panics if n isn't an IntNode.
func numAsInt(n AstNode) int {
	return n.(*IntNode).val
}

// Compute the result of an arithmetic built-in the same way the run-time
// standard library does.  With one argument, `-` negates it, and the other
// operators return it unchanged.  Otherwise, the arguments are combined left
// to right.  Until the first Dbl, this uses Int arithmetic (so `/` truncates
// toward zero), and after that it uses Dbl arithmetic.  So `(/ 7 2 1.0)` is
// 3.0, not 3.5.  The result is a Dbl if any argument is a Dbl.
//
// The caller must ensure that args is non-empty and allLiteralNumeric.
//
// @param op   The operator's name (e.g., "+")
// @param args The literal arguments
//
// @return The literal that the call becomes, and false (with no literal) if
// the call would divide an Int by an Int zero or the result would be an
// infinite or NaN Dbl
func foldArith(op string, args []AstNode) (AstNode, bool) {
	if len(args) == 1 {
		if op != "-" {
			return args[0], true
		}
		if d, ok := args[0].(*DblNode); ok {
			return dblLit(-d.val), true
		}
		return intLit(-numAsInt(args[0])), true
	}

	// While allInt is true, the result so far is iRes.  After that, it is fRes.
	_, isDbl := args[0].(*DblNode)
	allInt := !isDbl
	iRes, fRes := 0, numAsFloat(args[0])
	if allInt {
		iRes = numAsInt(args[0])
	}
	for _, a := range args[1:] {
		if _, isDbl := a.(*DblNode); allInt && !isDbl {
			b := numAsInt(a)
			switch op {
			case "+":
				iRes += b
			case "-":
				iRes -= b
			case "*":
				iRes *= b
			case "/", "%":
				if b == 0 {
					return nil, false
				}
				if op == "/" {
					iRes /= b
				} else {
					iRes %= b
				}
			}
			continue
		}
		if allInt {
			allInt, fRes = false, float64(iRes)
		}
		b := numAsFloat(a)
		switch op {
		case "+":
			fRes += b
		case "-":
			fRes -= b
		case "*":
			fRes *= b
		case "/":
			fRes /= b
		case "%":
			fRes = math.Mod(fRes, b)
		}
	}
	if allInt {
		return intLit(iRes), true
	}
	// An infinite or NaN result (e.g. from dividing a Dbl by zero) is not
	// folded, since there is no literal that can represent it
	if math.IsInf(fRes, 0) || math.IsNaN(fRes) {
		return nil, false
	}
	return dblLit(fRes), true
}

// Compute the result of a comparison built-in the same way the run-time
// standard library does.
//
// The caller must ensure that args is non-empty and allLiteralNumeric.
func foldCompare(op string, args []AstNode) AstNode {
	anyFloat := false
	for _, a := range args {
		if _, ok := a.(*DblNode); ok {
			anyFloat = true
			break
		}
	}
	result := true
	if anyFloat {
		curr := numAsFloat(args[0])
		for i := 1; i < len(args); i++ {
			next := numAsFloat(args[i])
			ok := false
			switch op {
			case "<":
				ok = curr < next
			case "<=":
				ok = curr <= next
			case "=":
				ok = curr == next
			case ">":
				ok = curr > next
			case ">=":
				ok = curr >= next
			}
			if !ok {
				result = false
				break
			}
			curr = next
		}
	} else {
		curr := numAsInt(args[0])
		for i := 1; i < len(args); i++ {
			next := numAsInt(args[i])
			ok := false
			switch op {
			case "<":
				ok = curr < next
			case "<=":
				ok = curr <= next
			case "=":
				ok = curr == next
			case ">":
				ok = curr > next
			case ">=":
				ok = curr >= next
			}
			if !ok {
				result = false
				break
			}
			curr = next
		}
	}
	return boolNode(result)
}

// Return true if the node is a literal (Int, Dbl, Bool, Char, or Str)
func literal(node AstNode) bool {
	switch node.(type) {
	case *IntNode, *DblNode, *BoolTrueNode, *BoolFalseNode, *CharNode, *StrNode:
		return true
	default:
		return false
	}
}

// Return true if the node is a literal or an identifier.  This is used to determine if node can be safe propogated or eliminated
func pure(node AstNode) bool { return literal(node) || isIdentifier(node) }

// Return true if the node is an identifier
func isIdentifier(node AstNode) bool {
	_, ok := node.(*IdentifierNode)
	return ok
}

// return copy of literal node to avoid mutating the original node when propagating
func cloneLiteral(node AstNode) AstNode {
	switch value := node.(type) {
	case *IntNode:
		return intLit(value.val)
	case *DblNode:
		return dblLit(value.val)
	case *BoolTrueNode:
		return boolNode(true)
	case *BoolFalseNode:
		return boolNode(false)
	case *CharNode:
		copy := &CharNode{val: value.val}
		copy.SetType(CHAR)
		return copy
	case *StrNode:
		return strLit(value.val)
	case *IdentifierNode:
		copy := &IdentifierNode{id: value.id}
		copy.SetType(value.Type())
		return copy
	default:
		return node
	}
}

// visit each child of node, returning true if any child was changed
func rewriteChildren(node AstNode, visit func(AstNode) (AstNode, bool)) bool {
	changed := false
	apply := func(slot *AstNode) {
		updated, didChange := visit(*slot)
		*slot = updated
		changed = changed || didChange
	}
	applyList := func(nodes []AstNode) {
		for i := range nodes {
			updated, didChange := visit(nodes[i])
			nodes[i] = updated
			changed = changed || didChange
		}
	}
	switch n := node.(type) {
	case *AndNode:
		applyList(n.exprs)
	case *OrNode:
		applyList(n.exprs)
	case *CallNode:
		applyList(n.exprs)
	case *BeginNode:
		applyList(n.exprs)
	case *IfNode:
		apply(&n.cond)
		apply(&n.if_true)
		apply(&n.if_false)
	case *CondNode:
		for i := range n.conditions {
			apply(&n.conditions[i].test)
			applyList(n.conditions[i].exprs)
		}
	case *DefineVarNode:
		apply(&n.expression)
	case *DefineFuncNode:
		applyList(n.body)
	case *LambdaDefNode:
		applyList(n.body)
	case *SetNode:
		apply(&n.expression)
	case *LetNode:
		for i := range n.vars {
			apply(&n.vars[i].val)
		}
		applyList(n.body)
	case *ApplyNode:
		apply(&n.function)
		apply(&n.args)
	}
	return changed
}

// rewrite visits executable AST nodes bottom up. Quoted data and Cons/Vec
// literals are deliberately opaque to all optimizer passes.
func rewrite(node AstNode, rule func(AstNode) (AstNode, bool)) (AstNode, bool) {
	switch node.(type) {
	case *QuoteNode, *TickNode, *ConsNode, *EmptyConsNode, *VecNode:
		return rule(node)
	}
	changed := rewriteChildren(node, func(child AstNode) (AstNode, bool) {
		return rewrite(child, rule)
	})
	updated, didChange := rule(node)
	return updated, changed || didChange
}

// fold a call to built in function with literal arguments
// return the folded literal and true if successful, otherwise return the original node and false
func foldCall(node *CallNode) (AstNode, bool) {
	if len(node.exprs) == 0 {
		return node, false
	}
	op, ok := node.exprs[0].(*IdentifierNode)
	if !ok {
		return node, false
	}
	args := node.exprs[1:]
	if len(args) == 0 {
		if op.id == "string-append" {
			return strLit(""), true
		}
		return node, false
	}

	switch op.id {
	case "+", "-", "*", "/", "%":
		if allLiteralNumeric(args) {
			if folded, ok := foldArith(op.id, args); ok {
				return folded, true
			}
		}
	case "<", "<=", "=", ">", ">=":
		if allLiteralNumeric(args) {
			return foldCompare(op.id, args), true
		}
	case "not":
		if len(args) == 1 {
			switch args[0].(type) {
			case *BoolTrueNode:
				return boolNode(false), true
			case *BoolFalseNode:
				return boolNode(true), true
			}
		}
	case "string-append":
		result := ""
		for _, arg := range args {
			str, ok := arg.(*StrNode)
			if !ok {
				return node, false
			}
			result += str.val
		}
		return strLit(result), true
	case "string-length":
		if len(args) == 1 {
			if str, ok := args[0].(*StrNode); ok {
				return intLit(utf8.RuneCountInString(str.val)), true
			}
		}
	case "integer?", "double?", "number?", "symbol?", "string?", "char?", "vector?", "list?", "null?":
		if len(args) != 1 || !pure(args[0]) || args[0].Type() == RUNTIME {
			return node, false
		}
		typ := args[0].Type()
		if (op.id == "list?" || op.id == "null?") && typ == CONS {
			return node, false
		}
		match := map[string]bool{
			"integer?": typ == INT,
			"double?":  typ == DBL,
			"number?":  typ == INT || typ == DBL,
			"symbol?":  typ == SYMBOL,
			"string?":  typ == STR,
			"char?":    typ == CHAR,
			"vector?":  typ == VEC,
			"list?":    false,
			"null?":    false,
		}
		return boolNode(match[op.id]), true
	}
	return node, false
}

// apply constant folding to node and return folded node and true if succesful
func foldRule(node AstNode) (AstNode, bool) {
	switch n := node.(type) {
	case *CallNode:
		return foldCall(n)
	case *AndNode:
		if len(n.exprs) > 0 {
			if _, ok := n.exprs[0].(*BoolFalseNode); ok {
				return boolNode(false), true
			}
		}
	case *OrNode:
		if len(n.exprs) > 0 {
			if _, ok := n.exprs[0].(*BoolTrueNode); ok {
				return boolNode(true), true
			}
		}
	case *IfNode:
		switch n.cond.(type) {
		case *BoolTrueNode:
			return n.if_true, true
		case *BoolFalseNode:
			return n.if_false, true
		}
	}
	return node, false
}

// return a copy of env map to create local environment for lamba or let expressions
func copyEnv(env map[string]AstNode) map[string]AstNode {
	copy := make(map[string]AstNode, len(env))
	for name, value := range env {
		copy[name] = value
	}
	return copy
}

// apply constant propagation to each node in nodes, updating env with any new bindings
func propagateList(nodes []AstNode, env map[string]AstNode, unstable map[string]bool, topLevel bool, changed *bool) {
	for i, node := range nodes {
		updated := propagateNode(node, env, unstable, changed)
		nodes[i] = updated
		if topLevel {
			// If this is a top-level node, update the environment with any new bindings
			if define, ok := updated.(*DefineVarNode); ok {
				// If the variable is not unstable and the expression is a literal, add it to the environment
				if !unstable[define.identifier.id] && literal(define.expression) {
					env[define.identifier.id] = cloneLiteral(define.expression)
				} else {
					// If the variable is unstable or the expression is not a literal, remove it from the environment
					delete(env, define.identifier.id)
				}
			}
		}
	}
}

// apply constant propagation to node, returning the updated node
func propagateNode(node AstNode, env map[string]AstNode, unstable map[string]bool, changed *bool) AstNode {
	switch n := node.(type) {
	case *IdentifierNode:
		if value, ok := env[n.id]; ok && !unstable[n.id] {
			*changed = true
			return cloneLiteral(value)
		}
		return node
	case *QuoteNode, *TickNode, *ConsNode, *EmptyConsNode, *VecNode:
		return node
	case *DefineVarNode:
		n.expression = propagateNode(n.expression, env, unstable, changed)
		delete(env, n.identifier.id)
		return n
	case *SetNode:
		n.expression = propagateNode(n.expression, env, unstable, changed)
		delete(env, n.identifier.id)
		return n
		// create local env that shadows outer env for let lambda and define func nodes
	case *LetNode:
		local := copyEnv(env)
		for i := range n.vars {
			delete(local, n.vars[i].id.id)
		}
		outer := copyEnv(env)
		for i := range n.vars {
			n.vars[i].val = propagateNode(n.vars[i].val, outer, unstable, changed)
			if !unstable[n.vars[i].id.id] && literal(n.vars[i].val) {
				local[n.vars[i].id.id] = cloneLiteral(n.vars[i].val)
			}
		}
		propagateList(n.body, local, unstable, false, changed)
		return n
		// for begin as well
	case *BeginNode:
		local := copyEnv(env)
		propagateList(n.exprs, local, unstable, false, changed)
		return n
	case *LambdaDefNode:
		local := copyEnv(env)
		for _, formal := range n.formals {
			delete(local, formal.id)
		}
		propagateList(n.body, local, unstable, false, changed)
		return n
	case *DefineFuncNode:
		local := copyEnv(env)
		for _, formal := range n.ids[1:] {
			delete(local, formal.id)
		}
		propagateList(n.body, local, unstable, false, changed)
		return n
		//if and cond have to propogate into each branch, but they don't create a new local env
	case *IfNode:
		n.cond = propagateNode(n.cond, env, unstable, changed)
		trueEnv, falseEnv := copyEnv(env), copyEnv(env)
		n.if_true = propagateNode(n.if_true, trueEnv, unstable, changed)
		n.if_false = propagateNode(n.if_false, falseEnv, unstable, changed)
		return n
	case *CondNode:
		for i := range n.conditions {
			n.conditions[i].test = propagateNode(n.conditions[i].test, copyEnv(env), unstable, changed)
			branchEnv := copyEnv(env)
			propagateList(n.conditions[i].exprs, branchEnv, unstable, false, changed)
		}
		return n
	default:
		rewriteChildren(n, func(child AstNode) (AstNode, bool) {
			updated := propagateNode(child, env, unstable, changed)
			return updated, updated != child
		})
		return n
	}
}

// Return true if node is a numeric literal with the given value
func numericLiteral(node AstNode, value int, dbl bool) bool {
	if dbl {
		n, ok := node.(*DblNode)
		return ok && n.val == float64(value)
	}
	n, ok := node.(*IntNode)
	return ok && n.val == value
}

// apply strength reduction to node, returning the updated node and true if changed
func strengthRule(node AstNode) (AstNode, bool) {
	call, ok := node.(*CallNode)
	if !ok || len(call.exprs) != 3 {
		return node, false
	}
	op, ok := call.exprs[0].(*IdentifierNode)
	if !ok {
		return node, false
	}
	left, right := call.exprs[1], call.exprs[2]
	leftNum := left.Type() == INT || left.Type() == DBL
	rightNum := right.Type() == INT || right.Type() == DBL
	switch op.id {
	case "+":
		if leftNum && numericLiteral(right, 0, false) {
			return left, true
		}
		if rightNum && numericLiteral(left, 0, false) {
			return right, true
		}
	case "-":
		if leftNum && numericLiteral(right, 0, false) {
			return left, true
		}
	case "/":
		if leftNum && numericLiteral(right, 1, false) {
			return left, true
		}
		if left.Type() == DBL && numericLiteral(right, 1, true) {
			return left, true
		}
	case "*":
		if leftNum && numericLiteral(right, 1, false) {
			return left, true
		}
		if rightNum && numericLiteral(left, 1, false) {
			return right, true
		}
		if left.Type() == DBL && numericLiteral(right, 1, true) {
			return left, true
		}
		if right.Type() == DBL && numericLiteral(left, 1, true) {
			return right, true
		}
		if left.Type() == INT && pure(left) && numericLiteral(right, 0, false) || right.Type() == INT && pure(right) && numericLiteral(left, 0, false) {
			return intLit(0), true
		}
		if pure(left) && (left.Type() == INT || left.Type() == DBL) && numericLiteral(right, 2, false) {
			return &CallNode{baseNode: baseNode{td: left.Type()}, exprs: []AstNode{&IdentifierNode{id: "+"}, cloneLiteral(left), cloneLiteral(left)}}, true
		}
		if pure(right) && (right.Type() == INT || right.Type() == DBL) && numericLiteral(left, 2, false) {
			return &CallNode{baseNode: baseNode{td: right.Type()}, exprs: []AstNode{&IdentifierNode{id: "+"}, cloneLiteral(right), cloneLiteral(right)}}, true
		}
	}
	return node, false
}

// apply dead-code elimination to node, returning the updated node and true if changed
func deadRule(node AstNode) (AstNode, bool) {
	clean := func(body []AstNode) ([]AstNode, bool) {
		if len(body) < 2 {
			return body, false
		}
		result := make([]AstNode, 0, len(body))
		for i, expr := range body {
			if i == len(body)-1 || !pure(expr) {
				result = append(result, expr)
			}
		}
		return result, len(result) != len(body)
	}
	switch n := node.(type) {
	case *BeginNode:
		body, changed := clean(n.exprs)
		n.exprs = body
		if len(body) == 1 {
			return body[0], true
		}
		return n, changed
	case *LetNode:
		body, changed := clean(n.body)
		n.body = body
		return n, changed
	case *LambdaDefNode:
		body, changed := clean(n.body)
		n.body = body
		return n, changed
	case *DefineFuncNode:
		body, changed := clean(n.body)
		n.body = body
		return n, changed
	case *CondNode:
		changed := false
		for i := range n.conditions {
			body, didChange := clean(n.conditions[i].exprs)
			n.conditions[i].exprs = body
			changed = changed || didChange
		}
		return n, changed
	}
	return node, false
}

// apply all optimization helper functions to forest of AST nodes
func optimizeForest(forest []AstNode, unstable map[string]bool) []AstNode {

	for iteration := 0; iteration < maxOptimizeIterations; iteration++ {
		changed := false
		foldChanged := false
		for i := range forest {
			var didChange bool
			forest[i], didChange = rewrite(forest[i], foldRule)
			foldChanged = foldChanged || didChange
		}
		changed = changed || foldChanged
		// propagate changed values through the forest, updating the environment with any new bindings
		propagateChanged := false
		propagateList(forest, make(map[string]AstNode), unstable, true, &propagateChanged)
		changed = changed || propagateChanged
		// apply strength reduction and dead-code elimination to each node in the forest
		for _, rule := range []func(AstNode) (AstNode, bool){strengthRule, deadRule} {
			for i := range forest {
				var didChange bool
				forest[i], didChange = rewrite(forest[i], rule)
				changed = changed || didChange
			}
		}
		if !changed {
			break
		}
	}
	return forest
}
