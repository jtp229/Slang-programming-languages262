# slang: The Standard Library

This document describes the behavior of each function in the slang standard library.
The tests in `tests/` check every behavior described here, including the exact text of error messages.
If this document and a test disagree, the test is correct (and please tell me!).

## Conventions

### Notation

A call is written `(name argument...)`.
In the signatures below, `n...` means "one or more numbers", and `x...` means "zero or more values", as stated in each row.
An example such as `(+ 1 2)` is `3` means that evaluating `(+ 1 2)` produces the value `3`.
When we say that a function takes "exactly one argument", any other number of arguments is an error.

Your functions are called by the interpreter, which is provided.
It evaluates each top-level expression in the program, and prints the resulting value on its own line.
(See *Output*, below, for what that looks like.)

### Types

Slang has these kinds of values: Int, Dbl, Bool (`#t`, `#f`), Char, Str, Symbol, Cons, the empty list `()`, Vec, and built-in functions.

### Checking arguments

Every function checks its arguments *in this order*, and reports the first problem it finds:

1. The number of arguments.
2. The type of each argument, from left to right.
3. The range of any index or size argument.

"Exactly N" and "one or more" below refer to step 1.

### Errors

When a function is misused, it returns an error instead of a value.
The interpreter prints the error, and then stops: it does not evaluate any remaining top-level expressions.
The error messages are listed in the table at the end.
Your messages must match exactly.

### Numbers

Many math functions take Ints and Dbls.
Unless it says otherwise, a function that takes numbers returns a Dbl if *any* argument is a Dbl, and an Int otherwise.
A Dbl whose value is whole prints without a decimal point (`3.0` prints as `3`).

## Arithmetic

All of these take one or more numbers.

| Function   | Result                                                                                     |
|------------|--------------------------------------------------------------------------------------------|
| `(+ n...)` | The sum. `(+ 1)` is `1`.                                                                   |
| `(- n...)` | The first number minus each of the others, in order.                                       |
|            | With one argument, the result is that argument negated: `(- 5)` is `-5`.                   |
| `(* n...)` | The product.                                                                               |
| `(/ n...)` | The first number divided by each of the others, in order.                                  |
|            | If every number so far is an Int, the division is integer division, truncated toward zero: |
|            | `(/ 7 2)` is `3`, `(/ 21 2 2)` is `5`, and `(/ 7 2.0)` is `3.5`.                           |
| `(% n...)` | The remainder, in the same way: `(% 7 2)` is `1`, `(% 7.5 2)` is `1.5`.                    |

Dividing an Int by an Int zero (with `/` or `%`) is an error, including later in the argument list: `(/ 10 2 0)`.
Floating-point division by zero is not specially handled, and none of the tests use it.

## Comparison

All of these take one or more numbers and return a Bool.
They compare each pair of *adjacent* numbers, and return `#f` as soon as one pair fails.
With one argument they return `#t`.

| Function    | True when the numbers are...                                                        |
|-------------|-------------------------------------------------------------------------------------|
| `(< n...)`  | strictly increasing: `(< 1 2 3)` is `#t`, `(< 1 3 2)` is `#f`, `(< 1 1 2)` is `#f`. |
| `(<= n...)` | non-decreasing.                                                                     |
| `(> n...)`  | strictly decreasing.                                                                |
| `(>= n...)` | non-increasing.                                                                     |
| `(= n...)`  | all equal, by numeric value: `(= 1 1.0)` is `#t`.                                   |

(`=` is about *numeric* equality, and an Int and a Dbl with the same value are equal.
See `eqv?` and `equal?` below for equality that cares about type.)

## Math Functions

| Function                           | Result                                                  |
|------------------------------------|---------------------------------------------------------|
| `(abs n)`                          | The absolute value. An Int for an Int, a Dbl for a Dbl. |
| `(sqrt n)`                         | The square root, as a Dbl.                              |
| `(pow base exp)`                   | `base` raised to `exp`, as a Dbl. Exactly two numbers.  |
| `(log10 n)`                        | The base-10 logarithm, as a Dbl.                        |
| `(loge n)`                         | The natural logarithm, as a Dbl.                        |
| `(sin n)`, `(cos n)`, `(tan n)`    | Trigonometric functions, as a Dbl.                      |
| `(asin n)`, `(acos n)`, `(atan n)` | Inverse trigonometric functions, as a Dbl.              |
| `(sinh n)`, `(cosh n)`, `(tanh n)` | Hyperbolic functions, as a Dbl.                         |
| `(nan? n)`                         | `#t` if `n` is NaN.                                     |

All of these except `pow` take exactly one number, and `pow` takes exactly two.

## Constants

These names are bound to Dbls:

`pi`, `e`, `tau` (which is $2 \times \pi$), `inf+` (positive infinity), `inf-` (negative infinity), and `nan`.

(How infinity and NaN print is not tested.
Use comparisons and `nan?` when you want to check them.)

## Type Predicates

Each takes exactly one argument, of any type, and returns a Bool.

| Function         | True when the argument is...                        |
|------------------|-----------------------------------------------------|
| `(integer? x)`   | an Int                                              |
| `(double? x)`    | a Dbl                                               |
| `(number? x)`    | an Int or a Dbl                                     |
| `(string? x)`    | a Str                                               |
| `(char? x)`      | a Char                                              |
| `(symbol? x)`    | a Symbol                                            |
| `(vector? x)`    | a Vec                                               |
| `(procedure? x)` | a function (here, a built-in function)              |
| `(list? x)`      | `()`, or a Cons whose `cdr` is `()` or another Cons |
| `(null? x)`      | `()`                                                |

`(not x)` takes exactly one argument of any type.
It returns `#t` if the argument is `#f`, and `#f` for everything else, including `0`.
(In slang, every value except `#f` counts as true.)

## Conversions

Each takes exactly one argument.

| Function              | Result                                                                                    |
|-----------------------|-------------------------------------------------------------------------------------------|
| `(integer->double i)` | The Int `i`, as a Dbl.                                                                    |
| `(double->integer d)` | The Dbl `d`, as an Int, truncated toward zero: `3.9` becomes `3` and `-3.9` becomes `-3`. |
| `(integer->string i)` | The text of an Int, as a Str: `(integer->string -7)` is `"-7"`.                           |
| `(double->string d)`  | The text of a Dbl, as a Str.                                                              |
| `(string->integer s)` | The Int that `s` spells. See below.                                                       |
| `(string->double s)`  | The Dbl that `s` spells. See below.                                                       |

For `string->integer` and `string->double`, *the whole string* must spell the number, in exactly this form:

* An Int is an optional `-`, and then one or more digits: `"42"`, `"-7"`, `"007"` (which is `7`).
* A Dbl is an optional `-`, one or more digits, and then optionally a `.` followed by one or more digits: `"2.5"`, `"-0.5"`, and also `"3"` (which is the Dbl `3`).

Anything else is a `String cannot be converted` error, including an empty string, spaces anywhere, a leading `+`, extra characters (`"12abc"`), a decimal point where an Int is required (`"3.7"`), a `.` with no digit before or after it (`".5"`, `"5."`), exponents (`"1e3"`), and words (`"0x1f"`, `"Infinity"`).

## Characters

| Function              | Result                                                               |
|-----------------------|----------------------------------------------------------------------|
| `(char->integer c)`   | The character code of `c`, as an Int: `(char->integer #\a)` is `97`. |
| `(integer->char i)`   | The Char with code `i`. It is an error unless `0 <= i <= 255`.       |
| `(char->string c)`    | A one-character Str.                                                 |
| `(char-equal? c1 c2)` | `#t` if the two Chars are the same. Exactly two Chars.               |

## Strings

Indexes start at 0.

| Function                  | Result                                                                            |
|---------------------------|-----------------------------------------------------------------------------------|
| `(string-length s)`       | The number of characters in `s`, as an Int.                                       |
| `(string-append s...)`    | A new Str made by joining zero or more Strs, in order. `(string-append)` is `""`. |
| `(substring s start end)` | The characters from index `start` (included) up to index `end` (not included).    |
|                           | It is an error unless `0 <= start <= end <= (string-length s)`.                   |
| `(string-char-at s i)`    | The Char at index `i`. It is an error unless `0 <= i < (string-length s)`.        |

## Lists and Pairs

| Function         | Result                                                               |
|------------------|----------------------------------------------------------------------|
| `(cons a b)`     | A new Cons whose first field is `a` and whose second is `b`.         |
| `(car c)`        | The first field of the Cons `c`.                                     |
| `(cdr c)`        | The second field of the Cons `c`.                                    |
| `(list x...)`    | A list of zero or more values. `(list)` is `()`.                     |
| `(set-car! c x)` | Replaces the first field of the Cons `c` with `x`, and returns `x`.  |
| `(set-cdr! c x)` | Replaces the second field of the Cons `c` with `x`, and returns `x`. |

`car`, `cdr`, `set-car!`, and `set-cdr!` give an error if their first argument is not a Cons.
That includes `()`: `(car (list))` is an error.
A list is a chain of Cons cells whose last `cdr` is `()`.

## Vectors

A vector is a fixed-size, mutable, zero-indexed sequence of values.

| Function              | Result                                                                         |
|-----------------------|--------------------------------------------------------------------------------|
| `(vector x...)`       | A new Vec holding zero or more values.                                         |
| `(vector-init n x)`   | A new Vec of `n` copies of `x`. `n` must be an Int, and not negative.          |
| `(vector-length v)`   | The number of elements, as an Int.                                             |
| `(vector-get v i)`    | The element at index `i`. It is an error unless `0 <= i < (vector-length v)`.  |
| `(vector-set! v i x)` | Stores `x` at index `i`, and returns `x`. The same range rule as `vector-get`. |

## Equality

Both take exactly two arguments, of any type, and return a Bool.

**`(eqv? a b)`** asks whether `a` and `b` are the same *thing*.

* Ints, Dbls, Chars, Symbols, Bools, and `()` are `eqv?` when they have the same type and the same value.
  So `(eqv? 1 1)` is `#t`, but `(eqv? 1 1.0)` is `#f`, because one is an Int and the other is a Dbl.
  Symbols are compared by name.
* Anything else (a Str, a Cons, a Vec, a function) is `eqv?` only to *itself*.
  A copy that looks the same is not `eqv?`.
  After `(define a (list 1 2))` and `(define b a)`, `(eqv? a b)` is `#t`, but `(eqv? a (list 1 2))` is `#f`.
  Likewise, two string literals are never `eqv?`, even if they hold the same characters.

**`(equal? a b)`** asks whether `a` and `b` *look the same*.

* Strs are `equal?` if they hold the same characters.
* Conses are `equal?` if their first fields are `equal?` and their second fields are `equal?`.
* Vecs are `equal?` if they have the same length and their elements are pairwise `equal?`.
* For everything else, `equal?` is the same as `eqv?`.
  So `(equal? 1 1.0)` is `#f`.

The difference matters when values change:
after `(define a (list 1 2))` and `(define b (list 1 2))`, `(equal? a b)` is `#t` and `(eqv? a b)` is `#f`.
Once you do `(set-car! b 9)`, `(equal? a b)` becomes `#f`.

## Output

`(display x)` takes exactly one argument, of any type.
It prints the text for `x` to standard output, with *no* newline after it, and returns `x`.
Because the interpreter also prints the value of every top-level expression, followed by a newline, a call to `display` at the top level shows its output *twice*.
For example, the program

```scheme
(display "hi")
(display 5)
```

prints

```
hihi
55
```

In each line, the first copy comes from `display`, and the second is the interpreter printing the value that `display` returned.

How values are printed:

| Value             | Printed as                                                                                                |
|-------------------|-----------------------------------------------------------------------------------------------------------|
| Int, Dbl          | the number: `42`, `-7`, `1.5`, `3` (for `3.0`)                                                            |
| Bool              | `#t` or `#f`                                                                                              |
| Str               | the characters themselves, with no quotes: `hi`                                                           |
| Char              | `#\a`, except that a space is `#\space`, a newline is `#\newline`, and a tab is `#\tab`                   |
| Symbol            | a quote and the name: `'abc`                                                                              |
| `()`              | `()`                                                                                                      |
| Cons              | `(` first ` . ` second `)`, with the fields printed the same way: `(list 1 2)` prints as `(1 . (2 . ()))` |
| Vec               | `#(` the elements separated by spaces `)`: `#(1 2 3)`                                                     |
| built-in function | `Built-in Function: ` and its name: `Built-in Function: car`                                              |

## Error Messages

Every error message starts with `Run-time Error: `, followed by one of the following messages:

| Message                       | When                                                                |
|-------------------------------|---------------------------------------------------------------------|
| `Wrong number of arguments`   | A built-in is called with the wrong number of arguments.            |
| `Argument is not a number`    | An argument that must be an Int or Dbl is not.                      |
| `Argument is not an integer`  | An argument that must be an Int is not.                             |
| `Argument is not a double`    | An argument that must be a Dbl is not.                              |
| `Argument is not a string`    | An argument that must be a Str is not.                              |
| `Argument is not a character` | An argument that must be a Char is not.                             |
| `Argument is not a cons cell` | An argument that must be a Cons is not (including `()`).            |
| `Argument is not a vector`    | An argument that must be a Vec is not.                              |
| `Index out of bounds`         | A string or vector index, or a `substring` range, is out of range.  |
| `Value cannot be negative`    | `vector-init` is given a negative size.                             |
| `String cannot be converted`  | `string->integer` or `string->double` cannot understand its string. |
| `Value out of range`          | `integer->char` is given an integer that is not between 0 and 255.  |
| `Division by zero`            | `/` or `%` divides an Int by an Int zero.                           |
| `Invalid identifier`          | A name is used that is not defined.                                 |
| `Argument is not a procedure` | The first element of a call is not a function.                      |

For example, `(car 5)` prints `Run-time Error: Argument is not a cons cell`, and `(substring "abc" 1 10)` prints `Run-time Error: Index out of bounds`.
