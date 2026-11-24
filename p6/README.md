# CSE 262: Programming Assignment 6: Memory Management for "slang"

The semester-long project in CSE 262 is to build a full interpreter for slang, a Scheme-like language.
This assignment focuses on run-time systems.
Until now, you've mostly been implementing slang in languages that are garbage collected.
As a result, your code up until now has been able to allocate objects using the underlying system's allocator, and then rely on the underlying language's garbage collector to do the work.
The only exception to this was your Rust code, which used `Rc` to reference count allocated objects.

In this assignment, you are provided with two working slang interpreters, written in C++ and C.
They are both functionally correct, but they do not free any memory.
Your task is to contrast two garbage collection strategies:

- In C++, you will convert the interpreter to use **reference counting**, via `std::shared_ptr`
- In C, you will write a **mark-sweep garbage collector**

## Project Details

Both interpreters read a file containing an XML AST, in the same format as in p3-p5, and evaluate each top-level expression in order, printing its value (or its error).
They support only the `-interpret` mode (and `-help`).

To keep the code small, these interpreters implement less of slang than you did in p3 and p4:

- There is no tail-call optimization, so deep recursion **will** crash the interpreter
- There is no `apply`; the special forms are `define`, `lambda`, `if`, `cond`, `and`, `or`, `begin`, `let`, `set!`, and `'`
- The built-in functions are:
  - Numbers: `+`, `-`, `*`, `/`, `%`, `=`, `<`, `<=`, `>`, `>=`, `abs`
  - Lists: `cons`, `car`, `cdr`, `list`, `list?`, `null?`, `set-car!`, `set-cdr!`
  - Strings: `string-append`, `string-length`, `substring`, `string?`
  - Vectors: `vector`, `vector-length`, `vector-get`, `vector-set!`, `vector?`
  - Other: `not`, `display`

You should not need to change what the interpreters *do*.
After your changes, every test should produce exactly the same output as before.
What changes is *when memory is reclaimed*.

## Part 1: Reference Counting in C++

The starter code is in the `cc` folder.
Every `Value`, `AstNode`, and `Env` is created with `new`, held through raw pointers, and never deleted.
You must convert the interpreter to use `std::shared_ptr` (and `std::make_shared`, `std::static_pointer_cast`, etc.), so that each object is freed as soon as the last reference to it goes away.
This will be quite a bit like `Rc` in Rust.
You should not need to call `delete` anywhere.

Reference counting can't reclaim everything.
After your conversion, some of the tests will still leak memory, and that is expected.
You should expect a question about this on the 6th quiz.

## Part 2: A Mark-Sweep Garbage Collector in C

The starter code is in the `c` folder.
Every `struct Value`, `struct AstNode`, and `struct Env` is allocated with `gc_alloc`, but nothing is ever freed.
You must implement a mark-sweep garbage collector, so that memory that the program can no longer reach is reclaimed **while the program is running**, and so that everything is freed by the time the program exits.

The interface to the collector is in `c/gc.h`, and `c/gc.c` has a stub for each of its functions.
Right now, `gc_alloc` just calls `calloc`, and the other functions do nothing.
The interface suggests how the pieces of the collector fit together:

- **Allocation:** `gc_alloc` should allocate room for a `GcHeader` followed by the object, link the header into a list of all objects, and return a pointer to the object (just past the header).
  This is also where you should decide whether it's time to collect (see "When to collect", below).
- **Roots:** `gc_push_roots` and `gc_pop_roots` maintain the "shadow stack", which is a linked list of `GcRootFrame`s.
  Each frame lives on the C stack, in the function that pushes it, and holds the addresses of that function's local variables that need protection.
  `gc_register_env` and `gc_register_forest` record the two other roots: the global environment and the AST.
  `gc_suspend` and `gc_resume` let you turn off collection while building objects that can't be registered yet (the global environment and the AST).
- **Marking:** `gc_collect` should first mark everything that is reachable from the roots.
  You'll want a mark function for each kind of object, which sets the object's `marked` bit and then marks every object it points to.
  For example, a cons cell points to its car and cdr, a closure points to its environment and to the AST node for its `lambda`, and an environment points to the values it binds and to its outer environment.
- **Sweeping:** Then `gc_collect` should walk the list of all objects, free each one that isn't marked, and clear the mark bit of each one that is.
  Some objects own memory that isn't part of the collector: a string's characters, a vector's array of items, or an environment's bindings.
  When the sweep frees such an object, it must free that memory too.

The hardest part is not in `gc.c`: it's putting root frames in the right places in the rest of the interpreter.
A `struct Value *` that is only stored in a local variable of a C function is in use, but the collector has no way to know about it unless you tell it.
Look at every function that allocates (directly or by calling `evaluate`), and ask which of its local variables point to objects that aren't reachable from any other root.
For example, `evaluate` needs to protect its `env`, a call needs to protect the function and its arguments while it evaluates other arguments, and a call or `let` needs to protect the new environment that it makes.
Don't forget to register the global environment and the AST in `slang.c`, and to unregister them and collect one last time at the end, so that everything is freed.

A collection can happen in the middle of evaluating an expression.
Most garbage collector bugs are "a collection happened at a moment when something that was in use was not reachable from a root."
These bugs only show up when a collection happens at exactly the wrong time, so while you are debugging, it is a very good idea to collect *very* often (e.g., on every allocation).

### When to collect

The simplest policy is to collect after every *N* allocations.
That is a good policy while debugging, but it is a bad one for real programs.
Each collection must mark every live object, so if the program keeps a lot of data alive (for example, a list with a million elements), every collection does a million units of work, even if only a few new objects were allocated since the last one.
With a small *N*, the program spends almost all of its time marking the same live objects over and over.

A better policy is to make the interval **adaptive**: after each collection, count how many objects survived, and do not collect again until the program has allocated about that many more objects (with some minimum, like 1024).
With that policy, the work of marking is proportional to the amount of allocation, no matter how much data the program keeps alive.
Your collector should use an adaptive policy (it doesn't have to be exactly this one).
A fixed interval that is small enough to catch bugs quickly will make any program that keeps a lot of data alive very slow.

## Testing

From this folder, run `./test.sh <lang> <mode>` (where `<lang>` is `c` or `cc`).

- `./test.sh <lang> interpret` runs the interpreter on every test in `tests/interpret/inputs`, and saves the output in `results/interpret/<lang>/`.
  Then run `./compare.sh <lang> interpret` to see which results differ from the expected outputs.
- `./test.sh <lang> valgrind` runs the same tests under `valgrind --leak-check=full`, saves each report in `results/valgrind/<lang>/`, and lists every test whose report isn't clean (no errors, and every heap block freed).
- `./test.sh <lang> memory` runs the tests in `tests/memory/inputs`, which allocate a lot of garbage but keep very little of it alive.
  It prints each test's peak memory use, and checks that it is within 64 MB.
  Then run `./compare.sh <lang> memory` to check the outputs.

Before you change anything, every `interpret` test passes, every `valgrind` report shows leaks, and every `memory` test is far over the limit.
When you are done, every test should pass in all three modes, with these exceptions:

- `deep_list` is expected to crash after your changes.
  You do not need to fix it.
- In C++, the tests that leak because of reference counting's limitations (see Part 1) will not be clean under valgrind.
- In C++, `churn_cycles` is not checked against the memory limit, for the same reason.

A test that passes `valgrind` but fails `memory` probably means that you only free memory when the program exits.

### Using The Sunlab Machines

The Sunlab machines need a newer compiler than the default one.
Before building, run `module load gcc-15.2.0`.
`valgrind` and the `tinyxml2` library (used by the C++ version) are already installed.

## Tips and Reminders

**Start Early**!
Read through the code before you change it.
In particular, find every place that a `Value`, `AstNode`, or `Env` is created, and every place that one is stored.

Commit your work often, and run the tests often.
A garbage collector bug can make a test that passed an hour ago start crashing.

Please be careful about not committing unnecessary files into your repository.
In particular, do not commit `c/obj64`, `cc/obj64`, or `results`.

## Grading

Each of the two parts of the assignment will be worth 50% of your grade.
For each part, I will check that the outputs are correct, that the `valgrind` reports are clean, and that the `memory` tests stay within the limit (other than the exceptions listed under Testing).

You should be sure to comment your code.
Comments will not be graded extensively, but if they are not present, or are not appropriate for the assignment, I may deduct points.

## Collaboration and Getting Help

Students may work in teams of 2 for this assignment.
If you plan to work in a team, you must notify me by Nov 30, so that I can set up your repository access.
If you are working in a team, you should **pair program** for the entire assignment.

If you require help, you may seek it from any of the following sources:

- The professor and TAs, via office hours or Piazza
- The Internet, as long as you use the Internet as a read-only resource and do not post requests for help to online sites.

It is not appropriate to share code with past or current CSE 262 students, unless you are sharing code with your teammate.

As always, you should be using AI responsibly.
Remember: Your goal is to learn.

## Deadline

You should be done with this assignment before 11:59 PM on December 6th.
Please be sure to `git commit` and `git push` before that time, so that I can promptly collect and grade your work.
