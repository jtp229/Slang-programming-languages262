# CSE 262: Scanning "slang" in Python

This is the Python part of the scanning assignment.
Your job is to finish `slang_scanner.py`.
To do this, you'll need to fill in the `transitions` table and write `scan_tokens`.
Your scanner **must** be table-driven.
That is, `scan_tokens` should be iterating through an array of `Transition` objects, finding the right one, and doing whatever it says to do.
You should not have a bunch of per-state functions.

## Running

```bash
python3 slang.py -scan path/to/file.scm
```

## Testing

From the `p1/` directory (one level up from here), `test.sh` and `compare.sh` run your scanner against the files in `tests/`:

```bash
cd ..
./test.sh python
./compare.sh python
```

## Suggestions

- Make sure you understand the `Transition` type.
  The goal here is to have a declarative scanner.
  You should be able to extend the grammar and this scanner should only need one new line of code (a new `Transition` in the table).
- Use helper functions sparingly.
  A few will make sense.
  But not many.
- The whole point of `Transition` and the `transitions` list is that `scan_tokens` should be a short, loop that just goes through the table.
  If you find yourself writing `if`/`elif` chains on scanner state inside `scan_tokens`, you're not doing it correctly.
- Be sure to understand the `maker` functions.
  They encapsulate the per-state logic for making tokens.
* Order matters in the transitions table.
  `scan_tokens` should use the first matching transition for the current state, so put more specific rules (matching a small `peek_set`) before catch-all rules (an empty or inverted `peek_set`) for the same `from_state`.
