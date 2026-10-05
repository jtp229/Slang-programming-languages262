# An enum for disambiguating the ast node types
(
    AST_AND,
    AST_APPLY,
    AST_CALL,
    AST_BEGIN,
    AST_COND,
    AST_DEFINE_VAR,
    AST_DEFINE_FUNC,
    AST_IDENTIFIER,
    AST_IF,
    AST_LAMBDA_DEF,
    AST_LET,
    AST_OR,
    AST_QUOTE,
    AST_SET,
    AST_TICK,
    AST_BOOL_TRUE,
    AST_BOOL_FALSE,
    AST_CHAR,
    AST_CONS,
    AST_EMPTY_CONS,
    AST_DBL,
    AST_INT,
    AST_STR,
    AST_SYMBOL,
    AST_VEC,
) = range(25)


def makeBoolTrueNode():
    """
    Create a dictionary to represent a Bool True datum

    @return A dictionary representing a BoolTrue node
    """
    return {"type": AST_BOOL_TRUE}


def makeBoolFalseNode():
    """
    Create a dictionary to represent a Bool False datum

    @return A dictionary representing a BoolFalse node
    """
    return {"type": AST_BOOL_FALSE}


def makeCharNode(val):
    """
    Create a dictionary to represent a Char datum

    @param val The character value

    @return A dictionary representing a Char node
    """
    return {"type": AST_CHAR, "val": val}


def makeConsNode(car, cdr):
    """
    Create a dictionary to represent a Cons datum

    @param car The car of the cons
    @param cdr The cdr of the cons

    @return A dictionary representing a Cons node
    """
    return {"type": AST_CONS, "car": car, "cdr": cdr}


def makeEmptyConsNode():
    """
    Create a dictionary to represent an Empty Cons datum

    @return A dictionary representing an EmptyCons node
    """
    return {"type": AST_EMPTY_CONS}


def makeDblNode(val):
    """
    Create a dictionary to represent a Dbl datum

    @param val The double value

    @return A dictionary representing a Dbl node
    """
    return {"type": AST_DBL, "val": val}


def makeIntNode(val):
    """
    Create a dictionary to represent an Int datum

    @param val The integer value

    @return A dictionary representing an Int node
    """
    return {"type": AST_INT, "val": val}


def makeStrNode(val):
    """
    Create a dictionary to represent a Str datum

    @param val The string value

    @return A dictionary representing a Str node
    """
    return {"type": AST_STR, "val": val}


def makeSymbolNode(val):
    """
    Create a dictionary to represent a Symbol datum

    @param val The symbol name

    @return A dictionary representing a Symbol node
    """
    return {"type": AST_SYMBOL, "val": val}


def makeVecNode(items):
    """
    Create a dictionary to represent a Vec datum

    @param items The items in the vector

    @return A dictionary representing a Vec node
    """
    return {"type": AST_VEC, "items": items}


def makeAndNode(exprs):
    """
    Create a dictionary to represent an And node

    @param exprs The expressions to evaluate

    @return A dictionary representing an And node
    """
    return {"type": AST_AND, "exprs": exprs}


def makeApplyNode(func, args):
    """
    Create a dictionary to represent an Apply node

    @param func The function to apply
    @param args The arguments to apply it to

    @return A dictionary representing an Apply node
    """
    return {"type": AST_APPLY, "func": func, "args": args}


def makeCallNode(exprs):
    """
    Create a dictionary to represent a Call node

    @param exprs The expressions making up the call

    @return A dictionary representing a Call node
    """
    return {"type": AST_CALL, "exprs": exprs}


def makeBeginNode(exprs):
    """
    Create a dictionary to represent a Begin node

    @param exprs The expressions to evaluate sequentially

    @return A dictionary representing a Begin node
    """
    return {"type": AST_BEGIN, "exprs": exprs}


def makeCondition(test, exprs):
    """
    Create a dictionary to represent a condition (not an AST node!)

    @param test  The test expression
    @param exprs The expressions to evaluate if the test is true

    @return A dictionary representing a condition
    """
    # NB: Dummy type, to simplify the parser
    return {"type": "", "test": test, "exprs": exprs}


def makeCondNode(conditions):
    """
    Create a dictionary to represent a Cond node

    @param conditions The list of conditions

    @return A dictionary representing a Cond node
    """
    return {"type": AST_COND, "conditions": conditions}


def makeDefineVarNode(id, expr):
    """
    Create a dictionary to represent a DefineVar node

    @param id   The identifier being defined
    @param expr The expression it's bound to

    @return A dictionary representing a DefineVar node
    """
    return {"type": AST_DEFINE_VAR, "id": id, "expr": expr}


def makeDefineFuncNode(ids, body):
    """
    Create a dictionary to represent a DefineFunc node

    @param ids  The identifier list (including name and params)
    @param body The function body

    @return A dictionary representing a DefineFunc node
    """
    return {"type": AST_DEFINE_FUNC, "ids": ids, "body": body}


def makeIdentifierNode(id):
    """
    Create a dictionary to represent an Identifier node

    @param id The identifier name

    @return A dictionary representing an Identifier node
    """
    return {"type": AST_IDENTIFIER, "id": id}


def makeIfNode(cond, if_true, if_false):
    """
    Create a dictionary to represent an If node

    @param cond     The test expression
    @param if_true  The expression to evaluate if true
    @param if_false The expression to evaluate if false

    @return A dictionary representing an If node
    """
    return {"type": AST_IF, "cond": cond, "if_true": if_true, "if_false": if_false}


def makeLambdaDefNode(formals, exprs):
    """
    Create a dictionary to represent a LambdaDef node

    @param formals The list of formal parameters
    @param exprs   The function body

    @return A dictionary representing a LambdaDef node
    """
    return {"type": AST_LAMBDA_DEF, "formals": formals, "exprs": exprs}


def makeLetDef(id, val):
    """
    Create a dictionary to represent a let binding (not an AST node!)

    @param id  The identifier being bound
    @param val The expression it's bound to

    @return A dictionary representing a let binding
    """
    # NB: Dummy type, to simplify the parser
    return {"type": "", "id": id, "val": val}


def makeLetNode(vars, body):
    """
    Create a dictionary to represent a Let node

    @param vars The list of let bindings
    @param body The body of the let

    @return A dictionary representing a Let node
    """
    return {"type": AST_LET, "vars": vars, "body": body}


def makeOrNode(exprs):
    """
    Create a dictionary to represent an Or node

    @param exprs The expressions to evaluate

    @return A dictionary representing an Or node
    """
    return {"type": AST_OR, "exprs": exprs}


def makeQuoteNode(datum):
    """
    Create a dictionary to represent a Quote node

    @param datum The datum being quoted

    @return A dictionary representing a Quote node
    """
    return {"type": AST_QUOTE, "datum": datum}


def makeSetNode(id, expr):
    """
    Create a dictionary to represent a Set node

    @param id   The identifier being set
    @param expr The expression it's set to

    @return A dictionary representing a Set node
    """
    return {"type": AST_SET, "id": id, "expr": expr}


def makeTickNode(datum):
    """
    Create a dictionary to represent a Tick node

    @param datum The datum being ticked

    @return A dictionary representing a Tick node
    """
    return {"type": AST_TICK, "datum": datum}
