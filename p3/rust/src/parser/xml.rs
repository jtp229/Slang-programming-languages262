use super::ast::{
    AndNode, ApplyNode, AstNode, AstNodeDetails, BeginNode, CallNode, CondNode, ConditionNode,
    ConsNode, Datum, DefineFuncNode, DefineVarNode, IdentifierNode, IfNode, LambdaDefNode,
    LetDefNode, LetNode, OrNode, QuoteNode, SetNode, TickNode, VecNode,
};
use minidom::Element;

static TAG_AND: &str = "And";
static TAG_APPLY: &str = "Apply";
static TAG_BEGIN: &str = "Begin";
static TAG_BOOL: &str = "Bool";
static TAG_CALL: &str = "Call";
static TAG_CHAR: &str = "Char";
static TAG_COND: &str = "Cond";
static TAG_CONDITION: &str = "Condition";
static TAG_CONS: &str = "Cons";
static TAG_DBL: &str = "Dbl";
static TAG_DEFINE_FUNC: &str = "DefineFunc";
static TAG_DEFINE_VAR: &str = "DefineVar";
static TAG_EMPTY: &str = "Empty";
static TAG_IDENTIFIER: &str = "Identifier";
static TAG_IF: &str = "If";
static TAG_INT: &str = "Int";
static TAG_LAMBDA: &str = "Lambda";
static TAG_LET: &str = "Let";
static TAG_OR: &str = "Or";
static TAG_QUOTE: &str = "Quote";
static TAG_SET: &str = "Set";
static TAG_STR: &str = "Str";
static TAG_SYMBOL: &str = "Symbol";
static TAG_TICK: &str = "Tick";
static TAG_VECTOR: &str = "Vector";

/// Remove escape characters from a string when reading from XML
///
/// @param s The string to unescape
///
/// @return The unescaped string
pub fn xml_unescape(s: &str) -> String {
    // We need to go char by char to get it right
    let mut ans = String::new();
    let mut in_escape = false;
    for i in s.bytes() {
        if !in_escape {
            if i != b'\\' {
                ans.push(i as char);
            } else {
                in_escape = true;
            }
        } else {
            if i == b'\\' {
                ans += "\\";
                in_escape = false;
            } else if i == b't' {
                ans += "\t";
                in_escape = false;
            } else if i == b'n' {
                ans += "\n";
                in_escape = false;
            } else if i == b'\'' {
                ans += "'";
                in_escape = false;
            }
            if in_escape {
                panic!("Invalid string?!?");
            }
        }
    }
    ans
}

/// Transform an XML node into an ASTNode
///
/// @param node A node from the xml parser
///
/// @return The corresponding AstNode
fn xml_to_node(node: &Element) -> AstNode {
    // TODO: This probably doesn't do enough error checking...
    let ns = "ast";
    let val = node.attr("val");
    // These are the "easy" ones
    let kind = if node.is(TAG_IDENTIFIER, ns) {
        AstNodeDetails::Identifier(IdentifierNode {
            id: val.unwrap().to_string(),
        })
    } else if node.is(TAG_BOOL, ns) {
        if val.unwrap() == "false" {
            AstNodeDetails::Datum(Datum::BoolFalse)
        } else {
            AstNodeDetails::Datum(Datum::BoolTrue)
        }
    } else if node.is(TAG_INT, ns) {
        AstNodeDetails::Datum(Datum::Int(val.unwrap().parse::<i64>().unwrap()))
    } else if node.is(TAG_DBL, ns) {
        AstNodeDetails::Datum(Datum::Dbl(val.unwrap().parse::<f64>().unwrap()))
    } else if node.is(TAG_CHAR, ns) {
        AstNodeDetails::Datum(Datum::Char(
            xml_unescape(val.unwrap()).chars().next().unwrap(),
        ))
    } else if node.is(TAG_STR, ns) {
        AstNodeDetails::Datum(Datum::Str(xml_unescape(val.unwrap())))
    } else if node.is(TAG_SYMBOL, ns) {
        AstNodeDetails::Datum(Datum::Symbol(val.unwrap().to_string()))
    } else if node.is(TAG_DEFINE_VAR, ns) {
        let identifier_node = xml_to_node(node.children().next().unwrap());
        let AstNodeDetails::Identifier(identifier) = identifier_node.details else {
            panic!("Xml Error");
        };
        let expression = Box::new(xml_to_node(node.children().nth(1).unwrap()));
        AstNodeDetails::DefineVar(DefineVarNode {
            id: identifier,
            expr: expression,
        })
    } else if node.is(TAG_IF, ns) {
        let cond = Box::new(xml_to_node(node.children().next().unwrap()));
        let if_true = Box::new(xml_to_node(node.children().nth(1).unwrap()));
        let if_false = Box::new(xml_to_node(node.children().nth(2).unwrap()));
        AstNodeDetails::If(IfNode {
            cond,
            if_true,
            if_false,
        })
    } else if node.is(TAG_SET, ns) {
        let identifier_node = xml_to_node(node.children().next().unwrap());
        let AstNodeDetails::Identifier(identifier) = identifier_node.details else {
            panic!("Xml Error");
        };
        let expression = Box::new(xml_to_node(node.children().nth(1).unwrap()));
        AstNodeDetails::Set(SetNode {
            id: identifier,
            expr: expression,
        })
    } else if node.is(TAG_QUOTE, ns) {
        let datum_node = xml_to_node(node.children().next().unwrap());
        let AstNodeDetails::Datum(datum) = datum_node.details else {
            panic!("Xml Error");
        };
        AstNodeDetails::Quote(QuoteNode { datum })
    } else if node.is(TAG_TICK, ns) {
        let datum_node = xml_to_node(node.children().next().unwrap());
        let AstNodeDetails::Datum(datum) = datum_node.details else {
            panic!("Xml Error");
        };
        AstNodeDetails::Tick(TickNode { datum })
    } else if node.is(TAG_LAMBDA, ns) {
        let mut formals = Vec::<IdentifierNode>::new();
        for f in node.children().next().unwrap().children() {
            let id_node = xml_to_node(f);
            let AstNodeDetails::Identifier(id) = id_node.details else {
                panic!("Expected identifier, found {}", node.name());
            };
            formals.push(id);
        }
        let mut body = Vec::<AstNode>::new();
        for e in node.children().nth(1).unwrap().children() {
            body.push(xml_to_node(e));
        }
        AstNodeDetails::LambdaDef(LambdaDefNode {
            formals,
            exprs: body,
        })
    } else if node.is(TAG_AND, ns) {
        let mut exprs = Vec::<AstNode>::new();
        for e in node.children() {
            exprs.push(xml_to_node(e));
        }
        AstNodeDetails::And(AndNode { exprs })
    } else if node.is(TAG_CALL, ns) {
        let mut exprs = Vec::<AstNode>::new();
        for e in node.children() {
            exprs.push(xml_to_node(e));
        }
        AstNodeDetails::Call(CallNode { exprs })
    } else if node.is(TAG_OR, ns) {
        let mut exprs = Vec::<AstNode>::new();
        for e in node.children() {
            exprs.push(xml_to_node(e));
        }
        AstNodeDetails::Or(OrNode { exprs })
    } else if node.is(TAG_BEGIN, ns) {
        let mut exprs = Vec::<AstNode>::new();
        for e in node.children() {
            exprs.push(xml_to_node(e));
        }
        AstNodeDetails::Begin(BeginNode { exprs })
    } else if node.is(TAG_VECTOR, ns) {
        let mut items = Vec::<Datum>::new();
        for e in node.children() {
            let datum_node = xml_to_node(e);
            let AstNodeDetails::Datum(datum) = datum_node.details else {
                panic!("Xml Error!");
            };
            items.push(datum);
        }
        AstNodeDetails::Datum(Datum::Vec(Box::new(VecNode { items })))
    } else if node.is(TAG_COND, ns) {
        let mut conditions = Vec::<ConditionNode>::new();
        for c in node.children() {
            let cond_node = xml_to_node(c);
            let AstNodeDetails::Condition(c) = cond_node.details else {
                panic!("Xml Error");
            };
            conditions.push(c);
        }
        AstNodeDetails::Cond(CondNode { conditions })
    } else if node.is(TAG_CONDITION, ns) {
        let test = Box::new(xml_to_node(
            node.children().next().unwrap().children().next().unwrap(),
        ));
        let mut exprs = Vec::<AstNode>::new();
        for a in node.children().nth(1).unwrap().children() {
            exprs.push(xml_to_node(a));
        }
        AstNodeDetails::Condition(ConditionNode { test, exprs })
    } else if node.is(TAG_CONS, ns) {
        let car_node = xml_to_node(node.children().next().unwrap());
        let AstNodeDetails::Datum(car) = car_node.details else {
            panic!("Xml Error!");
        };
        let cdr_node = xml_to_node(node.children().nth(1).unwrap());
        let AstNodeDetails::Datum(cdr) = cdr_node.details else {
            panic!("Xml Error!");
        };
        AstNodeDetails::Datum(Datum::Cons(Box::new(ConsNode { car, cdr })))
    } else if node.is(TAG_EMPTY, ns) {
        AstNodeDetails::Datum(Datum::EmptyCons)
    } else if node.is(TAG_APPLY, ns) {
        let func = Box::new(xml_to_node(node.children().next().unwrap()));
        let args = Box::new(xml_to_node(node.children().nth(1).unwrap()));
        AstNodeDetails::Apply(ApplyNode { func, args })
    } else if node.is(TAG_DEFINE_FUNC, ns) {
        let mut ids = Vec::new();
        for i in node.children().next().unwrap().children() {
            let id_node = xml_to_node(i);
            let AstNodeDetails::Identifier(id) = id_node.details else {
                panic!("Xml Error");
            };
            ids.push(id);
        }
        let mut body = Vec::new();
        for e in node.children().nth(1).unwrap().children() {
            body.push(xml_to_node(e));
        }
        AstNodeDetails::DefineFunc(DefineFuncNode { ids, body })
    } else if node.is(TAG_LET, ns) {
        let mut vars = Vec::<LetDefNode>::new();
        for b in node.children().next().unwrap().children() {
            let id_node = xml_to_node(b.children().next().unwrap());
            let AstNodeDetails::Identifier(id) = id_node.details else {
                panic!("Xml Error!");
            };
            let val = Box::new(xml_to_node(b.children().nth(1).unwrap()));
            vars.push(LetDefNode { id, val })
        }
        let mut body = Vec::<AstNode>::new();
        for e in node.children().nth(1).unwrap().children() {
            body.push(xml_to_node(e));
        }
        AstNodeDetails::Let(LetNode { vars, body })
    } else {
        panic!("Unexpected XML Tag name {}", node.name())
    };
    AstNode::new(kind)
}

/// Convert a string representation of XML into a list of AST nodes
///
/// @param xml A string of xml
///
/// @return A vector of AstNodes
pub fn xml_to_ast(xml: String) -> Vec<AstNode> {
    let root: minidom::Element = xml.parse().unwrap();
    root.children().map(xml_to_node).collect()
}
