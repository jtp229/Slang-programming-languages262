from values import *


class Env:
    """An environment/scope in which variables are defined"""

    def __init__(self, outer):
        """
        Construct an environment.  We need a single global value for false,
        for true, and for empty.  We optionally have a link to an enclosing
        scope

        @param outer The enclosing environment, or None
        """
        self.__outer = outer
        self.__map = {}

    def put(self, key: str, val):
        """
        Unconditionally put a key into this environment

        @param key The variable name
        @param val The value to bind to the name
        """
        self.__map[key] = val

    def get(self, key: str):
        """
        Look up the value for a given key; recurse to outer environments as
        needed.  Return an error on failure.

        @param key The variable name to look up

        @return The value bound to the name, or an ErrorValue
        """
        return self.__map[key] if key in self.__map.keys() else self.__outerGet(key)

    def __outerGet(self, key):
        """
        Look up the key in the outer scope

        @param key The variable name to look up

        @return The value bound to the name, or an ErrorValue
        """
        if not self.__outer:
            return makeErrorVal("Run-time Error: Invalid identifier")
        return self.__outer.get(key)

    def update(self, key: str, val):
        """
        Update a key's value **in the scope where it is defined**

        @param key The variable name to update
        @param val The new value
        """
        if self.__map.get(key) != None:
            self.__map[key] = val
        else:
            self.__outer.update(key, val)


def makeDefaultEnv():
    """
    Create a default environment by mapping the built-in functions into it

    NB: The standard library is a later assignment.  For now, the only
        built-ins are `+` and `=`, which are enough to test function calls,
        conditionals, and recursion.

    @return An environment populated with the built-in functions
    """
    e = Env(None)
    addBuiltins(e)
    return e


def makeInnerEnv(outer: Env):
    """
    Create an inner scope that links to its parent

    @param outer The parent environment

    @return A new environment linked to the parent
    """
    return Env(outer)


def addBuiltins(env):
    """
    Add the built-in functions to the given environment

    @param env The environment to add functions to
    """
    env.put("+", makeBuiltInFuncVal("+", add))
    env.put("=", makeBuiltInFuncVal("=", numEq))


def add(args):
    """
    Add one or more numbers.  The result is an Int if all of the arguments are
    Ints, and a Dbl if any of them is a Dbl.

    @param args The numbers to add

    @return The sum, or an ErrorValue if there are no arguments or any argument
            is not a number
    """
    if len(args) == 0:
        return makeErrorVal("Run-time Error: Invalid number of arguments")
    for arg in args:
        if arg["type"] not in [VAL_INT, VAL_DBL]:
            return makeErrorVal("Run-time Error: Argument is not a number")
    res = args[0]["val"]
    for arg in args[1:]:
        res += arg["val"]
    if any(arg["type"] == VAL_DBL for arg in args):
        return makeDblVal(res)
    return makeIntVal(res)


def numEq(args):
    """
    Check whether one or more numbers are all equal

    @param args The numbers to compare

    @return #t if the numbers are all equal, #f otherwise, or an ErrorValue if
            there are no arguments or any argument is not a number
    """
    if len(args) == 0:
        return makeErrorVal("Run-time Error: Invalid number of arguments")
    for arg in args:
        if arg["type"] not in [VAL_INT, VAL_DBL]:
            return makeErrorVal("Run-time Error: Argument is not a number")
    first = args[0]["val"]
    for arg in args[1:]:
        if first != arg["val"]:
            return makeBoolFalseVal()
    return makeBoolTrueVal()
