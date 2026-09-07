# CSE 262: Scheme Warm-Ups

This folder has seven small Scheme problems to solve.
They exist to get you more comfortable with Scheme's syntax and its recursion-first style.

The "Gambit" Scheme interpreter is available on the SunLab as `gsi` (type `module load gambit` first), or you can install it locally.
Scheme's syntax and philosophy are radically different from Java and Python.
You should think about recursion as much as possible.
Read Chapter 11 of the textbook for ideas on how to think like a Scheme programmer and write idiomatic code.

## Building and Running

Since `gsi` is an interpreter, the usual workflow is:

1. Write your code in the appropriate file.
2. In `gsi`, type `(load "filename.scm")` to load your file.
3. Invoke your code to test it, e.g. `(define x (read-list)) (my-reverse x)`.

When you find a bug, you'll typically need to exit the interpreter (`ctrl-d`), fix the code, and reload.

## The Problems

Each file has the shell of a single function for you to implement:

* `xor.scm`: compute exclusive-or using only `and`, `or`, and `not`.
* `read_list.scm`: read values from stdin via `read` until EOF, returning them (in reverse order) as a list; must use recursion, not iteration, and must not introduce any new global functions.
* `my_reverse.scm`: reverse a list without using the built-in `reverse`; must be tail recursive.
* `reduce.scm`: a general fold: apply a binary function across a list, starting from an identity value.
* `three-zip.scm`: zip three lists together element-wise; must be tail recursive, and must handle lists of different lengths by replicating the last element of any list that runs out early.
* `prime_divisors.scm`: compute the prime factorization of a number.
* `tree.scm`: a binary search tree implemented as a closure, supporting insertion and in-order/pre-order traversal.

## Suggestions

- You'll probably want to work on the assignments in the order listed above.
- Make sure your code doesn't print anything when it is loaded via `(load ...)`.
- Do not use imperative constructs like `let loop`.
- Do not make any extra global functions.
  Use nested scopes to hide any helpers you require.
- Make sure your code works using `gsi`.
  Other scheme interpreters have different syntax.
- Be sure to comment your code well.
  Explain *why* you solved each problem the way you did, not just what the code does.
- Make sure you're not accidentally introducing quadratic behavior (e.g., by calling an O(n) function like `length` or `append` inside a recursive loop).
- It's easy to find solutions to problems like these online.
  Please don't look: you'll get much less out of the assignment.
- You should not use any state modifying forms (set!, set-car!, etc.) in any of the solutions except `tree.scm`.
