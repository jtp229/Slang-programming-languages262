package cse262.slang.Scanner;

import java.util.List;

import cse262.slang.Parser.Parser;
import cse262.slang.Parser.Parser.ParseError;

/**
 * TokenStream is a wrapper around a list of tokens. The reason for the wrapper
 * is that our Parser can't just iterate through the tokens, because many of the
 * grammar's productions require a little bit of look-ahead (one or two tokens).
 * TokenStream provides an API that is easy enough to iterate through, but which
 * is also friendly to the requirements of Parser.
 */
public class TokenStream {
    /**
     * A collection of tokens, in the order they were observed in the source
     * file
     */
    private final List<Tokens.Token> tokens;

    /**
     * A lightweight iterator mechanism: this int just tells which entry in
     * tokens to return next.
     */
    private int next = 0;

    /**
     * Construct a TokenStream from a list of tokens
     *
     * @param tokens The tokens to use to make the stream
     */
    public TokenStream(List<Tokens.Token> tokens) {
        this.tokens = tokens;
    }

    /** Reset the iterator, so that we can iterate through the tokens */
    public void reset() {
        this.next = 0;
    }

    /**
     * Get the next token, via the iterator-like interface
     *
     * @return The next token in the stream
     */
    public Tokens.Token nextToken() {
        return (!hasNext()) ? null : tokens.get(next);
    }

    /**
     * Get the token after the next token, via the iterator-like interface
     *
     * @return The token after the next token in the stream
     */
    public Tokens.Token nextNextToken() {
        return (!hasNextNext()) ? null : tokens.get(next + 1);
    }

    /**
     * Remove the next token, without checking it
     */
    public void popAny() {
        ++next;
    }

    /**
     * Remove the next token, but only if it matched the expected type
     *
     * @param expected The expected type of the token to remove
     * @throws ParseError If the next token does not match the expected type
     */
    public void popToken(Class<?> expected) throws ParseError {
        if (!hasNext() || !expected.isInstance(tokens.get(next)))
            throw new Parser.ParseError(this);
        ++next;
    }

    /**
     * Report if there is more for the iterator to consume
     *
     * @return true if there is more for the iterator to consume, false otherwise
     */
    public boolean hasNext() {
        return next < tokens.size();
    }

    /**
     * Report if there is even more for the iterator to consume
     * 
     * @return true if there is even more for the iterator to consume, false
     *         otherwise
     */
    public boolean hasNextNext() {
        return (next + 1) < tokens.size();
    }
}
