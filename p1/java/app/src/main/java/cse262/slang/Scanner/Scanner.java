package cse262.slang.Scanner;
import java.util.List;
import java.util.ArrayList;
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
    private String source; // the source code to scan

    private int current = 0; // index of current character in the source string

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
    private boolean isAtEnd(String source) {
        if (current >= source.length()) { //if current index is greater then or equal to length of string, were at the the end of the string
            return true;
        }
        return false;
    }
    private char advance(String source) {
        char c = source.charAt(current);  //save the current character
        current++; //increment the index to move forward parsing
        return c;
    }

    private char peek(String source) { //see next character without consuming it
        if (isAtEnd(source)) {
            return '\0'; //return null character if at the end of the string
        }
        return source.charAt(current); //return the current character
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
        this.source = source;
        this.current = 0;
        this.start = 0;
        this.current_line = 1;
        this.line_start_char = 0;

        List<Tokens.Token> tokens = new ArrayList<>(); //create a new list of tokens to hold the scanned tokens


        throw new ScanError("scanTokens not implemented");
    }
}