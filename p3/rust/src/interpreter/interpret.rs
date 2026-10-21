use super::env::{make_inner_env, Env};
use super::values::{Cons, LambdaVal, Value, Vector};
use crate::parser::ast::{AstNode, AstNodeDetails, Datum, LambdaDefNode};
use std::{cell::RefCell, rc::Rc};


// Result of evaluating a function call
enum EvaluationStep {
    Value(Value),
    TailCall {
        expr: AstNode,
        env: Rc<RefCell<Env>>,
    },
}

// Interpret an AST node in the context of an environment
pub fn interpret_ast(expr: &AstNode, env: Rc<RefCell<Env>>) -> Result<Value, String> {
    let mut expr = expr.clone();
    let mut env = env;

    loop {
        match expr.details {
            // Handle constants and data structures
            AstNodeDetails::Datum(datum) => return Ok(datum_to_value(&datum)),
            // Handle identifier lookup
            AstNodeDetails::Identifier(identifier) => {
                return env
                    .borrow()
                    .get(&identifier.id)
                    .ok_or_else(|| "Run-time Error: Invalid identifier".to_string());
            }
            // QUOTE / TICK: evaluate the datum
            AstNodeDetails::Quote(quote) => return Ok(datum_to_value(&quote.datum)),
            AstNodeDetails::Tick(tick) => return Ok(datum_to_value(&tick.datum)),
            // DEFINE: as a variable (define id expr)
            AstNodeDetails::DefineVar(definition) => {
                let value = interpret_ast(&definition.expr, env.clone())?;
                env.borrow_mut().put(&definition.id.id, value.clone());
                return Ok(value);
            }
            // DEFINE: as a function (define (id ...) expr+)
            AstNodeDetails::DefineFunc(definition) => {
                let Some(name) = definition
                    .ids
                    .first()
                    .map(|identifier| identifier.id.clone())
                else {
                    return Err("Run-time Error: Invalid identifier".to_string());
                };
                let function = LambdaDefNode {
                    formals: definition.ids.into_iter().skip(1).collect(),
                    exprs: definition.body,
                };
                let value = Value::LambdaVal(LambdaVal {
                    env: env.clone(),
                    func: function,
                });
                env.borrow_mut().put(&name, value.clone());
                return Ok(value);
            }
            // LAMBDA: (lambda (id*) expr+)
            AstNodeDetails::LambdaDef(function) => {
                return Ok(Value::LambdaVal(LambdaVal {
                    env: env.clone(),
                    func: function,
                }));
            }
            // IF: (if test consequent alternate)
            AstNodeDetails::If(if_node) => {
                let condition = interpret_ast(&if_node.cond, env.clone())?;
                expr = if is_truthy(&condition) {
                    *if_node.if_true
                } else {
                    *if_node.if_false
                };
            }
            // AND: short-circuit; the last expression is in tail position
            AstNodeDetails::And(and_node) => {
                let Some((last, preceding)) = and_node.exprs.split_last() else {
                    return Ok(Value::BoolTrue);
                };
                for item in preceding {
                    let value = interpret_ast(item, env.clone())?;
                    if !is_truthy(&value) {
                        return Ok(value);
                    }
                }
                expr = last.clone();
            }
            // OR: short-circuit, the last expression is in tail position
            AstNodeDetails::Or(or_node) => {
                let Some((last, preceding)) = or_node.exprs.split_last() else {
                    return Ok(Value::BoolFalse);
                };
                for item in preceding {
                    let value = interpret_ast(item, env.clone())?;
                    if is_truthy(&value) {
                        return Ok(value);
                    }
                }
                expr = last.clone();
            }
            // BEGIN: evaluate each expression in order
            AstNodeDetails::Begin(begin_node) => {
                let Some((last, preceding)) = begin_node.exprs.split_last() else {
                    return Ok(Value::EmptyCons);
                };
                eval_prefix(preceding, env.clone())?;
                expr = last.clone();
            }
            // COND: evaluate the body of the first matching condition
            AstNodeDetails::Cond(cond_node) => {
                let mut selected_body = None;
                for condition in cond_node.conditions {
                    let value = interpret_ast(&condition.test, env.clone())?;
                    if is_truthy(&value) {
                        selected_body = Some(condition.exprs);
                        break;
                    }
                }
                let body = selected_body
                    .ok_or_else(|| "Run-time Error: No true condition in cond".to_string())?;
                let Some((last, preceding)) = body.split_last() else {
                    return Ok(Value::EmptyCons);
                };
                eval_prefix(preceding, env.clone())?;
                expr = last.clone();
            }
            // LET: evaluate bindings outside the new scope, then evaluate the body inside it
            AstNodeDetails::Let(let_node) => {
                let inner_env = Rc::new(RefCell::new(make_inner_env(env.clone())));
                for binding in let_node.vars {
                    let value = interpret_ast(&binding.val, env.clone())?;
                    inner_env.borrow_mut().put(&binding.id.id, value);
                }
                let Some((last, preceding)) = let_node.body.split_last() else {
                    return Ok(Value::EmptyCons);
                };
                eval_prefix(preceding, inner_env.clone())?;
                expr = last.clone();
                env = inner_env;
            }
            // SET!: update an existing binding
            AstNodeDetails::Set(set_node) => {
                let value = interpret_ast(&set_node.expr, env.clone())?;
                if env.borrow().get(&set_node.id.id).is_none() {
                    return Err("Run-time Error: Invalid identifier".to_string());
                }
                env.borrow_mut().update(&set_node.id.id, value.clone());
                return Ok(value);
            }
            // APPLY: call a function with arguments from a list
            AstNodeDetails::Apply(apply_node) => {
                let function = interpret_ast(&apply_node.func, env.clone())?;
                let list = interpret_ast(&apply_node.args, env.clone())?;
                let arguments = cons_to_vec(&list)?;
                match call_function(function, arguments)? {
                    EvaluationStep::Value(value) => return Ok(value),
                    EvaluationStep::TailCall {
                        expr: next_expr,
                        env: next_env,
                    } => {
                        expr = next_expr;
                        env = next_env;
                    }
                }
            }
            // CALL: evaluate the function and its arguments in order
            AstNodeDetails::Call(call_node) => {
                let Some((function_expr, argument_exprs)) = call_node.exprs.split_first() else {
                    return Err("Run-time Error: Invalid number of arguments".to_string());
                };
                let function = interpret_ast(function_expr, env.clone())?;
                let mut arguments = Vec::with_capacity(argument_exprs.len());
                for argument_expr in argument_exprs {
                    arguments.push(interpret_ast(argument_expr, env.clone())?);
                }
                match call_function(function, arguments)? {
                    EvaluationStep::Value(value) => return Ok(value),
                    EvaluationStep::TailCall {
                        expr: next_expr,
                        env: next_env,
                    } => {
                        expr = next_expr;
                        env = next_env;
                    }
                }
            }
            // Reject AST nodes that are only used inside another node
            AstNodeDetails::Condition(_) => {
                return Err("Run-time Error: Unknown AST node type".to_string());
            }
        }
    }
}

fn eval_prefix(exprs: &[AstNode], env: Rc<RefCell<Env>>) -> Result<(), String> {
    // Evaluate all but the last expression in a sequence
    for expr in exprs {
        interpret_ast(expr, env.clone())?;
    }
    Ok(())
}

fn call_function(func_val: Value, mut args: Vec<Value>) -> Result<EvaluationStep, String> {
    match func_val {
        Value::BuiltInFunc(builtin) => Ok(EvaluationStep::Value((builtin.func)(&mut args)?)),
        Value::LambdaVal(lambda) => {
            if args.len() != lambda.func.formals.len() {
                return Err("Run-time Error: Invalid number of arguments".to_string());
            }

            let lambda_env = Rc::new(RefCell::new(make_inner_env(lambda.env)));
            for (formal, argument) in lambda.func.formals.iter().zip(args) {
                lambda_env.borrow_mut().put(&formal.id, argument);
            }

            // Evaluate the body prefix and return the last expression for tail-call evaluation
            let Some((last, preceding)) = lambda.func.exprs.split_last() else {
                return Ok(EvaluationStep::Value(Value::EmptyCons));
            };
            eval_prefix(preceding, lambda_env.clone())?;
            Ok(EvaluationStep::TailCall {
                expr: last.clone(),
                env: lambda_env,
            })
        }
        _ => Err("Run-time Error: Cannot call non-function".to_string()),
    }
}

fn is_truthy(value: &Value) -> bool {
    !matches!(value, Value::BoolFalse)
}

fn cons_to_vec(value: &Value) -> Result<Vec<Value>, String> {
    // APPLY arguments must form a proper list ending in the empty list
    let mut result = Vec::new();
    let mut current = value.clone();

    loop {
        match current {
            Value::Cons(cons) => {
                let cons = cons.borrow();
                result.push(cons.car.clone());
                current = cons.cdr.clone();
            }
            Value::EmptyCons => return Ok(result),
            _ => return Err("Run-time Error: apply arguments must be a list".to_string()),
        }
    }
}

fn datum_to_value(datum: &Datum) -> Value {
    // Convert a datum recursively into its runtime value
    match datum {
        Datum::BoolTrue => Value::BoolTrue,
        Datum::BoolFalse => Value::BoolFalse,
        Datum::Char(character) => Value::Char(*character),
        Datum::Cons(cons) => Value::Cons(Rc::new(RefCell::new(Cons {
            car: datum_to_value(&cons.car),
            cdr: datum_to_value(&cons.cdr),
        }))),
        Datum::EmptyCons => Value::EmptyCons,
        Datum::Dbl(number) => Value::Dbl(*number),
        Datum::Int(number) => Value::Int(*number),
        Datum::Str(string) => Value::Str(string.clone()),
        Datum::Symbol(symbol) => Value::Symbol(symbol.clone()),
        Datum::Vec(vector) => Value::Vec(Rc::new(RefCell::new(Vector {
            items: vector.items.iter().map(datum_to_value).collect(),
        }))),
    }
}

// Pretty print a Value
pub fn value_to_string(val: &Value) -> String {
    match val {
        Value::BoolTrue => "#t".to_string(),
        Value::BoolFalse => "#f".to_string(),
        Value::Int(i) => format!("{}", i),
        Value::Dbl(f) => format!("{}", f),
        Value::Str(s) => s.to_string(),
        Value::Vec(v) => format!(
            "#({})",
            v.borrow()
                .items
                .iter()
                .map(value_to_string)
                .fold("".to_string(), |cur, nxt| format!("{} {}", cur.trim(), nxt))
        ),
        Value::Symbol(s) => format!("'{}", s),
        Value::Char(c) => match *c {
            ' ' => "#\\space".to_string(),
            '\n' => "#\\newline".to_string(),
            '\t' => "#\\tab".to_string(),
            _ => format!("#\\{}", c),
        },
        Value::BuiltInFunc(b) => format!("Built-in Function: {}", b.name),
        Value::LambdaVal(_) => "Lambda Function".to_string(),
        Value::EmptyCons => "()".to_string(),
        Value::Cons(cons) => format!(
            "({} . {})",
            value_to_string(&cons.borrow().car),
            value_to_string(&cons.borrow().cdr)
        ),
    }
}
