/** The set of types that can be assigned to an AstNode */
export enum TypeDecoration { RUNTIME, INT, DBL, BOOL, CHAR, STR, SYMBOL, CONS, VEC, LAMBDA, VOID }

/** AstNode is the base class for all nodes in our abstract syntax tree */
export class AstNode {
  public datum_type: TypeDecoration = TypeDecoration.RUNTIME;
}

/** Datum is a useful sub-class of AstNode, for values */
export class Datum extends AstNode { }

/** A Bool node representing "true" */
export class BoolTrueNode extends Datum { }

/** A Bool node representing "false" */
export class BoolFalseNode extends Datum { }

/** CharNode is for character values */
export class CharNode extends Datum {
  /**
   * Construct a CharNode
   *
   * @param val The character value in this node
   */
  constructor(public val: string) {
    super();
  }
}

/** ConsNode is a pair (cons cell), which is a value */
export class ConsNode extends Datum {
  /**
   * Construct a ConsNode
   *
   * @param car The first value of the pair
   * @param cdr The second value of the pair
   */
  constructor(public car: Datum, public cdr: Datum) {
    super();
  }
}

/** A Node representing an empty list */
export class EmptyConsNode extends Datum { }

/** DblNode is for double values */
export class DblNode extends Datum {
  /**
   * Construct a DblNode
   *
   * @param val The double value in this node
   */
  constructor(public val: number) {
    super();
  }
}

/** IntNode is for integer values */
export class IntNode extends Datum {
  /**
   * Construct an IntNode
   *
   * @param val The integer value in this node
   */
  constructor(public val: number) {
    super();
  }
}

/** StrNode is for string values */
export class StrNode extends Datum {
  /**
   * Construct a StringNode
   *
   * @param val The string value in this node
   */
  constructor(public val: string) {
    super();
  }
}

/** SymbolNode is for Scheme Symbols, which are values */
export class SymbolNode extends Datum {
  /**
   * Construct a SymbolNode
   *
   * @param name The name of this symbol
   */
  constructor(public name: string) {
    super();
  }
}

/** VecNode is for vector values */
export class VecNode extends Datum {
  /**
   * Construct a VecNode
   *
   * @param items The list of values in this vector
   */
  constructor(public items: Datum[]) {
    super();
  }
}

/** ParseError is *not* an AstNode.  We use it to report errors. */
export class ParseError {
  /**
   * Construct an ErrorNode
   *
   * @param message The text of the error message
   */
  constructor(public message: string) { }
}

/** CallNode is the default form */
export class CallNode extends AstNode {
  /**
   * Construct a CALL node
   *
   * @param exprs The set of expressions that comprise this call
   */
  constructor(public exprs: AstNode[]) {
    super();
  }
}

/** DefineVarNode is for the DEFINE_VAR special form */
export class DefineVarNode extends AstNode {
  /**
   * Construct a DefineVarNode
   *
   * @param identifier The identifier whose value is being defined
   * @param expression The expression to evaluate to get a value
   */
  constructor(public identifier: IdentifierNode, public expression: AstNode) {
    super();
  }
}

/** IdentifierNode is for identifiers (names that are bound to values) */
export class IdentifierNode extends AstNode {
  /**
   * Construct an IdentifierNode
   *
   * @param id The name associated with this identifier
   */
  constructor(public id: string) {
    super();
  }
}
