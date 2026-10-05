use super::env::Env;
use crate::parser::ast::LambdaDefNode;
use std::{cell::RefCell, rc::Rc};

/// The type of Built-In Functions.  We do it like this to satisfy Clippy.
pub type BuiltInFunction = Rc<dyn Fn(&mut [Value]) -> Result<Value, String>>;

/// BuiltInFunc(name: String, func: (Value[])->Value)
#[derive(Clone)]
pub struct BuiltInFunc {
    pub name: &'static str,
    pub func: BuiltInFunction,
}

/// Cons(car: AstNode, cdr: AstNode)
/// NB: Custom Constructor for List
#[derive(Clone)]
pub struct Cons {
    pub car: Value,
    pub cdr: Value,
}

/// LambdaVal(env: Env, func: LambdaDef)
#[derive(Clone)]
pub struct LambdaVal {
    pub env: Rc<RefCell<Env>>,
    pub func: LambdaDefNode,
}

/// Vector(items: AstNode[])
#[derive(Clone)]
pub struct Vector {
    pub items: Vec<Value>,
}

/// Value is can be a Bool, Builtin, Char, Cons, Dbl, Int, Lambda, Str, Symbol,
/// or Vec
#[derive(Clone)]
pub enum Value {
    BoolTrue,
    BoolFalse,
    BuiltInFunc(BuiltInFunc),
    Char(char),
    Cons(Rc<RefCell<Cons>>),
    EmptyCons,
    Dbl(f64),
    Int(i64),
    LambdaVal(LambdaVal),
    Str(String),
    Symbol(String),
    Vec(Rc<RefCell<Vector>>),
}
