package main

import "fmt"

// This file implements the -typecheck mode for slang.  The checker walks each
// top-level expression and tags every node with its type.  It works bottom-up:
// to check a node, it first checks the node's children, then reads their tags
// (via Type()) to work out the node's own type.  It stops at the first warning.
//
// The checker is "sound but incomplete": whenever it can't be sure of a type,
// it uses RUNTIME, and RUNTIME never causes a warning.
//
// A warning is immediately returned as an error, so checking stops immediately.
//
// The optimizer trusts these tags, so a tag must never be wrong.  The one way a
// variable's type can change while the program runs is when the variable is
// given a new value, by `set!` or by a second `define`.  So, before checking,
// findUnstableNames finds every name where that might happen, and every use of
// those names has type RUNTIME.

// The type environment for lexical scoping.  It maps each name in one scope to
// its static type, and links to the enclosing scope.
type TypeEnv struct {
	bindings map[string]TypeDecoration // The names defined in this scope
	outer    *TypeEnv                  // The enclosing scope (nil for the global scope)
}

// Bind a name to a type in this (innermost) scope, replacing any
// existing binding for that name in this scope
//
// @param key The name to bind
// @param t   The type to give it
func (e *TypeEnv) put(key string, t TypeDecoration) {
	e.bindings[key] = t
}

// Look up a name's type, searching from this scope outward
//
// @param key The name to look up
//
// @return The name's type, or RUNTIME if it isn't bound in any scope
func (e *TypeEnv) get(key string) TypeDecoration {
	t, ok := e.bindings[key]
	if ok {
		return t
	}
	if e.outer != nil {
		return e.outer.get(key)
	}
	return RUNTIME
}

// Create the global type environment: the math constants have type DBL, and
// every built-in function has type LAMBDA.
//
// Note that this list only gives the built-ins their LAMBDA type.  The
// parameter and result types used to check calls to them come from
// getBuiltinSignature.
//
// @return A new global type environment
func makeDefaultTypeEnv() *TypeEnv {
	env := &TypeEnv{bindings: make(map[string]TypeDecoration), outer: nil}
	// Constants
	env.put("pi", DBL)
	env.put("e", DBL)
	env.put("tau", DBL)
	env.put("inf+", DBL)
	env.put("inf-", DBL)
	env.put("nan", DBL)

	// Builtins
	builtins := []string{
		"+", "-", "*", "/", "%", ">", "<", ">=", "<=", "=",
		"abs", "sqrt", "sin", "cos", "tan", "asin", "acos", "atan", "sinh", "cosh", "tanh", "log10", "loge", "pow",
		"integer?", "double?", "number?", "symbol?", "procedure?", "string?", "char?", "vector?", "list?", "null?", "nan?", "not",
		"integer->double", "double->integer", "integer->string", "double->string",
		"car", "cdr", "cons", "list", "set-car!", "set-cdr!",
		"string-append", "string-length", "substring", "string-char-at", "char-equal?", "char->integer", "integer->char", "char->string", "string->integer", "string->double",
		"vector", "vector-init", "vector-length", "vector-get", "vector-set!",
		"display",
	}
	for _, b := range builtins {
		env.put(b, LAMBDA)
	}
	return env
}

// Create a new, empty scope nested inside another one
//
// @param outer The enclosing scope
//
// @return A new type environment whose outer scope is `outer`
func makeInnerTypeEnv(outer *TypeEnv) *TypeEnv {
	return &TypeEnv{bindings: make(map[string]TypeDecoration), outer: outer}
}

// The parameters and result of a built-in function.  Each parameter has a list
// of allowed types (RUNTIME is always allowed, since it means "not known until
// run time").
type Signature struct {
	inputs   [][]TypeDecoration // The allowed types of each required parameter
	variadic []TypeDecoration   // The allowed types of any extra arguments (nil if none are allowed)
	output   TypeDecoration     // The type of the result
	numeric  bool               // If true, the result is INT or DBL, depending on the arguments
	passArg  bool               // If true, the result has the type of the (only) argument
}

// Shorthands for the allowed types of a parameter.  When an argument doesn't
// match, the warning names the first type in the list, which is why tNUM
// starts with INT.  tANY is every type (except RUNTIME, which `allows`
// accepts anyway).
var (
	tNUM  = []TypeDecoration{INT, DBL}
	tINT  = []TypeDecoration{INT}
	tDBL  = []TypeDecoration{DBL}
	tSTR  = []TypeDecoration{STR}
	tCHAR = []TypeDecoration{CHAR}
	tCONS = []TypeDecoration{CONS}
	tVEC  = []TypeDecoration{VEC}
	tANY  = []TypeDecoration{BOOL, CHAR, CONS, DBL, INT, LAMBDA, STR, SYMBOL, VEC}
)

// Implement the "built-in signatures" table from the spec. Variadic built-ins
// have no required inputs, so they accept zero arguments.
//
// @param name The name of the function being called
//
// @return The built-in's signature, or nil if `name` isn't a built-in
func getBuiltinSignature(name string) *Signature {
	switch name {
	case "+", "-", "*", "/", "%":
		return &Signature{inputs: [][]TypeDecoration{}, variadic: tNUM, numeric: true}
	case ">", "<", ">=", "<=", "=":
		return &Signature{inputs: [][]TypeDecoration{}, variadic: tNUM, output: BOOL}
	case "abs":
		return &Signature{inputs: [][]TypeDecoration{tNUM}, numeric: true}
	case "sqrt", "sin", "cos", "tan", "asin", "acos", "atan", "sinh", "cosh", "tanh", "log10", "loge":
		return &Signature{inputs: [][]TypeDecoration{tNUM}, output: DBL}
	case "pow":
		return &Signature{inputs: [][]TypeDecoration{tNUM, tNUM}, output: DBL}
	case "integer?", "double?", "number?", "symbol?", "procedure?", "string?", "char?", "vector?", "list?", "null?", "nan?", "not":
		return &Signature{inputs: [][]TypeDecoration{tANY}, output: BOOL}
	case "integer->double":
		return &Signature{inputs: [][]TypeDecoration{tINT}, output: DBL}
	case "double->integer":
		return &Signature{inputs: [][]TypeDecoration{tDBL}, output: INT}
	case "integer->string":
		return &Signature{inputs: [][]TypeDecoration{tINT}, output: STR}
	case "double->string":
		return &Signature{inputs: [][]TypeDecoration{tDBL}, output: STR}
	case "car", "cdr":
		return &Signature{inputs: [][]TypeDecoration{tCONS}, output: RUNTIME}
	case "cons":
		return &Signature{inputs: [][]TypeDecoration{tANY, tANY}, output: CONS}
	case "list":
		return &Signature{inputs: [][]TypeDecoration{}, variadic: tANY, output: CONS}
	case "set-car!", "set-cdr!":
		return &Signature{inputs: [][]TypeDecoration{tCONS, tANY}, output: RUNTIME}
	case "string-append":
		return &Signature{inputs: [][]TypeDecoration{}, variadic: tSTR, output: STR}
	case "string-length":
		return &Signature{inputs: [][]TypeDecoration{tSTR}, output: INT}
	case "substring":
		return &Signature{inputs: [][]TypeDecoration{tSTR, tINT, tINT}, output: STR}
	case "string-char-at":
		return &Signature{inputs: [][]TypeDecoration{tSTR, tINT}, output: CHAR}
	case "char-equal?":
		return &Signature{inputs: [][]TypeDecoration{tCHAR, tCHAR}, output: BOOL}
	case "char->integer":
		return &Signature{inputs: [][]TypeDecoration{tCHAR}, output: INT}
	case "integer->char":
		return &Signature{inputs: [][]TypeDecoration{tINT}, output: CHAR}
	case "char->string":
		return &Signature{inputs: [][]TypeDecoration{tCHAR}, output: STR}
	case "string->integer":
		return &Signature{inputs: [][]TypeDecoration{tSTR}, output: INT}
	case "string->double":
		return &Signature{inputs: [][]TypeDecoration{tSTR}, output: DBL}
	case "vector":
		return &Signature{inputs: [][]TypeDecoration{}, variadic: tANY, output: VEC}
	case "vector-init":
		return &Signature{inputs: [][]TypeDecoration{tINT, tANY}, output: VEC}
	case "vector-length":
		return &Signature{inputs: [][]TypeDecoration{tVEC}, output: INT}
	case "vector-get":
		return &Signature{inputs: [][]TypeDecoration{tVEC, tINT}, output: RUNTIME}
	case "vector-set!":
		return &Signature{inputs: [][]TypeDecoration{tVEC, tINT, tANY}, output: RUNTIME}
	case "display":
		// `display` returns its argument
		return &Signature{inputs: [][]TypeDecoration{tANY}, passArg: true}
	}
	return nil
}

// Find identifier names whose type can't be trusted, because the variable they
// name might be given a value of a different type while the program runs.  A
// name is unstable if:
//   - It is the target of a `set!` anywhere in the program.
//   - It is defined by a `define` that is not itself a top-level expression.
//     In a function body, a `lambda` made before an inner `(define x ...)`
//     needs this conservative rule.
//   - It is defined by more than one top-level `define`.
//
// @param forest The program's top-level expressions
//
// @return The set of unstable names
func findUnstableNames(forest []AstNode) map[string]bool {
	//mark every name assigned by set
	setNames := make(map[string]bool)
	//mark a name defined by a define nested inside another expression or body
	innerDefineNames := make(map[string]bool)
	//count direct top level defines by name, mark names defined more than once
	defineCounts := make(map[string]int)
	// Walk the AST, marking names as unstable when they are assigned or defined
	var walk func(AstNode, bool)
	walk = func(node AstNode, topLevel bool) {
		// record a name defined by a define in the appropriate map
		recordDefinition := func(name string) {
			if topLevel {
				defineCounts[name]++
			} else {
				innerDefineNames[name] = true
			}
		}

		switch n := node.(type) {
		case *ConsNode:
			walk(n.car, false)
			walk(n.cdr, false)
		case *VecNode:
			for _, item := range n.items {
				walk(item, false)
			}
		case *AndNode:
			for _, expression := range n.exprs {
				walk(expression, false)
			}
		case *CallNode:
			for _, expression := range n.exprs {
				walk(expression, false)
			}
		case *BeginNode:
			for _, expression := range n.exprs {
				walk(expression, false)
			}
		case *Condition:
			walk(n.test, false)
			for _, expression := range n.exprs {
				walk(expression, false)
			}
		case *CondNode:
			for index := range n.conditions {
				walk(&n.conditions[index], false)
			}
		case *DefineVarNode:
			recordDefinition(n.identifier.id)
			walk(n.expression, false)
		case *DefineFuncNode:
			if len(n.ids) > 0 {
				recordDefinition(n.ids[0].id)
			}
			for _, expression := range n.body {
				walk(expression, false)
			}
		case *IfNode:
			walk(n.cond, false)
			walk(n.if_true, false)
			walk(n.if_false, false)
		case *LambdaDefNode:
			for _, expression := range n.body {
				walk(expression, false)
			}
		case *OrNode:
			for _, expression := range n.exprs {
				walk(expression, false)
			}
		case *QuoteNode:
			walk(n.datum, false)
		case *SetNode:
			setNames[n.identifier.id] = true
			walk(n.expression, false)
		case *TickNode:
			walk(n.datum, false)
		case *LetDef:
			walk(n.val, false)
		case *LetNode:
			for index := range n.vars {
				walk(&n.vars[index], false)
			}
			for _, expression := range n.body {
				walk(expression, false)
			}
		case *ApplyNode:
			walk(n.function, false)
			walk(n.args, false)
		}
	}

	for _, expr := range forest {
		walk(expr, true)
	}

	unstable := make(map[string]bool)
	for name, count := range defineCounts {
		if count > 1 {
			unstable[name] = true
		}
	}

	for name := range setNames {
		unstable[name] = true
	}
	for name := range innerDefineNames {
		unstable[name] = true
	}
	return unstable
}

// Check one top-level expression.  The environment is shared across calls, so a
// top-level `define` is visible to later expressions.
//
// @param expr     The top-level expression to check
// @param env      The global type environment
// @param unstable The program's unstable names (from findUnstableNames)
//
// @return The type of `expr`, and an error holding the first warning (or nil)

// Helper function to check a list of expressions, returning the type of the last one
func checkExpressions(expressions []AstNode, env *TypeEnv, unstable map[string]bool) (TypeDecoration, error) {
	typ := RUNTIME
	for _, expression := range expressions {
		var err error
		typ, err = typecheckAst(expression, env, unstable)
		if err != nil {
			return RUNTIME, err
		}
	}
	return typ, nil
}

// Helper function to check the body of a lambda, given the types of its arguments
func checkLambdaBody(formals []IdentifierNode, body []AstNode, argumentTypes []TypeDecoration, env *TypeEnv, unstable map[string]bool) (TypeDecoration, error) {
	local := makeInnerTypeEnv(env)
	for index := range formals {
		typ := RUNTIME
		if index < len(argumentTypes) {
			typ = argumentTypes[index]
		}
		formals[index].SetType(typ)
		local.put(formals[index].id, typ)
	}
	return checkExpressions(body, local, unstable)
}

// Check whether a type is allowed for a parameter,  RUNTIME is always allowed.
func typeAllowed(actual TypeDecoration, allowed []TypeDecoration) bool {
	if actual == RUNTIME {
		return true
	}
	for _, typ := range allowed {
		if actual == typ {
			return true
		}
	}
	return false
}

func typecheckAst(expr AstNode, env *TypeEnv, unstable map[string]bool) (TypeDecoration, error) {
	typ := RUNTIME

	switch n := expr.(type) {
	// Literal nodes
	case *IntNode:
		typ = INT
	case *DblNode:
		typ = DBL
	case *BoolTrueNode, *BoolFalseNode:
		typ = BOOL
	case *CharNode:
		typ = CHAR
	case *StrNode:
		typ = STR
	case *SymbolNode:
		typ = SYMBOL
	case *EmptyConsNode:
		typ = CONS

	// A names type is unknown if it can change at runtime
	case *IdentifierNode:
		if unstable[n.id] {
			typ = RUNTIME
		} else {
			typ = env.get(n.id)
		}

	// Check children first, stop on the first warning
	case *ConsNode:
		if _, err := typecheckAst(n.car, env, unstable); err != nil {
			return RUNTIME, err
		}
		if _, err := typecheckAst(n.cdr, env, unstable); err != nil {
			return RUNTIME, err
		}
		typ = CONS

	case *VecNode:
		for _, item := range n.items {
			if _, err := typecheckAst(item, env, unstable); err != nil {
				return RUNTIME, err
			}
		}
		typ = VEC

	// Quoted data isn't checked as an expression
	case *QuoteNode, *TickNode:
		typ = RUNTIME

	case *AndNode:
		// Check each expression in order; the result is known only if all arm types match.
		typ = RUNTIME
		if len(n.exprs) > 0 {
			if _, err := checkExpressions(n.exprs, env, unstable); err != nil {
				return RUNTIME, err
			}
			firstType := n.exprs[0].Type()
			sameType := true
			for _, expression := range n.exprs[1:] {
				if expression.Type() != firstType {
					sameType = false
					break
				}
			}
			if sameType {
				typ = firstType
			}
		}
	case *OrNode:
		// The type of an or is the type of its first expression, if all expressions have the same type
		typ = RUNTIME
		if len(n.exprs) > 0 {
			if _, err := checkExpressions(n.exprs, env, unstable); err != nil {
				return RUNTIME, err
			}
			firstType := n.exprs[0].Type()
			sameType := true
			for _, expression := range n.exprs[1:] {
				if expression.Type() != firstType {
					sameType = false
					break
				}
			}
			if sameType {
				typ = firstType
			}
		}
	case *BeginNode:
		// Check each expression in order, and return the type of the last one
		var err error
		typ, err = checkExpressions(n.exprs, env, unstable)
		if err != nil {
			return RUNTIME, err
		}
	case *IfNode:
		// Check the condition and both branches, then check that the branches have the same type
		if _, err := typecheckAst(n.cond, env, unstable); err != nil {
			return RUNTIME, err
		}
		trueType, err := typecheckAst(n.if_true, env, unstable)
		if err != nil {
			return RUNTIME, err
		}
		falseType, err := typecheckAst(n.if_false, env, unstable)
		if err != nil {
			return RUNTIME, err
		}
		if trueType != RUNTIME && falseType != RUNTIME && trueType != falseType {
			return RUNTIME, fmt.Errorf("Warning: Type mismatch in if branches: %s and %s", trueType, falseType)
		}
		if trueType == falseType {
			typ = trueType
		}
	case *CondNode:
		if len(n.conditions) == 0 {
			return RUNTIME, fmt.Errorf("Warning: cond does not have a final #t case")
		}
		if _, ok := n.conditions[len(n.conditions)-1].test.(*BoolTrueNode); !ok {
			return RUNTIME, fmt.Errorf("Warning: cond does not have a final #t case")
		}
		for index := range n.conditions {
			condition := &n.conditions[index]
			if _, err := typecheckAst(condition.test, env, unstable); err != nil {
				return RUNTIME, err
			}
			branchType, err := checkExpressions(condition.exprs, env, unstable)
			if err != nil {
				return RUNTIME, err
			}
			condition.SetType(branchType)
			if index == 0 {
				typ = branchType
			} else if typ != RUNTIME && branchType != RUNTIME && typ != branchType {
				return RUNTIME, fmt.Errorf("Warning: Type mismatch in cond branches: %s and %s", typ, branchType)
			}
		}
	case *DefineVarNode:
		// Check the value, then bind the identifier in the current scope.
		valueType, err := typecheckAst(n.expression, env, unstable)
		if err != nil {
			return RUNTIME, err
		}
		n.identifier.SetType(valueType)
		env.put(n.identifier.id, valueType)
		typ = valueType
	case *DefineFuncNode:
		// Bind the function before checking its body so recursive calls resolve.
		typ = LAMBDA
		if len(n.ids) > 0 {
			n.ids[0].SetType(LAMBDA)
			env.put(n.ids[0].id, LAMBDA)
			_, err := checkLambdaBody(n.ids[1:], n.body, nil, env, unstable)
			if err != nil {
				return RUNTIME, err
			}
		} else {
			if _, err := checkExpressions(n.body, env, unstable); err != nil {
				return RUNTIME, err
			}
		}
	case *LambdaDefNode:
		if _, err := checkLambdaBody(n.formals, n.body, nil, env, unstable); err != nil {
			return RUNTIME, err
		}
		typ = LAMBDA
	case *SetNode:
		valueType, err := typecheckAst(n.expression, env, unstable)
		if err != nil {
			return RUNTIME, err
		}
		n.identifier.SetType(RUNTIME)
		typ = valueType
	case *LetNode:
		// Check the values of the bindings, then check the body in a new environment that binds the identifiers to those types
		valueTypes := make([]TypeDecoration, len(n.vars))
		for index := range n.vars {
			valueType, err := typecheckAst(n.vars[index].val, env, unstable)
			if err != nil {
				return RUNTIME, err
			}
			valueTypes[index] = valueType
			n.vars[index].SetType(valueType)
			n.vars[index].id.SetType(valueType)
		}
		local := makeInnerTypeEnv(env)
		for index := range n.vars {
			local.put(n.vars[index].id.id, valueTypes[index])
		}
		var err error
		typ, err = checkExpressions(n.body, local, unstable)
		if err != nil {
			return RUNTIME, err
		}
	case *ApplyNode:
		// Check the function and the arguments, then check that the arguments are a list
		if _, err := typecheckAst(n.function, env, unstable); err != nil {
			return RUNTIME, err
		}
		argsType, err := typecheckAst(n.args, env, unstable)
		if err != nil {
			return RUNTIME, err
		}
		if argsType != CONS && argsType != RUNTIME {
			return RUNTIME, fmt.Errorf("Warning: Apply expects a list of arguments, found %s", argsType)
		}
	case *CallNode:
		// Check the operator and arguments, then check that the operator is a built-in or a lambda, and that the arguments match its signature
		if len(n.exprs) == 0 {
			break
		}
		operator := n.exprs[0]
		arguments := n.exprs[1:]
		if identifier, ok := operator.(*IdentifierNode); ok {
			if signature := getBuiltinSignature(identifier.id); signature != nil {
				if _, err := typecheckAst(operator, env, unstable); err != nil {
					return RUNTIME, err
				}
				argumentCount := len(arguments)
				requiredCount := len(signature.inputs)
				if argumentCount < requiredCount || signature.variadic == nil && argumentCount != requiredCount {
					return RUNTIME, fmt.Errorf("Warning: Wrong number of arguments for %s: expected %d, found %d", identifier.id, requiredCount, argumentCount)
				}
				argumentTypes := make([]TypeDecoration, len(arguments))
				for index, argument := range arguments {
					argumentType, err := typecheckAst(argument, env, unstable)
					if err != nil {
						return RUNTIME, err
					}
					argumentTypes[index] = argumentType
					allowed := signature.variadic
					if index < requiredCount {
						allowed = signature.inputs[index]
					}
					if !typeAllowed(argumentType, allowed) {
						return RUNTIME, fmt.Errorf("Warning: Type mismatch in call to %s: argument %d expected %s, found %s", identifier.id, index+1, allowed[0], argumentType)
					}
				}
				switch {
				case signature.passArg:
					typ = argumentTypes[0]
				case signature.numeric:
					typ = INT
					for _, argumentType := range argumentTypes {
						if argumentType == RUNTIME {
							typ = RUNTIME
							break
						}
						if argumentType == DBL {
							typ = DBL
						}
					}
				default:
					typ = signature.output
				}
				break
			}
		}
		// If the operator is a lambda, check that the arguments match its formals
		if lambda, ok := operator.(*LambdaDefNode); ok {
			if _, err := typecheckAst(operator, env, unstable); err != nil {
				return RUNTIME, err
			}
			if len(arguments) != len(lambda.formals) {
				return RUNTIME, fmt.Errorf("Warning: Wrong number of arguments for lambda: expected %d, found %d", len(lambda.formals), len(arguments))
			}
			argumentTypes := make([]TypeDecoration, len(arguments))
			for index, argument := range arguments {
				argumentType, err := typecheckAst(argument, env, unstable)
				if err != nil {
					return RUNTIME, err
				}
				argumentTypes[index] = argumentType
			}
			var err error
			typ, err = checkLambdaBody(lambda.formals, lambda.body, argumentTypes, env, unstable)
			if err != nil {
				return RUNTIME, err
			}
			lambda.SetType(LAMBDA)
			break
		}
		if _, err := typecheckAst(operator, env, unstable); err != nil {
			return RUNTIME, err
		}
		if _, err := checkExpressions(arguments, env, unstable); err != nil {
			return RUNTIME, err
		}
	default:
		typ = RUNTIME
	}

	expr.SetType(typ)
	return typ, nil
}
