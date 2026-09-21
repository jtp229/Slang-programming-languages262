# Scheme-Like Language (slang): Parsing

The following context-free grammar defines the syntax of slang.  This grammar
defines a **parse tree**, not an **abstract syntax tree**.  Note that this
grammar is not ambiguous, but it does leverage some shorthand in the
productions.  If your goal was to produce a parse tree, you would want to expand
these productions.  Since our goal is to produce an abstract syntax tree, the
shorthands in the grammar should not be a concern.

As written, the grammar requires two tokens of lookahead, and otherwise is a
straightforward "LL" grammar.  If you were to expand the productions, it would
be possible to only require one token of lookahead.

Note that in our specification, ALLCAPS elements refer to specific scanner
tokens.  The `+` and `*` symbols bind to the item immediately preceding them,
and represent "one or more" and "zero or more", respectively.  Elements whose
names are lowercase are not necessarily AST nodes.  Please consult the starter
code to find the suggested set of AST nodes.

## Productions

```bnf
<program> --> <expression>*

<expression> --> LPAREN DEFINE <identifier> <expression> RPAREN
               | LPAREN DEFINE LPAREN <identifier>+ RPAREN <expression>+ RPAREN
               | LPAREN QUOTE <datum> RPAREN
               | LPAREN LAMBDA <formals> <expression>+ RPAREN
               | LPAREN IF <expression> <expression> <expression> RPAREN
               | LPAREN SET <identifier> <expression> RPAREN
               | LPAREN AND <expression>+ RPAREN
               | LPAREN OR <expression>+ RPAREN
               | LPAREN BEGIN <expression>+ RPAREN
               | LPAREN COND <condition>+ RPAREN
               | LPAREN APPLY <expression> <expression> RPAREN
               | LPAREN LET LPAREN <letdef>+ RPAREN <expression>+ RPAREN 
               | ABBREV <datum>
               | <constant>
               | <identifier>
               | <call>

<condition> --> LPAREN <expression> <expression>+ RPAREN

<formals> --> LPAREN <identifier>* RPAREN

<call> --> LPAREN <expression>+ RPAREN

<constant> --> BOOL | INT | DBL | CHAR | STR

<datum> --> <constant> | <symbol>  | <list>  | <vector> | <cons>

<symbol> --> IDENTIFIER

<list> --> LPAREN <datum>* RPAREN

<cons> --> LPAREN <datum> DOT <datum> RPAREN

<vector> --> VEC <datum>* RPAREN

<identifier> --> IDENTIFIER

<letdef> --> LPAREN IDENTIFIER <expression> RPAREN
```

## Comparison with Scheme

* There are only 11 keywords in slang: `and`, `or`, `define`, `if`, `cond`,
  `lambda`, `set!`, `begin`, `let`, `apply`, and `quote`, each of which is a
  "special form". `<call>` is the "default form".
* Scheme is case-insensitive, but slang is case sensitive.
* This grammar does not include the `else` alias for inside of a `cond`
* In `gsi`, `and`, `or`, and `begin` are valid even when they are given zero
  arguments.  Our grammar requires at least one argument.
* Scheme supports arbitrary precision of numbers, but we only support 64-bit
  floating point (double) and 64-bit signed integer (i32) number types. The CFG
  is not concerned with invalid numerical constants, and the scanner is not
  required to detect them.
* The `lambda` and `let` special forms have some syntactic sugar: You need not
  use `begin` in order to have a `lambda` or `let` with multiple expressions.
* Our version of `apply` is simpler and less powerful than the one in Scheme
* `set!` and `define` are "statement-expressions": they mutate the environment
  and *also* return a value.  This is not the same behavior as in Scheme, but it
  makes slang easier to implement.
