from xml.dom import minidom
from slang_scanner import *


def xml_escape(s):
    """Escape a string for outputting it to XML as an attribute.

    Args:
        s: The string to escape.

    Returns:
        The escaped string.
    """
    ans = ""
    for i in s:
        if i == "\\":
            ans += "\\\\"
        elif i == "\t":
            ans += "\\t"
        elif i == "\n":
            ans += "\\n"
        elif i == "'":
            ans += "\\'"
        else:
            ans += i
    return ans


# A mapping from TokenTypes to the tag names we use in the XML
token_tag_map = {
    TOK_ABBREV: "AbbrevToken",
    TOK_AND: "AndToken",
    TOK_APPLY: "ApplyToken",
    TOK_BEGIN: "BeginToken",
    TOK_BOOL: "BoolToken",
    TOK_CHAR: "CharToken",
    TOK_COND: "CondToken",
    TOK_DBL: "DblToken",
    TOK_DEFINE: "DefineToken",
    TOK_DOT: "DotToken",
    TOK_EOF: "EofToken",
    TOK_IDENTIFIER: "IdentifierToken",
    TOK_IF: "IfToken",
    TOK_INT: "IntToken",
    TOK_LAMBDA: "LambdaToken",
    TOK_LEFT_PAREN: "LeftParenToken",
    TOK_LET: "LetToken",
    TOK_OR: "OrToken",
    TOK_QUOTE: "QuoteToken",
    TOK_RIGHT_PAREN: "RightParenToken",
    TOK_SET: "SetToken",
    TOK_STR: "StrToken",
    TOK_VECTOR: "VecToken",
    TOK_ERROR: "ErrorToken",
}


def to_xml_string(tokens):
    """Produce an XML representation of the given array of tokens.

    Args:
        tokens: The list of Tokens to serialize.

    Returns:
        A string containing the XML representation.
    """

    doc = minidom.Document()
    root = doc.createElement("Tokens")
    root.setAttribute("xmlns", "tokens")
    doc.appendChild(root)

    for token in tokens:
        tag = token_tag_map.get(token.tok_type)
        if not tag:
            raise RuntimeError("Unexpected tag")
        elt = doc.createElement(tag)
        if token.tok_type != TOK_EOF:
            elt.setAttribute("col", str(token.col))
            elt.setAttribute("line", str(token.line))
            if token.tok_type in [TOK_STR, TOK_ERROR, TOK_CHAR]:
                elt.setAttribute("val", xml_escape(token.literal))
            elif token.tok_type == TOK_IDENTIFIER:
                elt.setAttribute("val", xml_escape(token.text))
            elif token.tok_type == TOK_BOOL:
                elt.setAttribute("val", "true" if token.literal else "false")
            elif token.tok_type in [TOK_DBL, TOK_INT]:
                elt.setAttribute("val", str(token.literal))
        root.appendChild(elt)
        if token.tok_type == TOK_ERROR:
            return token.literal

    xml_str = root.toprettyxml(indent=" ")
    return xml_str.strip()
