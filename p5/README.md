# CSE 262: Programming Assignment 5: Type Checking and Optimizing "slang"

The semester-long project in CSE 262 is to build a full interpreter for slang, a Scheme-like language.
In p3 and p4, you wrote an interpreter and its standard library.
In this assignment, you will write two tools that run *before* the interpreter: a static type checker, and an optimizer that uses the type checker's results.

## Project Details

Both tools read a file containing an XML AST, in the same format as in p2-p4.
In `-typecheck` mode, your program prints the first type warning in the program, or nothing if there are none.
In `-optimize` mode, it type checks the program, and then (if there were no warnings) prints the optimized AST as XML.
Code for reading and writing the XML is provided.

The rules for both tools are in `docs/TypecheckAndOptimize.md`.
You should read it carefully before you start.

The type checker tags every AST node with its type.
The optimizer then uses those tags to decide which optimizations are safe.
For example, `(* e 1)` can only become `e` when the type checker has proven that `e` is a number.
The type checker doesn't just find mistakes.
It is what makes the optimizer possible.

## What To Expect

You will implement the type checker and optimizer twice: once in Go, and once in Scheme.
In each language, you will work in two files:

| Language | Type checker             | Optimizer              |
|----------|--------------------------|------------------------|
| Go       | `go/typechecker.go`      | `go/optimizer.go`      |
| Scheme   | `scheme/typechecker.scm` | `scheme/optimizer.scm` |

Each of these files has a few functions marked `TODO: implement this`.
Those are the entry points that the rest of the program calls, so you must keep their names and parameters.
You will probbaly need to add many more functions of your own, especially in Scheme, where it's good programming style to break the execution into small, reusable functions.

| Go                  | Scheme                | What it does                                      |
|---------------------|-----------------------|---------------------------------------------------|
| `findUnstableNames` | `find-unstable-names` | Find the program's unstable names (see the spec)  |
| `typecheckAst`      | `typecheck-ast`       | Check one top-level expression, and tag its nodes |
| `optimizeForest`    | `optimize-forest`     | Run the four optimization passes to a fixed point |

Be careful with `findUnstableNames` / `find-unstable-names`.
Its starter version returns an empty set, so nothing fails when you forget to write it.
Only a few tests depend on it, and they are easy to overlook.

## Provided Code

You should not need to edit `ast.go`, `parser_xml.go`, or `slang.go` (Go), or `astnodes.scm`, `parser_xml.scm`, `xml_helpers.scm`, or `slang.scm` (Scheme).
Every AST node has a type tag: use `Type()` and `SetType()` in Go, and `ast-get-type` and `ast-set-type!` in Scheme.

The type checker files also give you the type environment and the table of built-in signatures, since writing them yourself would just mean copying the spec:

| Go                                                  | Scheme                                                                                  |
|-----------------------------------------------------|-----------------------------------------------------------------------------------------|
| `TypeEnv`, `makeDefaultTypeEnv`, `makeInnerTypeEnv` | `make-type-env`, `type-env-push`, `type-env-put!`, `type-env-get`, `type-env-unstable?` |
| `Signature`, `getBuiltinSignature`                  | `get-builtin-signature`                                                                 |

### Arithmetic folding is provided: `foldArith`, `foldCompare`, and `fold-arith`

**You do not need to write the code that computes the result of a folded arithmetic or comparison call.**
It is already in the optimizer files, and you should use it as-is, so that your optimizer computes math values exactly the same as mine.

| Go                                                     | Scheme                         |
|--------------------------------------------------------|--------------------------------|
| `foldArith(op string, args []AstNode) (AstNode, bool)` | `(fold-arith op args)`         |
| `foldCompare(op string, args []AstNode) AstNode`       | (also `fold-arith`)            |
| `allLiteralNumeric(args []AstNode) bool`               | `(all-numeric-literals? args)` |

- `foldArith` (Go) handles `+`, `-`, `*`, `/`, and `%`.
  It returns the new literal (already tagged with its type) and `true`.
  It returns `false` when the call must not be folded: when it would divide an `Int` by an `Int` zero, or when the result would be an infinite or NaN `Dbl`.
  When it returns `false`, leave the call unchanged.
- `foldCompare` (Go) handles `<`, `<=`, `=`, `>`, and `>=`.
- `fold-arith` (Scheme) handles all ten operators.
  It returns the new literal (already tagged), or `#f` when the call must not be folded.
- **Before you call any of them, you must check** that the call's function is one of those operators, that it has **at least one** argument, and that **every** argument is an `Int` or `Dbl` literal (use `allLiteralNumeric` or `all-numeric-literals?`).
  They do not check this for you.
  A call with no arguments (like `(+)`) is a run-time error, so it is never folded.

Deciding *when* to fold, and walking the tree to find the calls, is your job.
The other folding rules (`not`, `and`, `or`, `if`, `string-append`, `string-length`, and the type predicates) are not provided.

The Go optimizer also provides `boolNode`, `intLit`, `dblLit`, and `strLit`, which make new literal nodes that are already tagged with their types.
The Scheme optimizer provides `with-type` (tag a new node), `same-type` (give a rebuilt node the type of the node it replaces), and `make-bool-lit`.
Remember that every node the optimizer creates needs the right type tag, because later passes read the tags.

## Testing

From this folder, run `./test.sh <lang> <mode>` (where `<lang>` is `go` or `scheme`, and `<mode>` is `typecheck` or `optimize`) to build your code, run it on every test input, and save the results in `results/<mode>/<lang>/`.
Then run `./compare.sh <lang> <mode>` to see which results differ from the expected outputs.
Each file in `tests/<mode>/inputs` is an XML AST, and the file with the same name in `tests/<mode>/outputs` is the expected result.

At first, every test fails, with a "not implemented" message.
The `optimize` tests type check the program first, so you will want the type checker working before you start on the optimizer.

The warnings are designed to be *easy to grade*, not *good*.
I expect your code to produce the exact same outputs as my test cases.
I might take off points if it doesn't.

## Tips and Reminders

**Start Early**.

Read the spec all the way through before you write any code.
Many of the rules only make sense once you see how the optimizer uses the types.

Get the type checker passing all of the `typecheck` tests before you start the optimizer.
Then write the optimizer one pass at a time, in order.

Please be careful about not committing unnecessary files into your repository.
In particular, do not commit `go/slang` or `results`.

## Grading

Each of the four parts of the assignment will be worth 25% of your grade.
As a reminder, the four parts are:

- Go type checking
- Go optimization
- Scheme type checking
- Scheme optimization

You should be sure to comment your code.
Comments will not be graded extensively, but if they are not present, or are not appropriate for the assignment, I may deduct points.

## Collaboration and Getting Help

Students may work in teams of 2 for this assignment.
**I strongly recommend working with a partner.**
If you plan to work in a team, you must notify me by Nov 13th, so that I can set up your repository access.
If you are working in a team, you should **pair program** for the entire assignment.
After all, your goal is to learn together, not to each learn a fraction of the material.

If you require help, you may seek it from any of the following sources:

- The professor and TAs, via office hours or Piazza
- The Internet, as long as you use the Internet as a read-only resource and do not post requests for help to online sites.

It is not appropriate to share code with past or current CSE 262 students, unless you are sharing code with your teammate.

There are two tricky things about this assignment:

- The first is that the spec is long and a bit hard to follow.
  You really should ask questions on Piazza or in class.
  The spec does not define a "perfect" optimizer or type checker.
  It defines components that are realistic for this class.
  You'll want to understand what *not* to do, so that you don't spend lots of time making something that is *too good* and thus *does not pass the tests*.
- The other is that you have to write nontrivial Scheme code.
  It's very reasonable to turn to an AI agent for help here, but remember that in the end, you need to understand the code you turn in.
  I'd suggest that you do Go first, then work on converting it to Scheme.
  As you do, focus on how you do things differently in Scheme than in Go.

As always, you should be using AI responsibly.
Remember: Your goal is to learn.

## Deadline

You should be done with this assignment before 11:59 PM on Nov 20th.
Please be sure to `git commit` and `git push` before that time, so that I can promptly collect and grade your work.
