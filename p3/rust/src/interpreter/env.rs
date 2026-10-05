use super::builtins::add_builtins;
use super::values::{BuiltInFunc, BuiltInFunction, Value};
use std::{cell::RefCell, collections::HashMap, rc::Rc};

/// The environment maps strings to values, and has a reference to the parent
/// scope
pub struct Env {
    /// The names that are defined in this scope
    pub map: HashMap<String, Value>,
    /// The parent scope
    pub outer: Option<Rc<RefCell<Env>>>,
}

impl Env {
    /// Add a key/value pair to the Environment
    ///
    /// @param key  The string name
    /// @param expr The expression that serves as the value
    pub fn put(&mut self, key: &str, expr: Value) {
        self.map.insert(key.to_string(), expr);
    }

    /// Get the value for a given key. If the key isn't mapped in this scope,
    /// look in outer scopes.
    ///
    /// @param key The key to look up
    ///
    /// @return The value associated with the key, or None
    pub fn get(&self, key: &str) -> Option<Value> {
        let res = self.map.get(key);
        match res {
            Some(v) => Some(v.clone()),
            None => match &self.outer {
                Some(e) => e.borrow().get(key),
                None => None,
            },
        }
    }

    /// Update the value for a given key. Throw an exception if the key isn't
    /// mapped.
    ///
    /// @param key  The key to update
    /// @param expr The new value for the key
    pub fn update(&mut self, key: &str, expr: Value) {
        match self.map.get(key) {
            Some(_) => {
                self.map.insert(key.to_string(), expr);
            }
            None => match &mut self.outer {
                Some(env) => env.borrow_mut().update(key, expr),
                None => panic!("Run-time Error: Identifier not found {}", key),
            },
        };
    }
}

/// Create the default environment.  The default environment is populated with
/// default values and default methods fof the built-in functions
///
/// @return The default environment
pub fn make_default_env() -> Env {
    let mut env = Env {
        map: HashMap::<String, Value>::new(),
        outer: None,
    };
    add_builtins(&mut env);
    env
}

/// Create a nested (inner) environment, suitable for a lambda
///
/// @param outer The enclosing environment
///
/// @return An environment that links to the provided `outer` environment
pub fn make_inner_env(outer: Rc<RefCell<Env>>) -> Env {
    Env {
        map: HashMap::<String, Value>::new(),
        outer: Some(outer),
    }
}

/// Put a function into the environment
///
/// @param env  The environment to add to
/// @param name The name of the function
/// @param func The built-in function to add
pub fn add_to_env(env: &mut Env, name: &'static str, func: BuiltInFunction) {
    env.put(name, Value::BuiltInFunc(BuiltInFunc { name, func }));
}
