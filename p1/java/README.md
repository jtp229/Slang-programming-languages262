# CSE 262: Scanning "slang" in Java

This is the Java part of the scanning assignment
Your job is to finish `app/src/main/java/cse262/slang/Scanner/Scanner.java`.
To do this, you'll need to implement a bunch of functions for keeping track of the scanner state and deciding what to do.
Note that your code should be $O(n)$, i.e., linear in the input length.

## Building and Running

```bash
./gradlew build
java -jar app/build/libs/app.jar -scan path/to/file.scm
```

## Testing

From the `p1/` directory (one level up from here), `test.sh` and `compare.sh` run your scanner against the files in `tests/`:

```bash
cd ..
./test.sh java
./compare.sh java
```

## Suggestions

- You shouldn't have any mutual recursion in your solution; everything should return to the main public function in `Scanner.java`.
- You'll probably want to structure your code with one method per scanner state
- Be sure to understand the FSM that is presented in `docs/Scanning.md`.
  Keeping your code aligned with those diagrams will make it much easier to debug.
* Line/column tracking is tricky for multi-character tokens.
  Don't put off strings and comments for too long.
* Test combinations, not just individual tokens.
  Bugs tend to hide at the boundaries between two token types.
  You can get full credit by handing the given test cases, but it's good to think about inputs beyond those in the tests.
