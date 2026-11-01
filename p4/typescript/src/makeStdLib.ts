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
  env.put('abs', new BuiltInFunc('abs', abs));
  env.put('sqrt', new BuiltInFunc('sqrt', sqrt));
  env.put('sin', new BuiltInFunc('sin', sin));
  env.put('cos', new BuiltInFunc('cos', cos));
  env.put('loge', new BuiltInFunc('loge', loge));
  env.put('log10', new BuiltInFunc('log10', log10));
  env.put('integer->double', new BuiltInFunc('integer->double', integerToDouble));
  env.put('char->string', new BuiltInFunc('char->string', charToString));
  env.put('char->integer', new BuiltInFunc('char->integer', charToInteger));
  env.put('double->integer', new BuiltInFunc('double->integer', doubleToInteger));
  env.put('string-length', new BuiltInFunc('string-length', stringLength));
  env.put('string-append', new BuiltInFunc('string-append', stringAppend));
  env.put('substring', new BuiltInFunc('substring', substring));
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

//math functions

//abs
function abs(args: Value[]): Value | InterpretError {
  //Validate
  const error = checkArgs(args, 1, ['Number']);
  if (error) {
    return error;
  }

  const val = args[0] 
  if(val instanceof Int || val instanceof Dbl){

  
  const result = Math.abs(val.val);

  return (val instanceof Int) ? new Int(result) : new Dbl(result);
  }
  return new InterpretError('Run-time Error: Argument is not a number');
}

function sqrt(args: Value[]): Value | InterpretError {
  //Validate
  const error = checkArgs(args, 1, ['Number']);
  if (error) {
    return error;
  }

  const val = args[0];
  if(val instanceof Int || val instanceof Dbl){
    const result = Math.sqrt(val.val);
    return new Dbl(result); //sqrt log10 and loge should always return a double
  }
  return new InterpretError('Run-time Error: Argument is not a number');
}

function log10(args: Value[]): Value | InterpretError {
  //Validate
  const error = checkArgs(args, 1, ['Number']);
  if (error) {
    return error;
  }

  const val = args[0];
  if (val instanceof Int || val instanceof Dbl) {
    const result = Math.log10(val.val);
    return new Dbl(result);
  }
  return new InterpretError('Run-time Error: Argument is not a number');
}

function loge(args: Value[]): Value | InterpretError {
  //Validate
  const error = checkArgs(args, 1, ['Number']);
  if (error) {
    return error;
  }
  
  const val = args[0];
  if(val instanceof Int || val instanceof Dbl){
    const result = Math.log(val.val);
    return new Dbl(result);
  }
  return new InterpretError('Run-time Error: Argument is not a number');
}


function doubleToInteger(args: Value[]): Value | InterpretError {
  //validate exactly one double
  const error = checkArgs(args, 1, [Dbl]);
  if(error) return error;
  const val = args[0];
  if(val instanceof Dbl){
    const result = Math.trunc(val.val);
    return new Int(result);
  }
  return new InterpretError('Run-time Error: Argument is not a double');
}

function integerToDouble(args: Value[]): Value | InterpretError {
  //validate exactly one integer
  const error = checkArgs(args, 1, [Int]);
  if(error) return error;
  const val = args[0];
  if(val instanceof Int){
    const result = val.val;
    return new Dbl(result);
  }
  return new InterpretError('Run-time Error: Argument is not an integer');
}

function charToString(args: Value[]): Value | InterpretError {
  //validate exactly one character
  const error = checkArgs(args, 1, [Char]);
  if(error) return error;
  const val = args[0];
  if(val instanceof Char){
    return new Str(val.val);
  }
  return new InterpretError('Run-time Error: Argument is not a character');
}

function charToInteger(args: Value[]): Value | InterpretError {
  //validate exactly one character
  const error = checkArgs(args, 1, [Char]);
  if(error) return error;
  const val = args[0];
  if(val instanceof Char){
    return new Int(val.val.charCodeAt(0)); //wrap thecharcode in an int
  }
  return new InterpretError('Run-time Error: Argument is not a character');
}

function sin(args: Value[]): Value | InterpretError {
  //Validate
  const error = checkArgs(args, 1, ['Number']);
  if (error) {
    return error;
  }

  const val = args[0];
  if(val instanceof Int || val instanceof Dbl){
    const result = Math.sin(val.val);
    return new Dbl(result);
  }
  return new InterpretError('Run-time Error: Argument is not a number');
}

function cos(args: Value[]): Value | InterpretError {
  //Validate
  const error = checkArgs(args, 1, ['Number']);
  if (error) {
    return error;
  }

  const val = args[0];
  if(val instanceof Int || val instanceof Dbl){
    const result = Math.cos(val.val);
    return new Dbl(result);
  }
  return new InterpretError('Run-time Error: Argument is not a number');
}
//string functinos
function stringLength(args: Value[]): Value | InterpretError {
  const error = checkArgs(args, 1, [Str]);
  if(error) return error;
  const val = args[0] as Str;
  return new Int(val.val.length);
}

function stringAppend(args: Value[]): Value | InterpretError {
  //use -1 for zero or more
  let result = "";
  for ( let a of args) {
    if(a instanceof Str) {
      result += a.val;
    } else {
      return new InterpretError('Run-time Error: Argument is not a string');
    }
  }
  return new Str(result);
}

function substring(args: Value[]): Value | InterpretError {
  const error = checkArgs(args, 3, [Str, Int, Int]);
  if(error) return error;
  const s = args[0] as Str;
  const start = args[1] as Int;
  const end = args[2] as Int;
  //strict range check
  if (start.val < 0 || start.val > end.val || end.val > s.val.length) {
    return new InterpretError('Run-time Error: Index out of bounds');
  }
  return new Str(s.val.substring(start.val, end.val));
}

function stringCharAt(args: Value[]): Value | InterpretError {
  const error = checkArgs(args, 2, [Str, Int]);
  if(error) return error;
  const s = (args[0] as Str).val;
  const index = (args[1] as Int).val;
  //strict range check
  if (index < 0 || index >= s.length) {
    return new InterpretError('Run-time Error: Index out of bounds');
  }
  return new Char(s.charAt(index));
}