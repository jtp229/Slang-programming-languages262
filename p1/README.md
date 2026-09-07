# CSE 262: Programming Assignment 1: Scanning "slang"

The semester-long project in CSE 262 is to build a full interpreter for a relatively complete subset of the Scheme programming language.
To avoid confusion, we will call our language "slang" (because it is a Scheme-like LANGuage).

Slang will not be a proper subset of Scheme: as we move through the semester, some features will be defined differently in slang than in Scheme, so that it is easier to implement slang.
And, of course, some challenging features of Scheme won't be part of our language.
Our goal is to be Turing complete and good enough to be something you can put on a resume, but not a production-quality interpreter.

In the first phase of the assignment, we will only worry about *scanning* slang.
That is, given a string that purports to be slang code, we will try to turn it into a sequence of tokens.

Implementing a scanner for slang will provide an opportunity to learn more about Scheme.
Implementing it twice, in two different languages, will provide an opportunity to compare and contrast different programming languages.
We'll also write some Scheme code as part of this assignment, in order to get a deeper understanding of functional programming and Scheme syntax.

## Project Details

Scanning is the first step in any compiler or interpreter.
It is the step that turns source code into "tokens" that are easier to work with throughout the rest of the compiler/interpreter.
Since the scanner in this assignment is not connected to a parser, it will simply output the tokens.
Code for this output is provided.

As you work on this assignment, you should try to write *idiomatic* code.
Try, as much as possible, to use language features that make your code more readable, more familiar to experts, and more succinct and expressive.

## What To Expect

The Java folder has the most starter code, and is probably the most approachable part of the assignment.
It isn't "easy" to implement a scanner for a regular language for the first time, but it isn't incredibly difficult either.
With just a small amount of lookahead, you should be able to correctly recognize all of the scheme syntax, and produce tokens.
Note that you should only need to work on a single file: `Scanner.java`.
As you do, you should try to solve this in a way that is familiar to a student who took CSE 017.
In particular, this means you should probably have a separate method for processing each scanner state.

After you've made progress on your Java code, move on to Python.
Your implementation in this language should be *very different* from your Java implementation.
It should use a *table-driven* scanner.
A constructor for a `transition` object is provided, which should guide you.
Note that a table-driven scanner is much more maintainable, but it requires you to think in a more functional and declarative way.

There is also a testing folder, which includes all of the tests that I will run against your scanners.

The Scheme folder has a set of small problems that will encourage you to get more familiar with Scheme syntax, and with some of the advanced behaviors that we will need to implement in later phases of the assignment.

## Tips and Reminders

**Start Early**.
Just reading the code and understanding what is happening takes time.
If you start reading the assignment early, you'll give yourself time to think about what is supposed to be happening, and that will help you to figure out what you will need to do.

When it comes time to implement your scanner, you will probably want to do the Java code first.
It is up to you whether to try to implement a full scanner first, and then test it, or to implement incrementally (for example, starting with numbers, then characters, then expressions).
Strings and characters are probably the most tricky.

As always, please be careful about not committing unnecessary files into your repository.

## Grading

The Java code will be worth 40% of your grade.
The Python code will be worth 40% of your grade.
The Scheme code will be worth 20% of your grade.

You should be sure to comment your code.
For the scanners, comments will not be graded extensively, only if their absence is egregious.
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
If you plan to work in a team, you must notify me by Sep 11th, so that I can set up your repository access.
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
I also don't think there's anything truly "boilerplate" in this assignment, so I don't think there are likely to be good uses of GenAI for this assignent.

## Deadline

You should be done with this assignment before 11:59 PM on Sep 18th, 2026.
Please be sure to `git commit` and `git push` before that time, so that I can promptly collect and grade your work.

There are many parts to this assignment, so you will probably want to `git push` frequently.
