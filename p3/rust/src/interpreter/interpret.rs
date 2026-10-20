use super::env::{make_inner_env, Env};
use super::values::{Cons, LambdaVal, Value, Vector};
use crate::parser::ast::{AstNode, AstNodeDetails, Datum, LambdaDefNode};
use std::{cell::RefCell, rc::Rc};






fn id_name(node: &str) -> String {
    node.to_string()
}
/// Interpret an AstNode in the context of an Environment
///
/// @param expr The AST node to interpret
/// @param env  The environment to interpret it in
///
/// @return The result of the interpretation, or an error message
pub fn interpret_ast(expr: &AstNode, env: Rc<RefCell<Env>>) -> Result<Value, String> {
    loop{
        let node_type = match &expr.details{
    //constants
            AstNodeDetails::BoolTrue => return Ok(Value::BoolTrue),
            AstNodeDetails::BoolFalse => return Ok(Value::BoolFalse),
            AstNodeDetails::Int(i) => return Ok(Value::Int(*i)),
            AstNodeDetails::Dbl(f) => return Ok(Value::Dbl(*f)),
            AstNodeDetails::Str(s) => return Ok(Value::Str(s.clone())),
            AstNodeDetails::Char(c) => return Ok(Value::Char(*c)),
            AstNodeDetails::Symbol(s) => return Ok(Value::Symbol(s.clone())),
            AstNodeDetails::EmptyCons => return Ok(Value::EmptyCons),


    //data structures
            AstNodeDetails::Cons { car, cdr } => {
                let car_val = interpret_ast((*car).clone(), env.clone())?;
                let cdr_val = interpret_ast((*cdr).clone(), env.clone())?;
                return Ok(Value::Cons(Rc::new(RefCell::new(Cons { 
                    car: car_val, 
                    cdr: cdr_val 
                }))))
            AstNodeDetails::Vec { items } => {
                let mut vec_items = Vec::new();
                for item in items {
                    vec_items.push(interpret_ast(item.clone(), env.clone())?);
                }
                return Ok(Value::Vec(Rc::new(RefCell::new(Vector {
                    items: vec_items,
                }))));
            }
            // Identifier lookup
            AstNodeDetails::Identifier(id) => {
                return env.borrow_mut().get(id);
            }

            // QUOTE/TICK: evaluate datum as tail
            AstNodeDetails::Quote { datum } => {
                expr = datum_to_ast_node(datum)?;
                continue;
            }

            AstNodeDetails::Tick { datum } => {
                expr = datum_to_ast_node(datum)?;
                continue;
            }

            // DEFINE VAR: (define id expr)
            AstNodeDetails::DefineVar { id, expr: value_expr } => {
                let val = interpret_ast(value_expr.as_ref().clone(), env.clone())?;
                env.borrow_mut().put(id.clone(), val.clone());
                return Ok(val);
            }

            // DEFINE FUNC: (define (id ...) expr+)
            AstNodeDetails::DefineFunc { ids, body } => {
                let lambda_val = Value::LambdaVal(Rc::new(LambdaVal {
                    env: env.clone(),
                    func: expr.clone(),
                }));
                env.borrow_mut().put(ids[0].clone(), lambda_val.clone());
                return Ok(lambda_val);
            }

            // LAMBDA: (lambda (id*) expr+)
            AstNodeDetails::LambdaDef { .. } => {
                return Ok(Value::LambdaVal(Rc::new(LambdaVal {
                    env: env.clone(),
                    func: expr.clone(),
                })));
            }

            // IF: (if test consequent alternate) — branch is tail
            AstNodeDetails::If { cond, if_true, if_false } => {
                let test_val = interpret_ast(cond.as_ref().clone(), env.clone())?;
                expr = if is_truthy(&test_val) {
                    if_true.as_ref().clone()
                } else {
                    if_false.as_ref().clone()
                };
                continue;
            }

            // AND: (and expr+) — last expr is tail
            AstNodeDetails::And { exprs } => {
                for e in &exprs[..exprs.len().saturating_sub(1)] {
                    let result = interpret_ast(e.clone(), env.clone())?;
                    if !is_truthy(&result) {
                        return Ok(result);
                    }
                }
                if exprs.is_empty() {
                    return Ok(Value::BoolTrue);
                }
                expr = exprs[exprs.len() - 1].clone();
                continue;
            }

            // OR: (or expr+) — last expr is tail
            AstNodeDetails::Or { exprs } => {
                for e in &exprs[..exprs.len().saturating_sub(1)] {
                    let result = interpret_ast(e.clone(), env.clone())?;
                    if is_truthy(&result) {
                        return Ok(result);
                    }
                }
                if exprs.is_empty() {
                    return Ok(Value::BoolFalse);
                }
                expr = exprs[exprs.len() - 1].clone();
                continue;
            }

            // BEGIN: (begin expr+) — last expr is tail
            AstNodeDetails::Begin { exprs } => {
                eval_prefix(&exprs[..exprs.len().saturating_sub(1)], env.clone())?;
                if exprs.is_empty() {
                    return Ok(Value::EmptyCons);
                }
                expr = exprs[exprs.len() - 1].clone();
                continue;
            }

            // COND: (cond (test expr+)+)
            AstNodeDetails::Cond { conditions } => {
                let mut matched = None;
                for condition in conditions {
                    let test_val = interpret_ast(condition.test.as_ref().clone(), env.clone())?;
                    if is_truthy(&test_val) {
                        matched = Some(&condition.exprs);
                        break;
                    }
                }
                let body = matched.ok_or("Run-time Error: No true condition in cond")?;
                eval_prefix(&body[..body.len().saturating_sub(1)], env.clone())?;
                if body.is_empty() {
                    return Ok(Value::EmptyCons);
                }
                expr = body[body.len() - 1].clone();
                continue;
            }

             // LET: (let ((id expr)+) expr+) — body last expr is tail
            AstNodeDetails::Let { vars, body } => {
                let let_env = make_inner_env(env.clone());
                for binding in vars {
                    let val = interpret_ast(binding.val.as_ref().clone(), env.clone())?;
                    let_env.borrow_mut().put(binding.id.clone(), val);
                }
                eval_prefix(&body[..body.len().saturating_sub(1)], let_env.clone())?;
                if body.is_empty() {
                    return Ok(Value::EmptyCons);
                }
                expr = body[body.len() - 1].clone();
                env = let_env;
                continue;
            }
             // SET!: (set! id expr)
            AstNodeDetails::Set { id, expr: value_expr } => {
                let val = interpret_ast(value_expr.as_ref().clone(), env.clone())?;
                env.borrow_mut().update(id.clone(), val.clone())?;
                return Ok(val);
            }

            // APPLY: (apply func args)
            AstNodeDetails::Apply { func: func_expr, args: args_expr } => {
                let func_val = interpret_ast(func_expr.as_ref().clone(), env.clone())?;
                let args_val = interpret_ast(args_expr.as_ref().clone(), env.clone())?;
                let args = cons_to_vec(&args_val)?;
                return call_function(&func_val, args, env);
            }

            // CALL: (func arg+)
            AstNodeDetails::Call { exprs } => {
                let func_val = interpret_ast(exprs[0].clone(), env.clone())?;
                let mut args = Vec::new();
                for arg_expr in &exprs[1..] {
                    args.push(interpret_ast(arg_expr.clone(), env.clone())?);
                }
                return call_function(&func_val, args, env);
            }

            _ => {
                return Err(format!("Unknown AST node type"));
            }
        };

    }
}
}

fn eval_prefix(exprs: &[AstNode], env: Rc<RefCell<Env>>) -> Result<(), String> {
    for expr in exprs{
        interpret_ast(expr.clone(), env.clone())?;
    }
    Ok(())
}


fn call_function(func_val: &Value, args: Vec<Value>, env: Rc<RefCell<Env>>) -> Result<Value, String> {
    match func_val {
        Value::BuiltInFunc(builtin) => builtin.func(&args),
        
    }

}



/// Pretty print a Value
///
/// @param val The Value to print
///
/// @return A string representation of the value
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
        Value::Cons(c) => {
             if let Ok(items) = cons_to_vec(val) {
               let item_strs: Vec<String> = items.iter().map(value_to_string).collect();
                return format!("({})", item_strs.join(" "));
             }
            format!(
                "({} . {})",
                value_to_string(&c.borrow().car),
                value_to_string(&c.borrow().cdr)
            )
        }
    }
}
