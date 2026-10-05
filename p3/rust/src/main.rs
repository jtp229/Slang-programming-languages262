mod args;
mod interpreter;
mod parser;

use args::{Args, Modes};
use interpreter::env::{make_default_env, Env};
use interpreter::interpret::{interpret_ast, value_to_string};
use parser::xml::xml_to_ast;
use std::{cell::RefCell, fs, rc::Rc};

/// Interpret an XML AST in an environment.
///
/// @param code  The XML AST to interpret
/// @param env   The environment in which to process the code
///
/// @return Result indicating success or an error message
fn process_code(code: String, env: Rc<RefCell<Env>>) -> Result<(), String> {
    // Turn the input XML into a forest, then walk it, printing the result after
    // each tree is walked
    let forest = xml_to_ast(code);
    for e in forest {
        let node = interpret_ast(&e, env.clone())?;
        println!("{}", value_to_string(&node));
    }
    Ok(())
}

/// Run the interpreter
fn main() {
    // Parse the arguments, make sure they are valid
    let parsed_args = Args::new();

    // If help was requested, print help and then exit immediately
    if parsed_args.mode == Modes::Help {
        parsed_args.print_help();
        return;
    }

    // Create the environment, so we have it for all evaluating that we do
    let default_environment = Rc::new(RefCell::new(make_default_env()));

    // Load the file and run it.  A run-time error stops the program, after
    // printing the error message.
    let code_to_run = fs::read_to_string(&parsed_args.file_name).unwrap_or_default();
    if let Err(e) = process_code(code_to_run, default_environment) {
        println!("{}", e);
    }
}
