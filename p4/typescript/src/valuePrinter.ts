import { BoolFalse, BoolTrue, BuiltInFunc, Char, Cons, Dbl, EmptyCons, Int, InterpretError, Str, Symbol, Value, Vec } from './Values';

/**
 * Pretty print AST Value nodes
 *
 * NB:  This only considers Bool, Int, Dbl, Str, Vec, Symbol, Char, Cons,
 *      and BuiltInFunc nodes.
 *
 * @param node The Value to print
 *
 * @return A string representation of the Value
 */
export function printValue(node: Value | InterpretError): string {
  if (node instanceof InterpretError) {
    return node.message;
  } else if (node instanceof BoolTrue) {
    return '#t';
  } else if (node instanceof BoolFalse) {
    return '#f';
  } else if (node instanceof Int) {
    return node.val + '';
  } else if (node instanceof Dbl) {
    return node.val + '';
  } else if (node instanceof Str) {
    return node.val;
  } else if (node instanceof Vec) {
    return '#(' + node.items.map((v) => printValue(v)).join(' ') + ')';
  } else if (node instanceof Symbol) {
    return "'" + node.name;
  } else if (node instanceof Char) {
    if (node.val === ' ') return '#\\space';
    if (node.val === '\n') return '#\\newline';
    if (node.val === '\t') return '#\\tab';
    return '#\\' + node.val;
  } else if (node instanceof BuiltInFunc) {
    return 'Built-in Function: ' + node.name;
  } else if (node instanceof EmptyCons) {
    return '()';
  } else if (node instanceof Cons) {
    return '(' + printValue(node.car) + ' . ' + printValue(node.cdr) + ')';
  }
  console.log(node);
  return 'Unreachable?';
}
