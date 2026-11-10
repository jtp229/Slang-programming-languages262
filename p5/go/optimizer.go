package main

import "math"

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

// Apply the four optimization passes in a fixedpoint worklist.
//
// @param forest   The program's top-level expressions, already type checked
// @param unstable The program's unstable names (from findUnstableNames)
//
// @return The optimized forest
func optimizeForest(forest []AstNode, unstable map[string]bool) []AstNode {
	// TODO: implement this
	panic("optimizeForest is not implemented")
}
