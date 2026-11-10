# Slang: Type Checking and Optimization

The tests in `tests/` check every behavior described here, including the exact text of every warning and the exact XML of every optimized program.
If this document and a test disagree, the test is correct (and please tell me!).

## Modes

- `-typecheck`: The first type warning in the program, or nothing if there are no warnings.
- `-optimize`: The first type warning, if there is one.  Otherwise, the optimized AST, as XML.

The top-level expressions are checked in order, sharing one type environment, so a `define` is visible to the expressions after it.
Checking stops at the first warning.
`-optimize` never optimizes a program that has a warning.

## Part 1: Type Checking

Every expression has one of these types: `INT`, `DBL`, `BOOL`, `CHAR`, `STR`, `SYMBOL`, `CONS` (a Cons or `()`), `VEC`, `LAMBDA` (a function), or `RUNTIME`.
`RUNTIME` means "unknown until the program runs", and it never causes a warning.
`INT` and `DBL` are different types.

### The type of each expression

| Expression                                    | Type                                                                                      |
|-----------------------------------------------|-------------------------------------------------------------------------------------------|
| `Int`, `Dbl`, `Bool`, `Char`, `Str`, `Symbol` | `INT`, `DBL`, `BOOL`, `CHAR`, `STR`, `SYMBOL`                                             |
| a Cons, `()`                                  | `CONS`                                                                                    |
| a Vec                                         | `VEC`                                                                                     |
| `quote` or `'`                                | `RUNTIME`                                                                                 |
| an identifier                                 | its type in the type environment; `RUNTIME` if it isn't there or its name is *unstable*   |
| `(define x e)`                                | the type of `e`; `x` gets the type of `e`                                                 |
| `(define (f x ...) body...)`                  | `LAMBDA`; `f` gets type `LAMBDA`, and each parameter is `RUNTIME` in the body             |
| `(lambda (x ...) body...)`                    | `LAMBDA`; each parameter is `RUNTIME` in the body                                         |
| `(set! x e)`                                  | the type of `e`                                                                           |
| `(if c a b)`                                  | the type of `a` and `b`; `RUNTIME` if either is `RUNTIME` (see Warnings)                  |
| `(cond ...)`                                  | the type of the first branch (see Warnings)                                               |
| `(begin e ...)`                               | the type of the last expression                                                           |
| `(let ((x e) ...) body...)`                   | the type of the last body expression; each `x` gets the type of its `e`                   |
| `(and ...)`, `(or ...)`                       | the type of the arms, if they all have the same type; otherwise `RUNTIME` (not a warning) |
| `(apply f args)`                              | `RUNTIME`                                                                                 |
| a call to a built-in                          | given by the built-in's signature (see below)                                             |
| a call to a `lambda` expression               | the type of the last body expression, with each parameter given the type of its argument  |
| any other call                                | `RUNTIME`                                                                                 |

- `display` returns its argument, and a function `define` has the function as its value.
- The type environment starts with every built-in function (type `LAMBDA`) and `pi`, `e`, `tau`, `inf+`, `inf-`, and `nan` (type `DBL`).
- Each `lambda`, function `define`, and `let` gets a new, nested type environment.
- You may assume that no program binds the name of a built-in function or constant (with a `define`, `set!`, parameter, or `let` variable).

### Unstable names

Before checking anything, the checker finds the program's **unstable** names.
A name is unstable if it is the target of a `set!` anywhere, if it is defined by a `define` that isn't a top-level expression, or if it is defined by more than one top-level `define`.
Every use of an unstable name has type `RUNTIME`.
Parameters and `let` variables do not, by themselves, make a name unstable.

### Warnings

The checker visits sub-expressions left to right, and checks each sub-expression before the expression that contains it.
It reports the first problem it finds.

| Problem                                                                               |
|---------------------------------------------------------------------------------------|
| the branches of an `if` have different types, and neither is `RUNTIME`                |
| the last branch of a `cond` doesn't have the literal test `#t`                        |
| a `cond` branch's type differs from the first branch's type, and neither is `RUNTIME` |
| the second part of an `apply` has a type other than `CONS` or `RUNTIME`               |
| a call to a `lambda` expression has the wrong number of arguments                     |
| a call to a built-in that takes exactly *n* arguments has a different number          |
| an argument to a built-in has a type that isn't allowed (and isn't `RUNTIME`)         |

- `cond`: the final `#t` test is checked first, and if it is missing, nothing else in the `cond` is checked.
  The type of a branch is the type of its last expression, and each later branch is compared, in order, to the first branch.
- Built-in calls: the number of arguments is checked first, then each argument's type, left to right.
  `<i>` counts from 1.
- The arithmetic built-ins and comparisons accept zero arguments without a warning.

### Built-in signatures

*num* means `INT` or `DBL`, *any* means any type, and `...` means "any number of these".

| Built-ins                                                | Parameters        | Result                                                   |
|----------------------------------------------------------|-------------------|----------------------------------------------------------|
| `+` `-` `*` `/` `%`                                      | *num*...          | `INT` if every argument is `INT`; `DBL` if any is `DBL`; |
|                                                          |                   | `RUNTIME` if any is `RUNTIME`                            |
| `<` `<=` `=` `>` `>=`                                    | *num*...          | `BOOL`                                                   |
| `abs`                                                    | *num*             | the type of its argument, as for `+`                     |
| `sqrt` `sin` `cos` `tan` `asin` `acos` `atan`            | *num*             | `DBL`                                                    |
| `sinh` `cosh` `tanh` `log10` `loge`                      | *num*             | `DBL`                                                    |
| `pow`                                                    | *num* *num*       | `DBL`                                                    |
| `integer?` `double?` `number?` `symbol?` `procedure?`    | *any*             | `BOOL`                                                   |
| `string?` `char?` `vector?` `list?` `null?` `nan?` `not` | *any*             | `BOOL`                                                   |
| `integer->double`                                        | `INT`             | `DBL`                                                    |
| `double->integer`                                        | `DBL`             | `INT`                                                    |
| `integer->string`                                        | `INT`             | `STR`                                                    |
| `double->string`                                         | `DBL`             | `STR`                                                    |
| `car` `cdr`                                              | `CONS`            | `RUNTIME`                                                |
| `cons`                                                   | *any* *any*       | `CONS`                                                   |
| `list`                                                   | *any*...          | `CONS`                                                   |
| `set-car!` `set-cdr!`                                    | `CONS` *any*      | `RUNTIME`                                                |
| `string-append`                                          | `STR`...          | `STR`                                                    |
| `string-length`                                          | `STR`             | `INT`                                                    |
| `substring`                                              | `STR` `INT` `INT` | `STR`                                                    |
| `string-char-at`                                         | `STR` `INT`       | `CHAR`                                                   |
| `char-equal?`                                            | `CHAR` `CHAR`     | `BOOL`                                                   |
| `char->integer`                                          | `CHAR`            | `INT`                                                    |
| `integer->char`                                          | `INT`             | `CHAR`                                                   |
| `char->string`                                           | `CHAR`            | `STR`                                                    |
| `string->integer`                                        | `STR`             | `INT`                                                    |
| `string->double`                                         | `STR`             | `DBL`                                                    |
| `vector`                                                 | *any*...          | `VEC`                                                    |
| `vector-init`                                            | `INT` *any*       | `VEC`                                                    |
| `vector-length`                                          | `VEC`             | `INT`                                                    |
| `vector-get`                                             | `VEC` `INT`       | `RUNTIME`                                                |
| `vector-set!`                                            | `VEC` `INT` *any* | `RUNTIME`                                                |
| `display`                                                | *any*             | the type of its argument                                 |

## Part 2: The Optimizer

The optimizer applies four passes, in this order, to the whole program: constant folding, constant propagation, strength reduction, and dead-code elimination.
It repeats all four until a full round changes nothing, or 50 rounds have run.
No pass looks inside quoted data (`quote`, `'`) or inside Cons and Vec literals.

**Types.** "The type of `e`" means the tag that the checker gave `e`.
The optimizer never runs the checker again.
A node that the optimizer creates has its obvious type: a literal has its own type, and the `(+ e e)` made by strength reduction has the type of `e`.

**Terms.** A **literal** is an `Int`, `Dbl`, `Bool`, `Char`, or `Str` node.
An expression is **pure** if it is a literal or an identifier.

### 1. Constant Folding

Sub-expressions are folded first, so folding works from the bottom up.

- A call to `+`, `-`, `*`, `/`, or `%` whose arguments are all `Int` or `Dbl` literals becomes a literal holding the result, computed as the standard library would.
  The call is not folded if it would divide by zero, or produce infinite or NaN.
- A call to `<`, `<=`, `=`, `>`, or `>=` whose arguments are all `Int` or `Dbl` literals becomes `#t` or `#f`.
- `(not #t)` becomes `#f`, and `(not #f)` becomes `#t`.
- `(and #f ...)` becomes `#f`, and `(or #t ...)` becomes `#t`, no matter what follows the first argument.
- `(if #t a b)` becomes `a`, and `(if #f a b)` becomes `b`.
- A call to `string-append` whose arguments are all `Str` literals (including none at all) becomes a single `Str` literal.
- `(string-length s)`, where `s` is a `Str` literal, becomes an `Int` literal.
- A call to a type predicate, `(p e)`, where `e` is pure and its type is not `RUNTIME`, becomes `#t` or `#f`, according to this table.
  `procedure?` and `nan?` are never folded.

  | Predicate        | `#t` when the type of `e` is | Otherwise |
  |------------------|------------------------------|-----------|
  | `integer?`       | `INT`                        | `#f`      |
  | `double?`        | `DBL`                        | `#f`      |
  | `number?`        | `INT` or `DBL`               | `#f`      |
  | `symbol?`        | `SYMBOL`                     | `#f`      |
  | `string?`        | `STR`                        | `#f`      |
  | `char?`          | `CHAR`                       | `#f`      |
  | `vector?`        | `VEC`                        | `#f`      |
  | `list?`, `null?` | never (`CONS` is not folded) | `#f`      |

### 2. Constant Propagation

A **constant** is a name that isn't unstable and is defined by either a top-level `(define x <literal>)` (not inside a `begin`, function, `let`, etc.) or a `(let ((x <literal>) ...) ...)` binding at any depth.
Each use of a constant, after its definition and within its scope, is replaced by its literal value.
A nested binding of the same name (a parameter, a `let` variable, or any `define` inside a body) hides the outer constant.
The `define` or `let` binding itself is never removed.

### 3. Strength Reduction

These rules only apply to calls with exactly two arguments.
`0`, `1`, and `2` are `Int` literals, and `1.0` is a `Dbl` literal.

| Expression               | Becomes   | Condition                          |
|--------------------------|-----------|------------------------------------|
| `(* e 1)`, `(* 1 e)`     | `e`       | `e` is `INT` or `DBL`              |
| `(+ e 0)`, `(+ 0 e)`     | `e`       | `e` is `INT` or `DBL`              |
| `(- e 0)`                | `e`       | `e` is `INT` or `DBL`              |
| `(/ e 1)`                | `e`       | `e` is `INT` or `DBL`              |
| `(* e 1.0)`, `(* 1.0 e)` | `e`       | `e` is `DBL`                       |
| `(/ e 1.0)`              | `e`       | `e` is `DBL`                       |
| `(* e 0)`, `(* 0 e)`     | `0`       | `e` is pure, and is `INT`          |
| `(* e 2)`, `(* 2 e)`     | `(+ e e)` | `e` is pure, and is `INT` or `DBL` |

### 4. Dead-Code Elimination

In the body of a `begin`, `let`, `lambda`, or function `define`, or in a `cond` branch, every pure expression except the last one is removed.
Then, a `begin` that holds exactly one expression is replaced by that expression.
Top-level expressions are never removed.
