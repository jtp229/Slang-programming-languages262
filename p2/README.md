# CSE 262: Programming Assignment 2: Parsing "slang"

The semester-long project in CSE 262 is to build a full interpreter for a relatively complete subset of the Scheme programming language.
To avoid confusion, we will call our language "slang" (because it is a Scheme-like LANGuage).

In the first phase of the project, we *scanned* slang: given a string that purports to be slang code, we turned it into a sequence of tokens.
In this phase, we will *parse* slang: given a sequence of tokens, we will turn it into an abstract syntax tree (AST), or report that the tokens do not form a valid program.

## Project Details

Parsing is the second step in any compiler or interpreter.
It is the step that checks whether the tokens make sense as a program, and that organizes them into a tree that is easier to work with throughout the rest of the compiler/interpreter.

For this assignment, "XML in, XML out" is the rule.
Your parser takes a file of tokens in the same XML format that your scanner produced in the last assignment.
It produces an XML description of the AST.
Since the input is XML tokens rather than source code, you do not need a working scanner in order to test your parser.
Code for reading the tokens and for writing the AST is provided.

The grammar for slang, along with the rules that go beyond the grammar, is in `docs/Parsing.md`.
You should read it carefully before you start.

As you work on this assignment, you should try to write *idiomatic* code.
Try, as much as possible, to use language features that make your code more readable, more familiar to experts, and more succinct and expressive.

## What To Expect

You will implement the parser twice: once in Java, and once in Go.
The Java parser should use the object-oriented features that you know well.
The Go parser should use the idioms that are typical of Go: multiple return values for errors (not exceptions), and type switches or interfaces to work with the different node types.

In each language, the AST node types and the code for reading tokens and writing XML are provided.
You should not need to change them.
Your work is in `Parser.java` and `parser.go`, respectively.

The grammar requires two tokens of lookahead.
The token stream provided to you has support for this.

There is also a `tests` folder, which includes all of the tests that I will run against your parsers.
Each file in `tests/inputs` is a sequence of tokens, and the file with the same name in `tests/outputs` is the expected result.
The expected result is either an XML AST, or a parse error message.

## Testing

From this folder, run `./test.sh <lang>` (where `<lang>` is `java` or `go`) to build your code, run it on every test input, and save the results in `results/<lang>/`.
Then run `./compare.sh <lang>` to see which results differ from the expected outputs.

Note that the error messages are designed to be *easy to grade*, not *good*.
I expect your parser to produce the exact same outputs as my test cases.
I will take off points if it doesn't.

## Go Tools

Go comes with tools that you should use as you work.
Run `gofmt -l .` (from the `go` folder) to list files that are not formatted according to Go's standard style, and `gofmt -w <file>` to fix a file automatically.
Run `go vet` to find code that compiles but is probably wrong, such as unused results, bad format strings, and unreachable code.
Your Go code should be `gofmt`-clean, and `go vet` should report nothing.
Most editors (including Visual Studio Code with the Go extension) can run `gofmt` every time you save.

## Tips and Reminders

**Start Early**.
Just reading the code and understanding what is happening takes time.

You will probably want to do the Java code first.
Once you have a working Java parser, the Go code will mostly be a matter of translating the same ideas into a language with different conventions.

As always, please be careful about not committing unnecessary files into your repository.

## Grading

The Java code will be worth 40% of your grade.
The Go code will be worth 40% of your grade.
The Scheme code will be worth 20% of your grade.

You should be sure to comment your code.
For the parsers, comments will not be graded extensively, only if their absence is egregious.
You may wish to use the provided Java code as a reference for how I comment code, and mimic it.
In particular, note that good comments are formatted in a way that lets the IDE read them and build tooltips for you from them.

Please be sure to use your tools well.
For example, Visual Studio Code (and emacs, and vim) have features that auto-format your code.
You should use these features, so that your code is legible.

Note that the error messages are designed to be *easy to grade*, not *good*.
I expect your scanner to produce the exact same outputs as my test cases.
I will take off points if it doesn't.

## Collaboration and Getting Help

Students may work in teams of 2 for this assignment.
**I strongly recommend working with a partner.**
If you plan to work in a team, you must notify me by Sep 25th, so that I can set up your repository access.
If you are working in a team, you should **pair program** for the entire assignment.
After all, your goal is to learn together, not to each learn half the material.

If you require help, you may seek it from any of the following sources:

* The professor and TAs, via office hours or Piazza
* The Internet, as long as you use the Internet as a read-only resource and do not post requests for help to online sites.
* You should try not to use the Internet when working on the Scheme portion of the assignment.
  I cannot enforce this requirement, but you'll learn an awful lot less if you look for Scheme answers online.

It is not appropriate to share code with past or current CSE 262 students, unless you are sharing code with your teammate.

StackOverflow is a wonderful tool for professional software engineers.
It is a horrible place to ask for help as a student.
For the scanner parts of the assignment, you should feel free to use StackOverflow, but only as a *read only* resource.
In this class, you should **never** need to ask a question on StackOverflow.

I can't keep you from using Generative AI tools on this assignment.
I can promise you that there are uses of Generative AI that will deeply hinder your learning.
I also don't think there's anything truly "boilerplate" in this assignment, so I don't think there are likely to be good uses of GenAI for this assignment.

## Deadline

You should be done with this assignment before 11:59 PM on Oct 2nd, 2026.
Please be sure to `git commit` and `git push` before that time, so that I can promptly collect and grade your work.

There are many parts to this assignment, so you will probably want to `git push` frequently.
