import { CallNode, AstNode, CharNode, ConsNode, DblNode, DefineVarNode, IdentifierNode, IntNode, StrNode, SymbolNode, VecNode, BoolTrueNode, BoolFalseNode, EmptyConsNode } from './AstNodes';
import { Env } from './Env';
import { BoolFalse, BoolTrue, BuiltInFunc, Char, Cons, Dbl, EmptyCons, Int, InterpretError, Str, Symbol, Value, Vec } from './Values';

/**
 * Interpret an AST node in the context of an environment
 *
 * @param expr An expression to evaluate
 * @param env  The environment in which to do the evaluation
 *
 * @return A value, or an InterpretError
 */
export function interpretAst(expr: AstNode, env: Env) {
  // NB:  In this phase of the assignment, we don't need a full interpreter for
  //      all of the language.
  if (expr instanceof DefineVarNode) {
    return interpretDefineVar(expr, env);
  } else if (expr instanceof CallNode) {
    return interpretCall(expr, env);
  } else if (expr instanceof IdentifierNode) {
    return env.get(expr.id) ?? new InterpretError('Run-time Error: Invalid identifier');
  } else if (expr instanceof ConsNode) {
    return interpretCons(expr, env);
  } else if (expr instanceof VecNode) {
    return interpretVec(expr, env);
  } else if (expr instanceof BoolTrueNode) {
    return new BoolTrue();
  } else if (expr instanceof BoolFalseNode) {
    return new BoolFalse();
  } else if (expr instanceof IntNode) {
    return new Int(expr.val);
  } else if (expr instanceof DblNode) {
    return new Dbl(expr.val);
  } else if (expr instanceof StrNode) {
    return new Str(expr.val);
  } else if (expr instanceof CharNode) {
    return new Char(expr.val);
  } else if (expr instanceof SymbolNode) {
    return new Symbol(expr.name);
  } else if (expr instanceof EmptyConsNode) {
    return new EmptyCons();
  } else {
    return new InterpretError('Run-time Error: Unrecognized AST node');
  }
}

/**
 * Visit a `define` of a variable by binding the value of its expression in the
 * current environment
 *
 * @return The value that was bound
 */
function interpretDefineVar(expr: DefineVarNode, env: Env): Value | InterpretError {
  let val = interpretAst(expr.expression, env);
  if (val instanceof InterpretError) {
    return val;
  }
  env.put(expr.identifier.id, val);
  return val;
}

/**
 * Visit a call node by evaluating the first expression to get a function,
 * evaluating the rest to get its arguments, and then calling the function
 *
 * @return The result of calling the function
 */
function interpretCall(expr: CallNode, env: Env): Value | InterpretError {
  let func = interpretAst(expr.exprs[0], env);
  if (func instanceof InterpretError) {
    return func;
  } else if (!(func instanceof BuiltInFunc)) {
    return new InterpretError('Run-time Error: Argument is not a procedure');
  }
  let args: Value[] = [];
  for (let argExpr of expr.exprs.slice(1)) {
    let arg = interpretAst(argExpr, env);
    if (arg instanceof InterpretError) {
      return arg;
    }
    args.push(arg);
  }

  return func.func(args);
}

/** Visit a cons by evaluating its car and cdr */
function interpretCons(expr: ConsNode, env: Env): Value | InterpretError {
  let car = interpretAst(expr.car, env);
  if (car instanceof InterpretError) {
    return car;
  }
  let cdr = interpretAst(expr.cdr, env);
  if (cdr instanceof InterpretError) {
    return cdr;
  }
  return new Cons(car, cdr);
}

/** Visit a vector by evaluating each of its items */
function interpretVec(expr: VecNode, env: Env): Value | InterpretError {
  let items: Value[] = [];
  for (let e of expr.items) {
    let v = interpretAst(e, env);
    if (v instanceof InterpretError) {
      return v;
    }
    items.push(v);
  }
  return new Vec(items);
}
