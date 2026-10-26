import { CallNode, AstNode, CharNode, ConsNode, DblNode, DefineVarNode, IdentifierNode, IntNode, StrNode, SymbolNode, VecNode, BoolTrueNode, BoolFalseNode, EmptyConsNode, Datum } from './AstNodes';
import { xmlUnescape } from './xmlHelpers';
import { Parser as XmlParser } from 'xml2js';

const TAG_BOOL = 'Bool';
const TAG_CALL = 'Call';
const TAG_CHAR = 'Char';
const TAG_CONS = 'Cons';
const TAG_DBL = 'Dbl';
const TAG_DEFINE_VAR = 'DefineVar';
const TAG_EMPTY = 'Empty';
const TAG_IDENTIFIER = 'Identifier';
const TAG_INT = 'Int';
const TAG_STR = 'Str';
const TAG_SYMBOL = 'Symbol';
const TAG_VECTOR = 'Vector';

/**
 * XmlParseError lets us return an error when we fail to parse the provided XML
 */
export class XmlParseError {
  /**
   * Construct an XmlParseError from a string message
   *
   * @param message The error message
   */
  constructor(readonly message: string) { }
}

/**
 * Transform an XML node into an ASTNode
 *
 * The nodes this transforms do not have much type information, and slang
 * doesn't have an XML schema, so it's necessary to do manual error checking.
 *
 * @param node The XML node to transform.  It is a node from the XML parser.
 *
 * @return an AstNode or an XmlParseError
 */
function xmlToNode(node: any): AstNode | XmlParseError {
  // TODO: This probably doesn't do enough error checking...
  let name = node['#name'],
    val = node['$']?.val + '';
  // These are the "easy" ones
  if (name === TAG_IDENTIFIER) {
    return new IdentifierNode(val);
  } else if (name === TAG_BOOL) {
    return val === 'false' ? new BoolFalseNode() : new BoolTrueNode();
  } else if (name === TAG_INT) {
    return new IntNode(parseInt(val));
  } else if (name === TAG_DBL) {
    return new DblNode(parseFloat(val));
  } else if (name === TAG_CHAR) {
    return new CharNode(xmlUnescape(val));
  } else if (name === TAG_STR) {
    return new StrNode(xmlUnescape(val));
  } else if (name === TAG_SYMBOL) {
    return new SymbolNode(val);
  } else if (name === TAG_DEFINE_VAR) {
    let id = xmlToNode(node['$$'][0]);
    if (id instanceof XmlParseError) {
      return id;
    } else if (!(id instanceof IdentifierNode)) {
      return new XmlParseError('Expected identifier');
    }
    let expr = xmlToNode(node['$$'][1]);
    if (expr instanceof XmlParseError) {
      return expr;
    }
    return new DefineVarNode(id, expr);
  } else if (name === TAG_CALL) {
    let exprs = [] as AstNode[];
    for (let e of node['$$']) {
      let expr = xmlToNode(e);
      if (expr instanceof XmlParseError) {
        return expr;
      }
      exprs.push(expr);
    }
    return new CallNode(exprs);
  } else if (name === TAG_VECTOR) {
    let data = [] as Datum[];
    if (node['$$'] !== undefined) {
      for (let d of node['$$']) {
        let datum = xmlToNode(d);
        if (datum instanceof XmlParseError) {
          return d;
        } else if (!(datum instanceof Datum)) {
          return new XmlParseError('Expected datum');
        }
        data.push(datum);
      }
    }
    return new VecNode(data);
  } else if (name === TAG_CONS) {
    let car = xmlToNode(node['$$'][0]);
    if (car instanceof XmlParseError) {
      return car;
    }
    let cdr = xmlToNode(node['$$'][1]);
    if (cdr instanceof XmlParseError) {
      return cdr;
    }
    return new ConsNode(car, cdr);
  } else if (name === TAG_EMPTY) {
    return new EmptyConsNode();
  }
  return new XmlParseError('Unexpected XML Tag name ' + name);
}

/**
 * Convert a string representation of XML into a list of tokens
 *
 * `xml` -- A string of xml
 *
 * @param xml The XML string to parse
 *
 * @return an array of AstNodes or an XmlParseError
 */
export function xmlToAst(xml: string): AstNode[] | XmlParseError {
  // The trick to simplify this is that we use the xml2js library.  When
  // configured as below, we'll receive an object that includes an array with
  // elements that represent the tokens, in order.
  let parser = new XmlParser({ explicitChildren: true, preserveChildrenOrder: true });
  let forest: AstNode[] = [];
  // NB: parseString takes a function, but it is synchronous
  parser.parseString(xml, (err, res) => {
    if (err) {
      return new XmlParseError('Invalid input file');
    }
    for (let n of res.Ast['$$']) {
      let node = xmlToNode(n);
      if (node instanceof XmlParseError) {
        return node;
      }
      forest.push(node);
    }
    return undefined;
  });
  return forest;
}
