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

  addBuiltin(env, '+', -1, ['Number'], add);
  addBuiltin(env, '-', -1, ['Number'], subtract);
  addBuiltin(env, '*', -1, ['Number'], multiply);
  addBuiltin(env, '/', -1, ['Number'], divide);
  addBuiltin(env, '%', -1, ['Number'], modulo);
  addBuiltin(env, '=', -1, ['Number'], numEq);
  addBuiltin(env, '<', -1, ['Number'], lessThan);
  addBuiltin(env, '<=', -1, ['Number'], lessThanOrEqual);
  addBuiltin(env, '>', -1, ['Number'], greaterThan);
  addBuiltin(env, '>=', -1, ['Number'], greaterThanOrEqual);
  addBuiltin(env, 'display', 1, [], display);
  addBuiltin(env, 'integer?', 1, [], isInteger);
  addBuiltin(env, 'double?', 1, [], isDouble);
  addBuiltin(env, 'number?', 1, [], isNumber);
  addBuiltin(env, 'string?', 1, [], isString);
  addBuiltin(env, 'char?', 1, [], isCharacter);
  addBuiltin(env, 'symbol?', 1, [], isSymbol);
  addBuiltin(env, 'vector?', 1, [], isVector);
  addBuiltin(env, 'procedure?', 1, [], isProcedure);
  addBuiltin(env, 'list?', 1, [], isList);
  addBuiltin(env, 'null?', 1, [], isNull);
  addBuiltin(env, 'not', 1, [], not);
  addBuiltin(env, 'abs', 1, ['Number'], abs);
  addBuiltin(env, 'sqrt', 1, ['Number'], sqrt);
  addBuiltin(env, 'pow', 2, ['Number', 'Number'], pow);
  addBuiltin(env, 'log10', 1, ['Number'], log10);
  addBuiltin(env, 'loge', 1, ['Number'], loge);
  addBuiltin(env, 'sin', 1, ['Number'], sin);
  addBuiltin(env, 'cos', 1, ['Number'], cos);
  addBuiltin(env, 'tan', 1, ['Number'], tan);
  addBuiltin(env, 'asin', 1, ['Number'], asin);
  addBuiltin(env, 'acos', 1, ['Number'], acos);
  addBuiltin(env, 'atan', 1, ['Number'], atan);
  addBuiltin(env, 'sinh', 1, ['Number'], sinh);
  addBuiltin(env, 'cosh', 1, ['Number'], cosh);
  addBuiltin(env, 'tanh', 1, ['Number'], tanh);
  addBuiltin(env, 'nan?', 1, ['Number'], isNaNValue);
  env.put('pi', new Dbl(Math.PI));
  env.put('e', new Dbl(Math.E));
  env.put('tau', new Dbl(2 * Math.PI));
  env.put('inf+', new Dbl(Number.POSITIVE_INFINITY));
  env.put('inf-', new Dbl(Number.NEGATIVE_INFINITY));
  env.put('nan', new Dbl(Number.NaN));
  addBuiltin(env, 'integer->double', 1, [Int], integerToDouble);
  addBuiltin(env, 'double->integer', 1, [Dbl], doubleToInteger);
  addBuiltin(env, 'integer->string', 1, [Int], integerToString);
  addBuiltin(env, 'double->string', 1, [Dbl], doubleToString);
  addBuiltin(env, 'string->integer', 1, [Str], stringToInteger);
  addBuiltin(env, 'string->double', 1, [Str], stringToDouble);
  addBuiltin(env, 'integer->char', 1, [Int], integerToChar);
  addBuiltin(env, 'char->integer', 1, [Char], charToInteger);
  addBuiltin(env, 'char->string', 1, [Char], charToString);
  addBuiltin(env, 'char-equal?', 2, [Char, Char], charEqual);
  addBuiltin(env, 'string-length', 1, [Str], stringLength);
  addBuiltin(env, 'string-append', 0, [Str], stringAppend);
  addBuiltin(env, 'substring', 3, [Str, Int, Int], substring);
  addBuiltin(env, 'string-char-at', 2, [Str, Int], stringCharAt);
  addBuiltin(env, 'cons', 2, [], cons);
  addBuiltin(env, 'car', 1, [Cons], car);
  addBuiltin(env, 'cdr', 1, [Cons], cdr);
  addBuiltin(env, 'list', 0, [], list);
  addBuiltin(env, 'set-car!', 2, [Cons, 'Any'], setCar);
  addBuiltin(env, 'set-cdr!', 2, [Cons, 'Any'], setCdr);
  addBuiltin(env, 'vector', 0, [], vector);
  addBuiltin(env, 'vector-init', 2, [Int, 'Any'], vectorInit);
  addBuiltin(env, 'vector-length', 1, [Vec], vectorLength);
  addBuiltin(env, 'vector-get', 2, [Vec, Int], vectorGet);
  addBuiltin(env, 'vector-set!', 3, [Vec, Int, 'Any'], vectorSet);
  addBuiltin(env, 'eqv?', 2, [], eqv);
  addBuiltin(env, 'equal?', 2, [], equal);
}
//add builtin functinos as type
type BuiltinImplementation = (args: Value[]) => Value | InterpretError;
//use this function to prevent having to return an error message after every single function
function addBuiltin(
  env: Env,
  name: string,
  count: number,
  types: ArgumentType[],
  implementation: BuiltinImplementation,
): void {
  env.put(name, new BuiltInFunc(name, args => {
    const error = checkArgs(args, count, types);
    return error ?? implementation(args);
  }));
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
  const anyDbl = args.some(value => value instanceof Dbl);
  const sum = args.reduce<number>((total, value) => total + (value as Int | Dbl).val, 0);
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
  const nums = args.map(value => (value as Int | Dbl).val);
  for (let i = 1; i < nums.length; ++i) {
    if (nums[i - 1] !== nums[i]) {
      return new BoolFalse();
    }
  }
  return new BoolTrue();
}
//display function to print values
function display(args: Value[]): Value | InterpretError {
  const val = args[0];

  process.stdout.write(printValue(val));
  return val;
}

//list of classes
type ArgumentType =
  | 'Any'
  | 'Number'
  | typeof Int
  | typeof Dbl
  | typeof Str
  | typeof Char
  | typeof Cons
  | typeof Vec;
//Dont have to type run-time error in every function
const TYPE_ERRORS: Record<string, string> = {
  Int: 'Argument is not an integer',
  Dbl: 'Argument is not a double',
  Str: 'Argument is not a string',
  Char: 'Argument is not a character',
  Cons: 'Argument is not a cons cell',
  Vec: 'Argument is not a vector',
};

/**
 * Check argument count and any declared types from left to right.
 *
 * @param args The values passed to the function.
 * @param count The expected number of arguments.
 *              -1 means "one or more".
 *               0 means "zero or more".
 * @param types A list of classes. If the list is shorter than the number of
 *              arguments, the last type in the list is applied to all
 *              remaining arguments.
 */
function checkArgs(args: Value[], count: number, types: ArgumentType[]): InterpretError | null {
  //  Check number of arguments
  if (count === -1) {
    if (args.length < 1) {
      return new InterpretError('Run-time Error: Wrong number of arguments');
    }
  } else if (count !== 0 && args.length !== count) {
    // If count is 0, we allow any number of arguments (zero or more)
    return new InterpretError('Run-time Error: Wrong number of arguments');
  }

  //  Check types from left to right
  for (let i = 0; i < args.length; i++) {
    // Use the type at index i.
    // If we've run out of specific types, reuse the last one provided.
    const expected = types[i] ?? types[types.length - 1];
    const val = args[i];

    if (expected === 'Number') {
      if (!(val instanceof Int || val instanceof Dbl)) {
        return new InterpretError('Run-time Error: Argument is not a number');
      }
    } else if (expected === 'Any') {
      continue;
    } else if (expected) {
      if (!(val instanceof expected)) {
        const message = TYPE_ERRORS[expected.name] || 'Invalid argument';
        return new InterpretError(`Run-time Error: ${message}`);
      }
    }
  }

  return null; // No error
}

//this function checks if a value satisfies a given predicate  
function typePredicate(args: Value[], predicate: (value: Value) => boolean): Value | InterpretError {
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

//simple math functions

//abs
function abs(args: Value[]): Value | InterpretError {
  const val = args[0] as Int | Dbl;
  const result = Math.abs(val.val);
  return val instanceof Int ? new Int(result) : new Dbl(result);
}

function sqrt(args: Value[]): Value | InterpretError {
  return new Dbl(Math.sqrt((args[0] as Int | Dbl).val));
}

function log10(args: Value[]): Value | InterpretError {
  return new Dbl(Math.log10((args[0] as Int | Dbl).val));
}

function loge(args: Value[]): Value | InterpretError {
  return new Dbl(Math.log((args[0] as Int | Dbl).val));
}

//transistioning functions
function doubleToInteger(args: Value[]): Value | InterpretError {
  return new Int(Math.trunc((args[0] as Dbl).val));
}

function integerToDouble(args: Value[]): Value | InterpretError {
  return new Dbl((args[0] as Int).val);
}

function charToString(args: Value[]): Value | InterpretError {
  return new Str((args[0] as Char).val);
}

function charToInteger(args: Value[]): Value | InterpretError {
  return new Int((args[0] as Char).val.charCodeAt(0));
}
//sin and cos
function sin(args: Value[]): Value | InterpretError {
  return new Dbl(Math.sin((args[0] as Int | Dbl).val));
}

function cos(args: Value[]): Value | InterpretError {
  return new Dbl(Math.cos((args[0] as Int | Dbl).val));
}
//string functinos
function stringLength(args: Value[]): Value | InterpretError {
  return new Int((args[0] as Str).val.length);
}

function stringAppend(args: Value[]): Value | InterpretError {
  return new Str(args.map(value => (value as Str).val).join(''));
}

function substring(args: Value[]): Value | InterpretError {
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
  const s = (args[0] as Str).val;
  const index = (args[1] as Int).val;
  //strict range check
  if (index < 0 || index >= s.length) {
    return new InterpretError('Run-time Error: Index out of bounds');
  }
  return new Char(s.charAt(index));
}

//scheme getters and setters, mutators
function cons(args: Value[]): Value | InterpretError {
  return new Cons(args[0], args[1]);
}

function car(args: Value[]): Value | InterpretError {
  return (args[0] as Cons).car;
}

function cdr(args: Value[]): Value | InterpretError {
  return (args[0] as Cons).cdr;
}

function list(args: Value[]): Value {
  let result: Value = new EmptyCons();
  for (let index = args.length - 1; index >= 0; index--) {
    result = new Cons(args[index], result);
  }
  return result;
}

function setCar(args: Value[]): Value | InterpretError {
  const cell = args[0] as Cons;
  cell.car = args[1];
  return args[1];
}

function setCdr(args: Value[]): Value | InterpretError {
  const cell = args[0] as Cons;
  cell.cdr = args[1];
  return args[1];
}

//math functions
//helper function that converts value objects and turns them into simple array of numbers
function numericValues(args: Value[]): number[] {
  return args.map(value => (value as Int | Dbl).val);
}
//helper function to decide whether final answer to problem should be int or dbl
//if all original arguments were Ints wrap value in an int, else wrap in a new dbl
//avoids a lot of redundant code
function numericResult(value: number, args: Value[]): Value {
  return args.every(argument => argument instanceof Int) ? new Int(value) : new Dbl(value);
}

function subtract(args: Value[]): Value | InterpretError {
  const values = numericValues(args);
  if (values instanceof InterpretError) return values; //ensure that we
  const result = values.length === 1
    ? -values[0]
    : values.slice(1).reduce((total, value) => total - value, values[0]);
  return numericResult(result, args);
}

function multiply(args: Value[]): Value | InterpretError {
  const values = numericValues(args);
  if (values instanceof InterpretError) return values;
  return numericResult(values.reduce((total, value) => total * value, 1), args);
}

function divide(args: Value[]): Value | InterpretError {
  const values = numericValues(args);
  if (values instanceof InterpretError) return values; 
  let result = values[0];
  let integerSoFar = args[0] instanceof Int;
  for (let index = 1; index < values.length; index++) {
    const divisor = values[index];
    if (integerSoFar && args[index] instanceof Int) {
      if (divisor === 0) return new InterpretError('Run-time Error: Division by zero');
      result = Math.trunc(result / divisor);
    } else {
      integerSoFar = false;
      result /= divisor;
    }
  }
  return integerSoFar ? new Int(result) : new Dbl(result);
}

function modulo(args: Value[]): Value | InterpretError {
  const values = numericValues(args);
  if (values instanceof InterpretError) return values;
  let result = values[0];
  let integerSoFar = args[0] instanceof Int;
  for (let index = 1; index < values.length; index++) {
    const divisor = values[index];
    if (integerSoFar && args[index] instanceof Int) {
      if (divisor === 0) return new InterpretError('Run-time Error: Division by zero');
      result %= divisor;
    } else {
      integerSoFar = false;
      result %= divisor;
    }
  }
  return integerSoFar ? new Int(result) : new Dbl(result);
}
//use compareNumbers to avoid writing similar code for lessThan, lessThanOrEqual, etc.
function compareNumbers(
  args: Value[],
  comparison: (left: number, right: number) => boolean,
): Value | InterpretError {
  const values = numericValues(args);
  if (values instanceof InterpretError) return values;
  for (let index = 1; index < values.length; index++) {
    if (!comparison(values[index - 1], values[index])) return new BoolFalse();
  }
  return new BoolTrue();
}

function lessThan(args: Value[]): Value | InterpretError {
  return compareNumbers(args, (left, right) => left < right);
}

function lessThanOrEqual(args: Value[]): Value | InterpretError {
  return compareNumbers(args, (left, right) => left <= right);
}

function greaterThan(args: Value[]): Value | InterpretError {
  return compareNumbers(args, (left, right) => left > right);
}

function greaterThanOrEqual(args: Value[]): Value | InterpretError {
  return compareNumbers(args, (left, right) => left >= right);
}

//use to avoid boilerplate code for tan asin, acos, etc
function unaryMath(args: Value[], operation: (value: number) => number): Value | InterpretError {
  return new Dbl(operation((args[0] as Int | Dbl).val));
}

function pow(args: Value[]): Value | InterpretError {
  return new Dbl(Math.pow((args[0] as Int | Dbl).val, (args[1] as Int | Dbl).val));
}

function tan(args: Value[]): Value | InterpretError {
  return unaryMath(args, Math.tan);
}

function asin(args: Value[]): Value | InterpretError {
  return unaryMath(args, Math.asin);
}

function acos(args: Value[]): Value | InterpretError {
  return unaryMath(args, Math.acos);
}

function atan(args: Value[]): Value | InterpretError {
  return unaryMath(args, Math.atan);
}

function sinh(args: Value[]): Value | InterpretError {
  return unaryMath(args, Math.sinh);
}

function cosh(args: Value[]): Value | InterpretError {
  return unaryMath(args, Math.cosh);
}

function tanh(args: Value[]): Value | InterpretError {
  return unaryMath(args, Math.tanh);
}

function isNaNValue(args: Value[]): Value | InterpretError {
  return Number.isNaN((args[0] as Int | Dbl).val) ? new BoolTrue() : new BoolFalse();
}

// more transition functions that I forgot to do
function integerToString(args: Value[]): Value | InterpretError {
  return new Str((args[0] as Int).val.toString());
}

function doubleToString(args: Value[]): Value | InterpretError {
  return new Str((args[0] as Dbl).val.toString());
}

function stringToInteger(args: Value[]): Value | InterpretError {
  const text = (args[0] as Str).val;
  if (!/^-?\d+$/.test(text)) {
    return new InterpretError('Run-time Error: String cannot be converted');
  }
  return new Int(Number.parseInt(text, 10));
}

function stringToDouble(args: Value[]): Value | InterpretError {
  const text = (args[0] as Str).val;
  if (!/^-?\d+(?:\.\d+)?$/.test(text)) {
    return new InterpretError('Run-time Error: String cannot be converted');
  }
  return new Dbl(Number(text));
}

function integerToChar(args: Value[]): Value | InterpretError {
  const code = (args[0] as Int).val;
  if (code < 0 || code > 255) {
    return new InterpretError('Run-time Error: Value out of range');
  }
  return new Char(String.fromCharCode(code));
}

function charEqual(args: Value[]): Value | InterpretError {
  return (args[0] as Char).val === (args[1] as Char).val ? new BoolTrue() : new BoolFalse();
}


//vector functions
function vector(args: Value[]): Value {
  return new Vec(args.slice());
}


function vectorInit(args: Value[]): Value | InterpretError {
  const length = (args[0] as Int).val;
  //have to include error messages for range and value specific errors.
  if (length < 0) return new InterpretError('Run-time Error: Value cannot be negative');
  return new Vec(Array.from({ length }, () => args[1]));
}

function vectorLength(args: Value[]): Value | InterpretError {
  return new Int((args[0] as Vec).items.length);
}

function vectorGet(args: Value[]): Value | InterpretError {
  const items = (args[0] as Vec).items;
  const index = (args[1] as Int).val;
  if (index < 0 || index >= items.length) {
    return new InterpretError('Run-time Error: Index out of bounds');
  }
  return items[index];
}

function vectorSet(args: Value[]): Value | InterpretError {
  const items = (args[0] as Vec).items;
  const index = (args[1] as Int).val;
  if (index < 0 || index >= items.length) {
    return new InterpretError('Run-time Error: Index out of bounds');
  }
  items[index] = args[2];
  return args[2];
}

function eqvValues(left: Value, right: Value): boolean {
  if (left instanceof Int && right instanceof Int) return left.val === right.val;
  if (left instanceof Dbl && right instanceof Dbl) return left.val === right.val;
  if (left instanceof Char && right instanceof Char) return left.val === right.val;
  if (left instanceof Symbol && right instanceof Symbol) return left.name === right.name;
  if (left instanceof BoolTrue) return right instanceof BoolTrue;
  if (left instanceof BoolFalse) return right instanceof BoolFalse;
  if (left instanceof EmptyCons) return right instanceof EmptyCons;
  return left === right;
}

function equalValues(left: Value, right: Value): boolean {
  if (left instanceof Str && right instanceof Str) return left.val === right.val;
  if (left instanceof Cons && right instanceof Cons) {
    return equalValues(left.car, right.car) && equalValues(left.cdr, right.cdr);
  }
  if (left instanceof Vec && right instanceof Vec) {
    return left.items.length === right.items.length
      && left.items.every((item, index) => equalValues(item, right.items[index]));
  }
  return eqvValues(left, right);
}

function eqv(args: Value[]): Value | InterpretError {
  return eqvValues(args[0], args[1]) ? new BoolTrue() : new BoolFalse();
}

function equal(args: Value[]): Value | InterpretError {
  return equalValues(args[0], args[1]) ? new BoolTrue() : new BoolFalse();
}