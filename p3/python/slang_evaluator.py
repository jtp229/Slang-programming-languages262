from astnodes import makeLambdaDefNode
import astnodes
import slang_env
from values import *


def interpret_ast(in_expr, in_env):
    """
    Interpret an AstNode in the context of an Environment

    @param in_expr The AST node to interpret
    @param in_env  The environment to interpret it in

    @return The result of the interpretation (a value)
    """

    return makeErrorVal("interpret_ast is not implemented")


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
        #  TODO:  Right now, we print CONS cells in the most naive way. Consider
        #         alternatives that are more pretty for lists??
        l = val_to_scheme(val["car"])
        r = val_to_scheme(val["cdr"])
        return f"({l} . {r})"
