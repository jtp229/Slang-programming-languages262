
/** InterpretError provides a way of returning an error message */
export class InterpretError {
  /**
   * Construct an error from a message
   *
   * @param message The error message
   */
  constructor(readonly message: string) { }
}

/** A Bool representing "true" */
export class BoolTrue { }

/** A Bool representing "false" */
export class BoolFalse { }

/** BuiltInFunc is for built in function values */
export class BuiltInFunc {
  /**
   * Construct a BuiltInFunc
   *
   * @param name The name of this built-in function, for debugging
   * @param func The function
   */
  constructor(public name: string, public func: (args: Value[]) => Value | InterpretError) { }
}

/** Char is for character values */
export class Char {
  /**
   * Construct a Char
   *
   * @param val The character value
   */
  constructor(public val: string) { }
}

/** Cons is a pair (cons cell), which is a value */
export class Cons {
  /**
   * Construct a Cons
   *
   * @param car The first value of the pair
   * @param cdr The second value of the pair
   */
  constructor(public car: Value, public cdr: Value) { }
}

/** An empty list */
export class EmptyCons { }

/** Dbl is for double values */
export class Dbl {
  /**
   * Construct a Dbl
   *
   * @param val The double value
   */
  constructor(public val: number) { }
}

/** Int is for integer values */
export class Int {
  /**
   * Construct an Int
   *
   * @param val The integer value
   */
  constructor(public val: number) { }
}

/** Str is for string values */
export class Str {
  /**
   * Construct a Str
   *
   * @param val The string value
   */
  constructor(public val: string) { }
}

/** Symbol is for Scheme Symbols, which are values */
export class Symbol {
  /**
   * Construct a Symbol
   *
   * @param name The name of this symbol
   */
  constructor(public name: string) { }
}

/** Vec is for vector values */
export class Vec {
  /**
   * Construct a Vec
   *
   * @param items The list of values in this vector
   */
  constructor(public items: Value[]) { }
}

/** Value is a union of all the Value types used by the interpreter */
export type Value = BoolTrue | BoolFalse | BuiltInFunc | Char | Cons | EmptyCons | Dbl | Int | Str | Symbol | Vec;
