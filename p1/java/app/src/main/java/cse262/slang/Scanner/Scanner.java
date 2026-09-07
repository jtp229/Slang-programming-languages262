package cse262.slang.Scanner;

/**
 * Scanner is responsible for taking a string that is the source code of a
 * program, and transforming it into a stream of tokens.
 *
 * It is tempting to think "if my code doesn't crash when I give it good input,
 * then I have done a good job". However, a good scanner needs to be able to
 * handle incorrect programs. The bare minimum is that the scanner should not
 * crash if the input is invalid. Even better is if the scanner can print a
 * useful diagnostic message about the point in the source code that was
 * incorrect. Best, of course, is if the scanner can somehow "recover" and keep
 * on scanning, so that it can report additional syntax errors.
 *
 * **In this class, "even better" is good enough for full credit**
 *
 * [262] This is the only java file that you need to edit in this assignment.
 *
 * You are allowed to add private methods and fields to this class. You may also
 * add imports.
 */
public class Scanner {
    /** The index of the first character of the in-progress token */
    private int start = 0;

    /** Count the newlines we consume */
    private int current_line = 1;

    /** index in `source` where current line begins */
    private int line_start_char = 0;

    /**
     * Compute the column number where the current token starts
     *
     * @return The 1-indexed column number
     */
    private int col() {
        return start - line_start_char + 1;
    }

    /**
     * An exception to capture the situation where the scanner encounters an
     * error
     */
    public class ScanError extends Exception {
        /** The line number where the error arose */
        public int line;
        /** The column number where the column arose */
        public int col;

        /**
         * Construct a ScanError from a message and a location
         *
         * @param message The error message
         */
        public ScanError(String message) {
            super(String.format("Scan Error: line %d, col %d", current_line, col()));
            line = current_line;
            col = col();
        }
    }

    /**
     * scanTokens works through the source and transforms it into a list of
     * tokens. It adds an EOF token at the end, unless there is an error.
     *
     * @param source The source code to scan
     *
     * @return The token stream
     */
    public TokenStream scanTokens(String source) throws ScanError {
        throw new ScanError("scanTokens not implemented");
    }
}