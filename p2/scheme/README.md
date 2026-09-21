# CSE 262: Programming Assignment 2: Scheme

This folder has seven small Scheme problems to solve.
They build on the recursion-first habits from Assignment 1, and add vectors, higher-order functions, string algorithms, and a basic hash table to your toolkit.

The "Gambit" Scheme interpreter is available on the SunLab as `gsi` (type `module load gambit` first), or you can install it locally.

## Building and Running

Since `gsi` is an interpreter, the usual workflow is:

1. Write your code in the appropriate file.
2. In `gsi`, type `(load "filename.scm")` to load your file.
3. Invoke your code to test it.

When you find a bug, you'll typically need to exit the interpreter (`ctrl-d`), fix the code, and reload.

## The Problems

- `cv2s.scm`: convert a vector of characters into a string, without using `vector->list` or `list->string`.
- `l2v.scm`: convert a list into a vector, without using `list->vector`.
- `my_map.scm`: apply a function to every element of a list and return the results as a new list, without using the built-in `map`; must be tail recursive.
- `fib.scm`: compute the nth Fibonacci number using a tail-recursive helper that counts up from 0, in linear time -- not the naive exponential double-recursive definition.
- `contains-substring.scm`: check whether one string contains another, comparing characters directly rather than using built-in string search.
- `substring-wildcard.scm`: like `contains-substring`, but the pattern may use `?` as a single-character wildcard.
- `simple-hash.scm`: a hash *set* of strings, built as a vector of buckets and a closure (in the same "method receiver" style as `tree.scm` from Assignment 1), using the djb2 hash function.

## Suggestions

- You'll probably want to work on the problems in the order listed above.
- Do not use imperative constructs like `let loop`.
- Do not make any extra global functions. Use nested scopes to hide any helpers you require.
- Make sure your code works using `gsi`. Other Scheme interpreters have different syntax.
- Be sure to comment your code well. Explain *why* you solved each problem the way you did, not just what the code does.
- Make sure you're not accidentally introducing quadratic behavior (e.g., by calling an O(n) function like `length` or `append` inside a recursive loop).
- It's easy to find solutions to problems like these online. Please don't look: you'll get much less out of the assignment.
- `cv2s.scm` and `l2v.scm` need to build a fixed-size string/vector by index, so `string-set!`/`vector-set!` are expected there. Otherwise, you should not use any state-modifying forms (`set!`, `vector-set!`, etc.) except in `simple-hash.scm`.
