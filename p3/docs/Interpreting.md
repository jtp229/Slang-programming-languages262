# Scheme-Like Language (slang): Interpreting

This document informally describes the semantics and behavior of each type of expression in slang.  
It also discusses some other things you'll want to keep in mind.

## Error Propagation

When a slang expression cannot produce a result (e.g., because of invalid operands to a function), it will return an error *value*.  
Error values immediately propagate "all the way to the top".
Consider this code:

```scheme
(if #f (+ 1 2) (begin (+ 1 2) (- 3 'a) (+ 4 5)))
```

The second argument to `begin` will return an error, and thus `begin` must immediately return that error.
This, in turn, means that `if` will return an error, even though it might seem that it would be OK to return `9`.

## Quoted And Non-Quoted Constants

A constant value in the AST, whether quoted or not, always evaluates to the equivalent Value type.
Note that Value and AST Node types are distinct.
A constant is not evaluated by returning the AST node.

This is particularly important for Cons and Vec AstNodes.
The interpreter will need to interpret each element.

## Identifiers

The provided environment type should make this part quite easy :)

## The Default Form (CallNode)

To interpret the default form, you must interpret the first argument, and then determine if it is callable (a BuiltInFunc or a LambdaVal).
If not, an error should result.
Otherwise, the arguments should be evaluated, in order.
BuiltIns handle their own type checking, so you should be able to immediately call the builtin with an array of argument values, if the callable is a builtin.
Otherwise, you'll need to make a new environment, and then evaluate the lambda within that environment.

## `(and expr+)`

`and` has short-circuit evaluation.  Note that `and` can return values other
than `#t` and `#f`

## `(or expr+)`

`or` has short-circuit evaluation.  Note that `or` can return values other
than `#t` and `#f`

## `(define id expr)` (Define-Variable)

This should evaluate the expression to produce a value, and then create a binding in the current environment for that variable.
Note that unlike Scheme, slang also returns the value of the variable as the value of the `define`.

## `(define (id+) expr+)` (Define-Function)

This is easier than you think.
Interpreting a function definition just means creating a LambdaVal.

## `(if expr0 expr1 expr2)`

This is another straightforward one.
The main things to remember are that there are many possible values to indicate "true", and that you should not evaluate both `expr` and `expr2`.

## `(cond (expr0 expr+)+)`

This behavior should be familiar by now.
Remember that the value of the cond is the value of the last expression in the `expr+` clause.
Note that slang requires you to return an error if no cond arm's first expression evaluates to true.
The parser guarantees that every clause has at least one `expr`.

## `(lambda (id*) )`

This is just like Define-Function.

## `(set!)`

As long as you understand the provided methods of the Environment, this won't be tricky.

## `(begin)`

This should be pretty straightforward too.

## `(let)`

This is compensating for the lack of macros in our language.
But it's also not hard.
The main challenge is to be sure to make a new environment in which to bind the names.

## `(apply)`

The hardest part of `apply` is remembering what it means.
Once you remember, then it's just like Call (the default form).

## `(quote)`

We won't worry about Scheme's homoiconic properties.
Consequently, Quote just evaluates to whatever you get by evaluating its datum.
