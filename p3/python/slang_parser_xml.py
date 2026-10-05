from xml.dom import minidom

import astnodes


def xml_unescape(s):
    """
    Remove escape characters from a string when reading from XML

    @param s The escaped string

    @return The unescaped string
    """
    ans = ""
    in_escape = False
    for i in s:
        if not in_escape:
            if i != "\\":
                ans += i
            else:
                in_escape = True
        else:
            if i == "\\":
                ans += "\\"
                in_escape = False
            elif i == "t":
                ans += "\t"
                in_escape = False
            elif i == "n":
                ans += "\n"
                in_escape = False
            elif i == "'":
                ans += "'"
                in_escape = False
            if in_escape:
                raise RuntimeError("Invalid string?!?")
    return ans


def children(node):
    """
    Return the non-text children of `node`

    @param node The XML node

    @return An iterator over the non-text child nodes
    """
    return filter(lambda n: n.nodeType != minidom.Node.TEXT_NODE, node.childNodes)


def parseChildren(node):
    """
    Parse the children of `node`

    @param node The XML node whose children should be parsed

    @return A list of parsed AstNodes
    """
    exprs = []
    for e in children(node):
        if e.nodeType != minidom.Node.TEXT_NODE:
            exprs.append(parse_node(e))
    return exprs


def parse_node(node):
    """
    Transform an XML node into an AST Node

    @param node The XML node to parse

    @return The corresponding AstNode
    """
    val = node.attributes["val"].value if node.attributes else None
    if node.nodeType == minidom.Node.TEXT_NODE:
        return ""
    if node.localName == "Identifier":
        return astnodes.makeIdentifierNode(str(val))
    elif node.localName == "Bool" and str(val) == "false":
        return astnodes.makeBoolFalseNode()
    elif node.localName == "Bool" and str(val) == "true":
        return astnodes.makeBoolTrueNode()
    elif node.localName == "Int":
        return astnodes.makeIntNode(int(val if val else 0))
    elif node.localName == "Dbl":
        return astnodes.makeDblNode(float(val if val else 0))
    elif node.localName == "Char":
        return astnodes.makeCharNode(xml_unescape(str(val)))
    elif node.localName == "Str":
        return astnodes.makeStrNode(xml_unescape(str(val)))
    elif node.localName == "Symbol":
        return astnodes.makeSymbolNode(str(val))
    # Now we get to the harder ones...
    elif node.localName == "DefineVar":
        kids = list(children(node))
        identifier = parse_node(kids[0])
        expression = parse_node(kids[1])
        return astnodes.makeDefineVarNode(identifier, expression)
    elif node.localName == "If":
        kids = list(children(node))
        cond = parse_node(kids[0])
        if_true = parse_node(kids[1])
        if_false = parse_node(kids[2])
        return astnodes.makeIfNode(cond, if_true, if_false)
    elif node.localName == "Set":
        parts = list(children(node))  # turn the iterator into a list
        identifier = parse_node(parts[0])
        expression = parse_node(parts[1])
        return astnodes.makeSetNode(identifier, expression)
    elif node.localName == "Quote":
        parts = list(children(node))  # turn the iterator into a list
        datum = parse_node(parts[0])
        return astnodes.makeQuoteNode(datum)
    elif node.localName == "Tick":
        datum = parse_node(node.childNodes[0])
        return astnodes.makeTickNode(datum)
    elif node.localName == "Lambda":
        kids = list(children(node))
        formals = parseChildren(kids[0])
        body = parseChildren(kids[1])
        return astnodes.makeLambdaDefNode(formals, body)
    elif node.localName == "And":
        return astnodes.makeAndNode(parseChildren(node))
    elif node.localName == "Call":
        return astnodes.makeCallNode(parseChildren(node))
    elif node.localName == "Or":
        return astnodes.makeOrNode(parseChildren(node))
    elif node.localName == "Begin":
        return astnodes.makeBeginNode(parseChildren(node))
    elif node.localName == "Vector":
        return astnodes.makeVecNode(parseChildren(node))
    elif node.localName == "Cond":
        return astnodes.makeCondNode(parseChildren(node))
    elif node.localName == "Condition":
        kids = list(children(node))
        test = parse_node(list(children(kids[0]))[0])
        exprs = parseChildren(kids[1])
        return astnodes.makeCondition(test, exprs)
    elif node.localName == "Cons":
        parts = list(children(node))  # turn the iterator into a list
        car = parse_node(parts[0])
        cdr = parse_node(parts[1])
        return astnodes.makeConsNode(car, cdr)
    elif node.localName == "Empty":
        return astnodes.makeEmptyConsNode()
    elif node.localName == "Apply":
        kids = list(children(node))
        func = parse_node(kids[0])
        args = parse_node(kids[1])
        return astnodes.makeApplyNode(func, args)
    elif node.localName == "DefineFunc":
        kids = list(children(node))
        ids = parseChildren(kids[0])
        body = parseChildren(kids[1])
        return astnodes.makeDefineFuncNode(ids, body)
    elif node.localName == "Let":
        vars = []
        (bindings, actions) = list(children(node))  # turn the iterator into a list
        for binding in children(bindings):
            b = list(children(binding))
            vars.append(astnodes.makeLetDef(parse_node(b[0]), parse_node(b[1])))
        body = parseChildren(actions)
        return astnodes.makeLetNode(vars, body)
    else:
        raise RuntimeError(f"Unexpected XML Tag name {node.localName}")


def readForestFromXml(xml):
    """
    Given a string representation of an Xml file, read Ast nodes from it and
    return them as an array

    @param xml The XML string to parse

    @return A list of AstNodes
    """
    doc = minidom.parseString(xml)
    root = doc.documentElement
    res = []
    for node in children(root):
        res.append(parse_node(node))
    return res
