# CSE 262: Programming Assignment 4: The "slang" Standard Library

The semester-long project in CSE 262 is to build a full interpreter for slang, a Scheme-like language.
In the previous assignment, you wrote an interpreter in Rust and in Python.
Its only built-in functions were `+` and `=`.
In this assignment, you will write the rest of the standard library: the math, character, string, list, and vector functions, equality, and `display`.
You will write most of it in TypeScript, but there are a few small functions you'll write in Scheme.

## Project Details

A simple interpreter is provided for you, and you should not need to edit it.
Like the one in p3, it reads a file containing an XML AST, and prints the value of each top-level expression.
This interpreter is very small: it only understands calls to built-in functions, variable definitions, and data such as numbers, strings, lists, and vectors.
This is all you need to do the assignment.

Your job is to implement the standard library for Slang.
The requirements for the standard library are described in `docs/StdLib.md`.
You should read it carefully before you start.

You will only need to work in `typescript/src/makeStdLib.ts`.
Right now it holds `+` and `=`.
There are over 50 functions to implement.
If you write each one the way `+` and `=` are written, you will write nearly the same code over and over, and it will be thousands of lines.
Part of the assignment is to notice that, and to find a better way to organize your code.
You should think about higher-order functions.
This will be unintuitive at first, so you should wait until you've implemented a few functions *the ugly way*, then think about what helper functions you need.
For reference, my solution has 610 lines of helpers, and then only 236 lines to implement all of the library.
Each library function (other than display) was just one line.

(Note: I intentionally did not give any hints about my implementation in the provided code.
If you ask on Piazza, I'll share some hints, but I really want you to think about how to use higher-order functions, so I'm not too keen to share my solution... the thinking is the main goal here!)

## Scheme Exercises

The `scheme` folder has a set of small problems, to be solved in Scheme.
These are also parts of the standard library, but they can be implemented in pure Scheme, once the standard library is working.
See `scheme/README.md` for details.

## Testing

From this folder, run `./test.sh typescript` to build your TypeScript code, run it on every test input, and save the results in `results/typescript/`.
Then run `./compare.sh typescript` to see which results differ from the expected outputs.
Each file in `tests/inputs` is an XML AST, and the file with the same name in `tests/outputs` is the expected result.
There are tests for every function in `docs/StdLib.md`, and for the errors each function can report.

At first, every TypeScript test fails.
Many tests use `display`, so you will want to write it first, but be sure to understand the somewhat unintuitive behavior of `display` in this assignment.
It's like that to help with testing, even though it's annoying.

When a run-time error occurs, the interpreter prints the error and stops.
The error messages are designed to be *easy to grade*, not *good*.
I expect your interpreter to produce the exact same outputs as my test cases.
I might take off points if it doesn't.

Note that I did not provide test cases for the Scheme part of the assignment.
I have them, but I'm not going to share them, because I want you to be thinking about how to verify your code.
Please be sure to only use idioms that are supported by slang.
Obviously, this means no `let loop` syntax.
But it also means you should only be using parts of the standard library that are in this assignment.

## Tips and Reminders

**Start Early**.

In TypeScript, get a few functions working first, and then look for patterns.
It is much easier to reorganize code that works than to design the perfect structure before you've written any.

Please be careful about not committing unnecessary files into your repository.
In particular, do not commit `typescript/node_modules`, `typescript/dist`, or `results`.

## Grading

The TypeScript code will be worth 80% of your grade, and the Scheme code will be worth 20%.

You should be sure to comment your code.
Comments will not be graded extensively, but if they are not present, or are not appropriate for the assignment, I may deduct points.

Please be sure to use your tools well.
For example, Visual Studio Code has outstanding TypeScript support.
The assignment will be a lot harder if you're not leveraging your tools.

## Collaboration and Getting Help

Students may work in teams of 2 for this assignment.
**I strongly recommend working with a partner.**
If you plan to work in a team, you must notify me by Oct 30th, so that I can set up your repository access.
If you are working in a team, you should **pair program** for the entire assignment.
After all, your goal is to learn together, not to each learn a fraction of the material.

If you require help, you may seek it from any of the following sources:

* The professor and TAs, via office hours or Piazza
* The Internet, as long as you use the Internet as a read-only resource and do not post requests for help to online sites.

It is not appropriate to share code with past or current CSE 262 students, unless you are sharing code with your teammate.

StackOverflow is a wonderful tool for professional software engineers.
It is a horrible place to ask for help as a student.
For this assignment, you should feel free to use StackOverflow, but only as a *read only* resource.
In this class, you should **never** need to ask a question on StackOverflow.

In this assignment, the interesting part is noticing the pattern in the library and designing code to take advantage of it.
It's easy to use GenAI to crank out many lines of code per function.
You won't get full credit if you do that.
It's also easy to use GenAI to find a way to make your code elegant and functional and succinct.
You won't learn much if you let AI do that for you.
Remember: Your goal is to learn.

## Deadline

You should be done with this assignment before 11:59 PM on Nov 6th.
Please be sure to `git commit` and `git push` before that time, so that I can promptly collect and grade your work.
