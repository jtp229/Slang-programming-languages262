# Scheme Exercises

The purpose of this part of the assignment is to get you thinking about standard library implementations in more detail.
One could argue that the parts of the standard library that you implemented in TypeScript *can not* be implemented any other way.
For example, it does not seem possible to implement `+` using just special forms.
However, at some point, the minimal, built-in parts of the standard library become robust enough that you can implement the rest of the standard library in slang (well, Scheme).

To that end, in this folder, you will find six incomplete Scheme files.
Each has the shell of one or more functions that a good slang implementation should provide.
But we're not going to implement them in slang.
We're going to implement them in Scheme.

Read the comment at the top of each file: it says what the functions should do, and which built-in functions you are not allowed to use.

These exercises make use of a small compatibility layer that makes gsi's Scheme look like slang:
`slang_compat.scm` lets `gsi` understand some names from the slang standard library that Scheme spells differently (`loge`, `pi`, `inf+`, `nan?`, `double->integer`, ...).
Each of the files I provided loads it for you.

Be sure to comment your code, so that I understand why you solved each part the way you did.
