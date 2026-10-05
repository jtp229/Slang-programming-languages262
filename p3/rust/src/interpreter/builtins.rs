use super::env::{add_to_env, Env};
use super::values::Value;
use std::rc::Rc;

/// Add the built-in functions to the given environment.
///
/// NB: The standard library is a later assignment.  For now, the only
///     built-ins are `+` and `=`, which are enough to test function calls,
///     conditionals, and recursion.
///
/// @param env The environment to add functions to
pub fn add_builtins(env: &mut Env) {
    add_to_env(env, "+", Rc::new(add));
    add_to_env(env, "=", Rc::new(num_eq));
}

/// Add one or more numbers.  The result is an Int if all of the arguments are
/// Ints, and a Dbl if any of them is a Dbl.
///
/// @param args The numbers to add
///
/// @return The sum, or an error if there are no arguments or any argument is
///         not a number
fn add(args: &mut [Value]) -> Result<Value, String> {
    if args.is_empty() {
        return Err("Run-time Error: Invalid number of arguments".to_string());
    }
    let mut any_dbl = false;
    for a in args.iter() {
        match a {
            Value::Int(_) => {}
            Value::Dbl(_) => any_dbl = true,
            _ => return Err("Run-time Error: Argument is not a number".to_string()),
        }
    }
    if any_dbl {
        let to_dbl = |v: &Value| match v {
            Value::Int(i) => *i as f64,
            Value::Dbl(d) => *d,
            _ => unreachable!(),
        };
        let sum = args[1..]
            .iter()
            .fold(to_dbl(&args[0]), |s, a| s + to_dbl(a));
        Ok(Value::Dbl(sum))
    } else {
        let to_int = |v: &Value| match v {
            Value::Int(i) => *i,
            _ => unreachable!(),
        };
        let sum = args[1..]
            .iter()
            .fold(to_int(&args[0]), |s, a| s + to_int(a));
        Ok(Value::Int(sum))
    }
}

/// Check whether one or more numbers are all equal.  If any argument is a Dbl,
/// the comparison is done on Dbls.
///
/// @param args The numbers to compare
///
/// @return #t if the numbers are all equal, #f otherwise, or an error if there
///         are no arguments or any argument is not a number
fn num_eq(args: &mut [Value]) -> Result<Value, String> {
    if args.is_empty() {
        return Err("Run-time Error: Invalid number of arguments".to_string());
    }
    let mut any_dbl = false;
    for a in args.iter() {
        match a {
            Value::Int(_) => {}
            Value::Dbl(_) => any_dbl = true,
            _ => return Err("Run-time Error: Argument is not a number".to_string()),
        }
    }
    let all_equal = if any_dbl {
        let to_dbl = |v: &Value| match v {
            Value::Int(i) => *i as f64,
            Value::Dbl(d) => *d,
            _ => unreachable!(),
        };
        args.windows(2).all(|p| to_dbl(&p[0]) == to_dbl(&p[1]))
    } else {
        let to_int = |v: &Value| match v {
            Value::Int(i) => *i,
            _ => unreachable!(),
        };
        args.windows(2).all(|p| to_int(&p[0]) == to_int(&p[1]))
    };
    Ok(if all_equal {
        Value::BoolTrue
    } else {
        Value::BoolFalse
    })
}
