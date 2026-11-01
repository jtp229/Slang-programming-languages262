import { Env } from './Env';
import {
  BoolFalse,
  BoolTrue,
  BuiltInFunc,
  Char,
  Cons,
  Dbl,
  EmptyCons,
  Int,
  InterpretError,
  Str,
  Symbol,
  Value,
  Vec,
} from './Values';
import { printValue } from './valuePrinter';

declare const process: { stdout: { write(value: string): boolean } };

/**
 * Add the standard library to the given environment.
 *
 * NB: For now, the only built-ins are `+` and `=`.  Each is written out "the
 *     long way", as a function from an array of argument Values to a result
 *     Value (or an InterpretError).  The rest of the library, which is
 *     described in docs/StdLib.md, is your job.
 *
 * @param env The environment to add functions to
 */
export function addStdLib(env: Env) {
  env.put('+', new BuiltInFunc('+', add));
  env.put('=', new BuiltInFunc('=', numEq));
  env.put('display', new BuiltInFunc('display', display));
  env.put('integer?', new BuiltInFunc('integer?', isInteger));
  env.put('double?', new BuiltInFunc('double?', isDouble));
  env.put('number?', new BuiltInFunc('number?', isNumber));
  env.put('string?', new BuiltInFunc('string?', isString));
  env.put('char?', new BuiltInFunc('char?', isCharacter));
  env.put('symbol?', new BuiltInFunc('symbol?', isSymbol));
  env.put('vector?', new BuiltInFunc('vector?', isVector));
  env.put('procedure?', new BuiltInFunc('procedure?', isProcedure));
  env.put('list?', new BuiltInFunc('list?', isList));
  env.put('null?', new BuiltInFunc('null?', isNull));
  env.put('not', new BuiltInFunc('not', not));
}

/**
 * Add one or more numbers.  The result is an Int if all of the arguments are
 * Ints, and a Dbl if any of them is a Dbl.
 *
 * @param args The numbers to add
 *
 * @return The sum, or an error if there are no arguments or any argument is
 *         not a number
 */
function add(args: Value[]): Value | InterpretError {
  if (args.length === 0) {
    return new InterpretError('Run-time Error: Wrong number of arguments');
  }
  let anyDbl = false;
  let sum = 0;
  for (let a of args) {
    if (a instanceof Int) {
      sum += a.val;
    } else if (a instanceof Dbl) {
      anyDbl = true;
      sum += a.val;
    } else {
      return new InterpretError('Run-time Error: Argument is not a number');
    }
  }
  return anyDbl ? new Dbl(sum) : new Int(sum);
}

/**
 * Check that one or more numbers are all equal, by numeric value.  Each pair of
 * adjacent numbers is compared, and the answer is #f as soon as one pair is not
 * equal.
 *
 * @param args The numbers to compare
 *
 * @return #t if all of the arguments are equal, #f otherwise, or an error if
 *         there are no arguments or any argument is not a number
 */
function numEq(args: Value[]): Value | InterpretError {
  if (args.length === 0) {
    return new InterpretError('Run-time Error: Wrong number of arguments');
  }
  // Check all of the types first, so that a type error is reported even if an
  // earlier pair already differs
  let nums: number[] = [];
  for (let a of args) {
    if (a instanceof Int || a instanceof Dbl) {
      nums.push(a.val);
    } else {
      return new InterpretError('Run-time Error: Argument is not a number');
    }
  }
  for (let i = 1; i < nums.length; ++i) {
    if (nums[i - 1] !== nums[i]) {
      return new BoolFalse();
    }
  }
  return new BoolTrue();
}

function display(args: Value[]): Value | InterpretError {
  if (args.length !== 1) {
    return new InterpretError('Run-time Error: Wrong number of arguments');
  }
  const val = args[0];

  process.stdout.write(printValue(val));
  return val;
}


type ArgumentType =
  | 'Number'
  | typeof Int
  | typeof Dbl
  | typeof Str
  | typeof Char
  | typeof Cons
  | typeof Vec;

const TYPE_ERRORS: Record<string, string> = {
  Int: 'Argument is not an integer',
  Dbl: 'Argument is not a double',
  Str: 'Argument is not a string',
  Char: 'Argument is not a character',
  Cons: 'Argument is not a cons cell',
  Vec: 'Argument is not a vector',
};

// Check argument count and any declared types from left to right
function checkArgs(args: Value[], count: number, types: ArgumentType[]): InterpretError | null {
  // 1. Check number of arguments
  if (count === -1) {
    if (args.length < 1) {
      return new InterpretError('Run-time Error: Wrong number of arguments');
    }
  } else if (args.length !== count) {
    return new InterpretError('Run-time Error: Wrong number of arguments');
  }

  // 2. Check types from left to right
  for (let i = 0; i < args.length; i++) {
    const expected = types[i];
    const val = args[i];

    if (expected === 'Number') {
      if (!(val instanceof Int || val instanceof Dbl)) {
        return new InterpretError('Run-time Error: Argument is not a number');
      }
    } else if (expected) {
      if (!(val instanceof expected)) {
        const message = TYPE_ERRORS[expected.name] || 'Invalid argument';
        return new InterpretError(`Run-time Error: ${message}`);
      }
    }
  }

  return null; // No error
}
function typePredicate(args: Value[], predicate: (value: Value) => boolean): Value | InterpretError {
  const error = checkArgs(args, 1, []);
  if (error) {
    return error;
  }
  return predicate(args[0]) ? new BoolTrue() : new BoolFalse();
}

// Type predicates
function isInteger(args: Value[]): Value | InterpretError {
  return typePredicate(args, value => value instanceof Int);
}

function isDouble(args: Value[]): Value | InterpretError {
  return typePredicate(args, value => value instanceof Dbl);
}

function isNumber(args: Value[]): Value | InterpretError {
  return typePredicate(args, value => value instanceof Int || value instanceof Dbl);
}

function isString(args: Value[]): Value | InterpretError {
  return typePredicate(args, value => value instanceof Str);
}

function isCharacter(args: Value[]): Value | InterpretError {
  return typePredicate(args, value => value instanceof Char);
}

function isSymbol(args: Value[]): Value | InterpretError {
  return typePredicate(args, value => value instanceof Symbol);
}

function isVector(args: Value[]): Value | InterpretError {
  return typePredicate(args, value => value instanceof Vec);
}

function isProcedure(args: Value[]): Value | InterpretError {
  return typePredicate(args, value => value instanceof BuiltInFunc);
}

function isList(args: Value[]): Value | InterpretError {
  return typePredicate(args, value => {
    let tail = value;
    while (tail instanceof Cons) {
      tail = tail.cdr;
    }
    return tail instanceof EmptyCons;
  });
}

function isNull(args: Value[]): Value | InterpretError {
  return typePredicate(args, value => value instanceof EmptyCons);
}

function not(args: Value[]): Value | InterpretError {
  return typePredicate(args, value => value instanceof BoolFalse);
}
