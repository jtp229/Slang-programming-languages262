package cse262.slang.Parser;

import java.util.List;

import cse262.slang.Scanner.TokenStream;

/**
 * Parser is the second step in our interpreter. It is responsible for turning a
 * sequence of tokens into an abstract syntax tree.
 *
 * Parse errors are reported via the checked ParseError exception, declared on
 * every method that can fail.
 *
 * [CSE 262] This is the only java file that you need to edit in this assignment.
 *
 * The reference solution has ~329 lines of code (plus 38 blank lines and ~245
 * lines of comments). Your code may be longer or shorter... the
 * line-of-code count is just a reference.
 *
 * You are allowed to add private methods and fields to this class. You may also
 * add imports.
 */
public class Parser {
    /**
     * An exception class for indicating that the parser encountered an error
     */
    public static class ParseError extends Exception {
        /**
         * Construct a ParseError and attach a message to it
         *
         * @param msg The message to attach to the exception
         */
        public ParseError(String msg) {
            super(msg);
        }

        /**
         * Construct a ParseError using the current stream to get line/column
         * numbers
         *
         * @param stream The token stream to extract the line and column numbers from
         */
        public ParseError(TokenStream stream) {
            super(formatError(stream));
        }

        /**
         * Create an error message with the current line and column
         *
         * @param stream The token stream, for extracting the offending token
         *
         * @return An error message
         */
        private static String formatError(TokenStream stream) {
            if (stream.hasNext()) {
                var t = stream.nextToken();
                return String.format("Parse Error: line %d, col %d", t.line, t.col);
            }
            return "Parse Error: EOF";
        }
    }

    /**
     * Transform a stream of tokens into a forest of AstNodes. It is assumed
     * that the TokenStream has an extra EOF at the end. This is really just a
     * single transition: <program> --> <expression>*
     *
     * @param tokens a stream of tokens
     *
     * @return A list of AstNodes
     */
    public List<AstNodes.AstNode> parse(TokenStream tokens) throws ParseError {
        throw new ParseError("parse not implemented");
    }
}
