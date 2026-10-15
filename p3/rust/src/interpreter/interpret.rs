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
                
            }







        }




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
            // TODO:  Right now, we print CONS cells in the most naive way.
            //        Consider alternatives that are more pretty for lists?
            format!(
                "({} . {})",
                value_to_string(&c.borrow().car),
                value_to_string(&c.borrow().cdr)
            )
        }
    }
}
