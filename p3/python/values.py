# An enum for disambiguating the value types
(
    VAL_TRUE,
    VAL_FALSE,
    VAL_BUILTIN,
    VAL_CHAR,
    VAL_CONS,
    VAL_EMPTY,
    VAL_DBL,
    VAL_INT,
    VAL_LAMBDA,
    VAL_STR,
    VAL_SYMBOL,
    VAL_VECTOR,
    VAL_ERROR,
) = range(13)


def makeBuiltInFuncVal(name, func):
    """
    A built-in function consists of a name and a Python function object

    @param name The name of the built-in function
    @param func The Python function object

    @return A dictionary representing a built-in function value
    """
    return {"type": VAL_BUILTIN, "name": name, "func": func}


def makeConsVal(car, cdr):
    """
    A Cons cell is just two items

    @param car The car of the cons
    @param cdr The cdr of the cons

    @return A dictionary representing a cons value
    """
    return {"type": VAL_CONS, "car": car, "cdr": cdr}


def makeLambdaVal(env, func):
    """
    A Lambda *value* is just a binding of a LambdaDef to an environment

    @param env  The environment the lambda was defined in
    @param func The LambdaDef AST node

    @return A dictionary representing a lambda value
    """
    return {"type": VAL_LAMBDA, "env": env, "func": func}


def makeVectorVal(items):
    """
    A Vector consists of an array of items

    @param items The items in the vector

    @return A dictionary representing a vector value
    """
    return {"type": VAL_VECTOR, "items": items}


def makeBoolTrueVal():
    """
    A Bool(True) does not need a special value

    @return A dictionary representing a true value
    """
    return {"type": VAL_TRUE}


def makeBoolFalseVal():
    """
    A Bool(False) does not need a special value

    @return A dictionary representing a false value
    """
    return {"type": VAL_FALSE}


def makeEmptyVal():
    """
    An Empty List does not need a special value

    @return A dictionary representing an empty list value
    """
    return {"type": VAL_EMPTY}


def makeCharVal(val):
    """
    A Char value just wraps a character

    @param val The character value

    @return A dictionary representing a char value
    """
    return {"type": VAL_CHAR, "val": val}


def formatDbl(val):
    """
    Format a float for output the way Rust (the reference implementation)
    does: whole-number values are printed with no decimal point (e.g.
    `1.0` -> `"1"`), while non-whole values print their natural/minimal
    decimal representation (e.g. `2.5` -> `"2.5"`), matching Python's
    default `str()` behavior for the fractional part.

    @param val The double value to format

    @return A string representation of the value
    """
    val = float(val)
    if val.is_integer():
        return str(int(val))
    return str(val)


def makeDblVal(val):
    """
    A Dbl value just wraps a float

    @param val The double value

    @return A dictionary representing a double value
    """
    return {"type": VAL_DBL, "val": val}


def makeIntVal(val):
    """
    A Int value just wraps an integer

    @param val The integer value

    @return A dictionary representing an integer value
    """
    return {"type": VAL_INT, "val": val}


def makeStrVal(val):
    """
    A Str value just wraps a string

    @param val The string value

    @return A dictionary representing a string value
    """
    return {"type": VAL_STR, "val": val}


def makeSymbolVal(val):
    """
    A Symbol value just wraps a symbol name

    @param val The symbol name

    @return A dictionary representing a symbol value
    """
    return {"type": VAL_SYMBOL, "val": val}


def makeErrorVal(msg):
    """
    An Error value just wraps an error message

    @param msg The error message

    @return A dictionary representing an error value
    """
    return {"type": VAL_ERROR, "msg": msg}


def makeList(items):
    """
    Construct `cons` nodes from a list.

    @param items The list of items to convert

    @return A dictionary representing the head of a cons list
    """
    if len(items) == 1:
        return makeConsVal(items[0], makeEmptyVal())
    else:
        return makeConsVal(items[0], makeList(items[1:]))
