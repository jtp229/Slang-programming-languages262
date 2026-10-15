from astnodes import *
import slang_env
from values import *


def id_name(node):

   # Extract a variable name string from an Identifier AST node or a bare string
    if isinstance(node, dict) and "id" in node and node.get("type") == AST_IDENTIFIER:
        return node["id"]
    return node


def interpret_ast(in_expr, in_env):
    """
    Interpret an AstNode in the context of an Environment

    @param in_expr The AST node to interpret
    @param in_env  The environment to interpret it in

    @return The result of the interpretation (a value)
    """
    expr = in_expr
    env = in_env

    while True:
        node_type = expr["type"]

        # Handle constants
        if node_type == AST_BOOL_TRUE:
            return makeBoolTrueVal()
        elif node_type == AST_BOOL_FALSE:
            return makeBoolFalseVal()
        elif node_type == AST_INT:
            return makeIntVal(expr["val"])
        elif node_type == AST_DBL:
            return makeDblVal(expr["val"])
        elif node_type == AST_STR:
            return makeStrVal(expr["val"])
        elif node_type == AST_CHAR:
            return makeCharVal(expr["val"])
        elif node_type == AST_SYMBOL:
            return makeSymbolVal(expr["val"])
        elif node_type == AST_EMPTY_CONS:
            return makeEmptyVal()

        # handle data structures
        elif node_type == AST_CONS:
            car_val = interpret_ast(expr["car"], env)
            if isErrorVal(car_val):
                return car_val
            cdr_val = interpret_ast(expr["cdr"], env)
            if isErrorVal(cdr_val):
                return cdr_val
            return makeConsVal(car_val, cdr_val)

        elif node_type == AST_VEC:
            items = []
            for item in expr["items"]:
                val = interpret_ast(item, env)
                if isErrorVal(val):
                    return val
                items.append(val)
            return makeVectorVal(items)

        # handle identifier lookup
        elif node_type == AST_IDENTIFIER:
            return env.get(expr["id"])

        # QUOTE / TICK: evaluate the datum (tail)
        elif node_type == AST_QUOTE or node_type == AST_TICK:
            expr = expr["datum"]
            continue

        # DEFINE: as a variable (define id expr)
        elif node_type == AST_DEFINE_VAR:
            val = interpret_ast(expr["expr"], env)
            if isErrorVal(val):
                return val
            env.put(id_name(expr["id"]), val)
            return val

        # DEFINE: as a function (define (id+) expr+)
        elif node_type == AST_DEFINE_FUNC:
            lambda_val = makeLambdaVal(env, expr)
            env.put(id_name(expr["ids"][0]), lambda_val)
            return lambda_val

        # LAMBDA: (lambda (id*) expr+)
        elif node_type == AST_LAMBDA_DEF:
            return makeLambdaVal(env, expr)

        # IF: (if test consequent alternate) — branch is tail
        elif node_type == AST_IF:
            test_val = interpret_ast(expr["cond"], env)
            if isErrorVal(test_val):
                return test_val
            expr = expr["if_true"] if isTruthy(test_val) else expr["if_false"]
            continue

        # AND: (and expr+) — last expr is tail
        elif node_type == AST_AND:
            exprs = expr["exprs"]
            for e in exprs[:-1]:
                result = interpret_ast(e, env)
                if isErrorVal(result):
                    return result
                if not isTruthy(result):
                    return result
            if not exprs:
                return makeBoolTrueVal()
            expr = exprs[-1]
            continue

        # OR: (or expr+) — last expr is tail
        elif node_type == AST_OR:
            exprs = expr["exprs"]
            for e in exprs[:-1]:
                result = interpret_ast(e, env)
                if isErrorVal(result):
                    return result
                if isTruthy(result):
                    return result
            if not exprs:
                return makeBoolFalseVal()
            expr = exprs[-1]
            continue

        # BEGIN: (begin expr+) — last expr is tail
        elif node_type == AST_BEGIN:
            result = eval_prefix(expr["exprs"], env)
            if result is not None:
                return result
            expr = expr["exprs"][-1]
            continue

        # COND: (cond (test expr+)+) 
        elif node_type == AST_COND:
            matched = None
            for condition in expr["conditions"]:
                test_val = interpret_ast(condition["test"], env)
                if isErrorVal(test_val):
                    return test_val
                if isTruthy(test_val):
                    matched = condition["exprs"]
                    break
            if matched is None:
                return makeErrorVal("Run-time Error: No true condition in cond")
            result = eval_prefix(matched, env)
            if result is not None:
                return result
            expr = matched[-1]
            continue

        # LET: (let ((id expr)+) expr+) — body last expr is tail
        elif node_type == AST_LET:
            let_env = slang_env.makeInnerEnv(env)
            for binding in expr["vars"]:
                val = interpret_ast(binding["val"], env)
                if isErrorVal(val):
                    return val
                let_env.put(id_name(binding["id"]), val)
            result = eval_prefix(expr["body"], let_env)
            if result is not None:
                return result
            expr = expr["body"][-1]
            env = let_env
            continue

        # SET!: (set! id expr)
        elif node_type == AST_SET:
            val = interpret_ast(expr["expr"], env)
            if isErrorVal(val):
                return val
            env.update(id_name(expr["id"]), val)
            return val

        # APPLY: (apply func args)
        elif node_type == AST_APPLY:
            func_val = interpret_ast(expr["func"], env)
            if isErrorVal(func_val):
                return func_val
            args_val = interpret_ast(expr["args"], env)
            if isErrorVal(args_val):
                return args_val
            args = consToArray(args_val)
            if args is None:
                return makeErrorVal("Run-time Error: apply arguments must be a list")

        # CALL (func arg+)
        elif node_type == AST_CALL:
            func_val = interpret_ast(expr["exprs"][0], env)
            if isErrorVal(func_val):
                return func_val
            args = []
            for i in range(1, len(expr["exprs"])):
                arg_val = interpret_ast(expr["exprs"][i], env)
                if isErrorVal(arg_val):
                    return arg_val
                args.append(arg_val)

        else:
            return makeErrorVal(f"Run-time Error: Unknown AST node type: {node_type}")

        if node_type in (AST_CALL, AST_APPLY):
            if func_val["type"] == VAL_BUILTIN:
                return func_val["func"](args)
            if func_val["type"] != VAL_LAMBDA:
                return makeErrorVal(f"Run-time Error: Cannot call {func_val['type']}")

            lambda_func = func_val["func"]
            if lambda_func["type"] == AST_LAMBDA_DEF:
                formals = [id_name(f) for f in lambda_func["formals"]]
                body = lambda_func["exprs"]
            elif lambda_func["type"] == AST_DEFINE_FUNC:
                formals = [id_name(f) for f in lambda_func["ids"][1:]]
                body = lambda_func["body"]
            else:
                return makeErrorVal("Run-time Error: Invalid lambda")

            if len(args) != len(formals):
                return makeErrorVal("Run-time Error: Invalid number of arguments")

            lambda_env = slang_env.makeInnerEnv(func_val["env"])
            for i, formal in enumerate(formals):
                lambda_env.put(formal, args[i])

            result = eval_prefix(body, lambda_env)
            if result is not None:
                return result
            expr = body[-1]
            env = lambda_env
            continue


def eval_prefix(exprs, env):
    
    #Evaluate all but the last expression in a sequence
    if not exprs:
        return makeEmptyVal()
    for e in exprs[:-1]:
        result = interpret_ast(e, env)
        if isErrorVal(result):
            return result
    return None


def val_to_scheme(val):
    """
    Pretty print values

    @param val The Value to print

    @return A string representation of the value
    """
    if val["type"] == VAL_TRUE:
        return "#t"
    elif val["type"] == VAL_FALSE:
        return "#f"
    elif val["type"] == VAL_INT:
        return str(val["val"])
    elif val["type"] == VAL_DBL:
        return formatDbl(val["val"])
    elif val["type"] == VAL_STR:
        return val["val"]
    elif val["type"] == VAL_VECTOR:
        res = "#("
        for e in val["items"]:
            res = res + str(val_to_scheme(e)) + " "
        return res[: len(res) - 1] + ")"
    elif val["type"] == VAL_SYMBOL:
        return f"'{val['val']}"
    elif val["type"] == VAL_CHAR:
        if val["val"] == " ":
            return "#\\space"
        elif val["val"] == "\n":
            return "#\\newline"
        elif val["val"] == "\t":
            return "#\\tab"
        else:
            return f"#\\{val['val']}"
    elif val["type"] == VAL_BUILTIN:
        return f"Built-in Function: {val['name']}"
    elif val["type"] == VAL_LAMBDA:
        return "Lambda Function"
    elif val["type"] == VAL_EMPTY:
        return "()"
    elif val["type"] == VAL_ERROR:
        return val["msg"]
    elif val["type"] == VAL_CONS:
       # items = consToArray(val)
       # if items is not None:
         #   return "(" + " ".join(val_to_scheme(item) for item in items) + ")"
         #I want to print lists more nicely but if I do I fail 3 tests, so comment this out
        l = val_to_scheme(val["car"])
        r = val_to_scheme(val["cdr"])
        return f"({l} . {r})"
    else:
        return f"<unknown value type: {val['type']}>"


def isErrorVal(val):
     #check if value is error value
     return val["type"] == VAL_ERROR

def isTruthy(val):
    #determine if value is true in slang
    return val["type"] != VAL_FALSE

def consToArray(cons_val):
    result = []
    current = cons_val

    while current["type"] == VAL_CONS:
        result.append(current["car"])
        current = current["cdr"]

    if current["type"] == VAL_EMPTY:
        return result
    else:
        return None  # Not a proper list



