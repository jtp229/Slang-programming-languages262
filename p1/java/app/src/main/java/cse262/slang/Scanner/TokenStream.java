package cse262.slang.Scanner;

import java.util.List;

/**
 * TokenStream is a wrapper around a list of tokens, with a simple
 * iterator-like API for consuming them one at a time.
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

    /**
     * Get the next token, via the iterator-like interface
     *
     * @return The next token in the stream
     */
    public Tokens.Token nextToken() {
        return (!hasNext()) ? null : tokens.get(next);
    }

    /**
     * Remove the next token, without checking it
     */
    public void popAny() {
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
}
