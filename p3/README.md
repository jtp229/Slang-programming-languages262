# CSE 262: Programming Assignment 3: Interpreting "slang"

The semester-long project in CSE 262 is to build a full interpreter for a relatively complete subset of the Scheme programming language.
To avoid confusion, we will call our language "slang" (because it is a Scheme-like LANGuage).

In the first phase of the project, we *scanned* slang: given a string that purports to be slang code, we turned it into a sequence of tokens.
In the second phase, we *parsed* slang: given a sequence of tokens, we turned it into an abstract syntax tree (AST).
In this phase, we will *interpret* slang: given an AST, we will evaluate it, and print the value produced by each top-level expression.

## Project Details

Interpretation is the step that gives a program its meaning.
Your interpreter walks the AST, keeps track of which names are bound to which values, and computes the value of each expression.

For this assignment, "XML in, text out" is the rule.
Your interpreter takes a file containing an AST in the same XML format that your parser produced in the last assignment.
It prints the value of each top-level expression, one per line.
Since the input is an XML AST rather than source code, you do not need a working scanner or parser in order to test your interpreter.
Code for reading the AST from XML is provided.

The rules for evaluating each kind of AST node are in `docs/Interpreting.md`.
You should read it carefully before you start.

The standard library (math, lists, strings, vectors, `display`, ...) is the subject of the next assignment.
In this assignment, there are only two built-in functions: `+` and `=`.
They are enough to test function calls, conditionals, and recursion.

As you work on this assignment, you should try to write *idiomatic* code.
Try, as much as possible, to use language features that make your code more readable, more familiar to experts, and more succinct and expressive.

## What To Expect

You will implement the interpreter twice: once in Rust, and once in Python.
You only need to edit one file for Rust, and one file for Python.
The Rust interpreter should use the idioms that are typical of Rust: `Result` and the `?` operator for errors (not panics), `match` to work with the different node and value types, and `Rc`/`RefCell` where values must be shared.
The Python interpreter does not use classes for the AST or for values: every AST node and every value is a `dict`, with a `"type"` field that says what kind of node or value it is.
This creates a valuable opportunity to compare and contrast:
Rust is probably the most strongly typed language you'll work with in the next 10 years.
This way of writing Python is probably the least strongly typed language you'll ever work with.

When evaluating an expression produces a run-time error, your interpreter should print the error message and stop: it should not evaluate any of the remaining top-level expressions.

## The Big Challenge

To receive full credit on the assignment, you will need to implement tail-recursion optimizations.
This is easier than you think, but requires a lot of thinking.
You'll definitely want to start early, and ask questions about this.
The tests include a deeply tail-recursive function that will crash an interpreter that does not do this.

There is also a `tests` folder, which includes all of the tests that I will run against your interpreters.
Each file in `tests/inputs` is an XML AST, and the file with the same name in `tests/outputs` is the expected result.

## Testing

From this folder, run `./test.sh <lang>` (where `<lang>` is `rust` or `python`) to build your code, run it on every test input, and save the results in `results/<lang>/`.
Then run `./compare.sh <lang>` to see which results differ from the expected outputs.

Note that the error messages are designed to be *easy to grade*, not *good*.
I expect your interpreter to produce the exact same outputs as my test cases.
I will take off points if it doesn't.

## Rust Tools

Rust comes with tools that you should use as you work.
Run `cargo fmt` (from the `rust` folder) to format your code according to Rust's standard style.
Run `cargo clippy` to find code that compiles but is probably wrong or not idiomatic.
Your Rust code should be formatted with `cargo fmt`.
In the starter code, there are lots of warnings about unused code.
By the time you're done, `cargo clippy` should report nothing and `cargo build` should report no warnings.

Most editors (including Visual Studio Code with the rust-analyzer extension) can run `cargo fmt` every time you save.

## Tips and Reminders

**Start Early**.
Just reading the code and understanding what is happening takes time.

Tail calls are the trickiest part of the assignment.
Get the rest of the interpreter working first, then restructure your code so that tail calls avoid making a recursive call.

As always, please be careful about not committing unnecessary files into your repository.
In particular, do not commit the `rust/target` folder.

## Grading

The Rust and Python code will each be worth 50% of your grade.

You should be sure to comment your code.
Comments will not be graded extensively, only if their absence is egregious.
You may wish to use the provided code as a reference for how I comment code, and mimic it.
In particular, note that good comments are formatted in a way that lets the IDE read them and build tooltips for you from them.

Please be sure to use your tools well.
For example, Visual Studio Code (and emacs, and vim) have features that auto-format your code.
You should use these features, so that your code is legible.

## Collaboration and Getting Help

Students may work in teams of 2 for this assignment.
**I strongly recommend working with a partner.**
If you plan to work in a team, you must notify me by Oct 9th, so that I can set up your repository access.
If you are working in a team, you should **pair program** for the entire assignment.
After all, your goal is to learn together, not to each learn half the material.

If you require help, you may seek it from any of the following sources:

* The professor and TAs, via office hours or Piazza
* The Internet, as long as you use the Internet as a read-only resource and do not post requests for help to online sites.

It is not appropriate to share code with past or current CSE 262 students, unless you are sharing code with your teammate.

StackOverflow is a wonderful tool for professional software engineers.
It is a horrible place to ask for help as a student.
For this assignment, you should feel free to use StackOverflow, but only as a *read only* resource.
In this class, you should **never** need to ask a question on StackOverflow.

I can't keep you from using Generative AI tools on this assignment.
I can promise you that there are uses of Generative AI that will deeply hinder your learning.
I also don't think there's anything truly "boilerplate" in this assignment, so I don't think there are likely to be good uses of GenAI for this assignment.

## Deadline

You should be done with this assignment before 11:59 PM on Oct 16th.
Please be sure to `git commit` and `git push` before that time, so that I can promptly collect and grade your work.

There are many parts to this assignment, so you will probably want to `git push` frequently.
