import { Env } from './Env';
import { BoolFalse, BoolTrue, BuiltInFunc, Dbl, Int, InterpretError, Value } from './Values';

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
