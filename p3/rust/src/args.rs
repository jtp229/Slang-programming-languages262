use std::env;

/// Modes gives a programmatic name to each of the command-line options to the
/// program
///
/// NB: I was trying to avoid using external crates, so slang does its own
///     argument parsing, instead of using clap.
#[derive(Clone, PartialEq)]
pub enum Modes {
    Help,
    Interpret,
}

/// Our program is a command-line program, and it can be configured by passing
/// arguments to it on the command line (such as `-help`). By encapsulating each
/// command-line option in one of these ArgDesc objects, we can make our code a
/// little less brittle to changes.
struct ArgDesc {
    /// The text of the command-line argument (e.g., `-help`)
    arg: &'static str,
    /// The text to display in a help message (e.g., `Display a help message`)
    desc: &'static str,
    /// The programmatic identifier for this command line option
    arg_id: Modes,
}

impl ArgDesc {
    /// Create an ArgDesc from its parts
    ///
    /// NB: There's nothing special about constructing an ArgDesc, but having
    ///     this impl reduces lines of code below.
    ///
    /// @param arg    The text of the command-line argument
    /// @param desc   The text to display in a help message
    /// @param arg_id The programmatic identifier for this command line option
    ///
    /// @return A new ArgDesc object
    fn new(arg: &'static str, desc: &'static str, arg_id: Modes) -> ArgDesc {
        ArgDesc { arg, desc, arg_id }
    }
}

/// The Args object describes the available command-line arguments, and records
/// which options were provided to the program
pub struct Args {
    /// An array with the printable options for how to use the program
    options: Vec<ArgDesc>,

    /// The selected mode of operation
    pub mode: Modes,

    /// The file to load and run (empty in help mode)
    pub file_name: String,
}

impl Args {
    /// Use the command-line arguments to configure this object so that it
    /// describes the desired behavior of the program.
    ///
    /// NB: If the user does not provide exactly one mode and exactly one file
    ///     name, then the help option will be enabled automatically.
    ///
    /// @return A new Args object configured from the command line
    pub fn new() -> Args {
        // Set up the hard-coded options
        let options = vec![
            ArgDesc::new("-help", "Display this message and exit", Modes::Help),
            ArgDesc::new("-interpret", "Interpret XML AST", Modes::Interpret),
        ];

        // Counters for making sure the usage is correct
        let mut num_modes = 0;
        let mut num_names = 0;

        // Parse the command line to figure out which mode is requested, and
        // which file name to use
        //
        // NB: This is O(n^2), but it's OK, since `n` will never be large
        let mut mode = Modes::Help;
        let mut file_name = String::new();
        for arg in env::args().skip(1) {
            let mut found = false;
            for o in &options {
                if o.arg == arg {
                    mode = o.arg_id.clone();
                    num_modes += 1;
                    found = true;
                    break;
                }
            }
            if !found {
                file_name = arg;
                num_names += 1;
            }
        }

        // Only move out of help mode if the args were valid
        if num_modes == 1 && num_names == 1 {
            Args {
                options,
                mode,
                file_name,
            }
        } else {
            Args {
                options,
                mode: Modes::Help,
                file_name,
            }
        }
    }

    /// Display a help message that describes how to use this program
    pub fn print_help(&self) {
        // Print the generic information
        println!("rslang -- An interpreter for a Scheme-like language (Rust version)");
        println!("  Usage: rslang [mode] [filename]");
        println!("    * The named file will be loaded and evaluated");
        println!("  Modes:");
        // Compute the max length of an arg, so we can make a nice list of argument options
        let mut max_len = 0;
        for o in &self.options {
            max_len = std::cmp::max(max_len, o.arg.len());
        }
        // Print the list of arguments
        for o in &self.options {
            println!("    {:<pad$}  {}", o.arg, o.desc, pad = max_len);
        }
    }
}
