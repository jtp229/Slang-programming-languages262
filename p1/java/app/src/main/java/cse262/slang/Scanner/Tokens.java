package cse262.slang.Scanner;

import java.util.HashMap;

/**
 * Tokens is a wrapper class. The high-level goal here is to set up 23 different
 * classes, to represent the 23 different tokens that our scanner needs to
 * understand. These tokens share some common code, but are very uninteresting:
 * they are just POJOs (plain old Java objects) consisting of a few final
 * fields.
 *
 * Rather than create 23 separate Java files, Tokens declares these 23 classes
 * (as well as their base class) as nested classes. These nested classes are
 * `static`, because we do not need a reference to the `Tokens` class in order
 * to make them.
 *
 * If you don't like this kind of code, that's OK. It's ugly. It's a hack. It's
 * just organized like this so that we don't need 24 files of boilerplate code.
 * Better yet, you don't need to edit this file, so as long as you can
 * understand what's going on here, you're good.
 */
public class Tokens {
    public static HashMap<Class<?>, String> tokenNames = new HashMap<>() {
        {
            put(Abbrev.class, "ABBREV");
            put(And.class, "AND");
            put(Apply.class, "APPLY");
            put(Begin.class, "BEGIN");
            put(Bool.class, "BOOL");
            put(Char.class, "CHAR");
            put(Cond.class, "COND");
            put(Dbl.class, "DBL");
            put(Define.class, "DEFINE");
            put(Dot.class, "DOT");
            put(Eof.class, "EOF");
            put(Identifier.class, "IDENTIFIER");
            put(If.class, "IF");
            put(Int.class, "INT");
            put(Lambda.class, "LAMBDA");
            put(LeftParen.class, "LPAREN");
            put(Let.class, "LET");
            put(Or.class, "OR");
            put(Quote.class, "QUOTE");
            put(RightParen.class, "RPAREN");
            put(Set.class, "SET");
            put(Str.class, "STR");
            put(Vec.class, "VEC");
        }
    };

    /**
     * Token is the base class for all of the tokens that are
     * understood/recognized by our scanner. Token itself is abstract, because
     * we don't ever want to make instances of it. However, it is not just an
     * interface, because it defines common state that is used by all of its
     * descendants.
     */
    public static abstract class Token {
        /** The source program characters that led to this token being made */
        public final String tokenText;

        /** The line within the source code where `tokenText` appears */
        public final int line;

        /** The column within `line` where `tokenText` appears */
        public final int col;

        /**
         * Construct a Token
         *
         * @param tokenText The string that generated the token
         * @param line      The source line of code where tokenText appears
         * @param col       The column within the source line of code
         */
        public Token(String tokenText, int line, int col) {
            this.tokenText = tokenText;
            this.line = line;
            this.col = col;
        }
    }

    /** A token for `'` (quote shorthand) */
    public static class Abbrev extends Token {
        /**
         * Construct by forwarding to the Token constructor
         *
         * @param tokenText The string that generated the token
         * @param line      The source line of code where tokenText appears
         * @param col       The column within the source line of code
         */
        public Abbrev(String tokenText, int line, int col) {
            super(tokenText, line, col);
        }
    }

    /** A token for `and` (keyword / special form) */
    public static class And extends Token {
        /**
         * Construct by forwarding to the Token constructor
         *
         * @param tokenText The string that generated the token
         * @param line      The source line of code where tokenText appears
         * @param col       The column within the source line of code
         */
        public And(String tokenText, int line, int col) {
            super(tokenText, line, col);
        }
    }

    /** A token for `apply` (keyword / special form) */
    public static class Apply extends Token {
        /**
         * Construct by forwarding to the Token constructor
         *
         * @param tokenText The string that generated the token
         * @param line      The source line of code where tokenText appears
         * @param col       The column within the source line of code
         */
        public Apply(String tokenText, int line, int col) {
            super(tokenText, line, col);
        }
    }

    /** A token for `begin` (keyword / special form) */
    public static class Begin extends Token {
        /**
         * Construct by forwarding to the Token constructor
         *
         * @param tokenText The string that generated the token
         * @param line      The source line of code where tokenText appears
         * @param col       The column within the source line of code
         */
        public Begin(String tokenText, int line, int col) {
            super(tokenText, line, col);
        }
    }

    /** A token for a boolean value (`#t` or `#f`) */
    public static class Bool extends Token {
        /** The logical value of this token */
        public final boolean value;

        /**
         * Construct by saving the value and forwarding to the Token
         * constructor
         *
         * @param tokenText The string that generated the token
         * @param line      The source line of code where tokenText appears
         * @param col       The column within the source line of code
         * @param value     The boolean value of the token
         */
        public Bool(String tokenText, int line, int col, boolean value) {
            super(tokenText, line, col);
            this.value = value;
        }
    }

    /** A token for a character value */
    public static class Char extends Token {
        /** The character value of this token */
        public final char value;

        /**
         * Construct by saving the value and forwarding to the Token
         * constructor
         *
         * @param tokenText The string that generated the token
         * @param line      The source line of code where tokenText appears
         * @param col       The column within the source line of code
         * @param value     The character value of the token
         */
        public Char(String tokenText, int line, int col, char value) {
            super(tokenText, line, col);
            this.value = value;
        }
    }

    /** A token for `cond` (keyword / special form) */
    public static class Cond extends Token {
        /**
         * Construct by forwarding to the Token constructor
         *
         * @param tokenText The string that generated the token
         * @param line      The source line of code where tokenText appears
         * @param col       The column within the source line of code
         */
        public Cond(String tokenText, int line, int col) {
            super(tokenText, line, col);
        }
    }

    /** A token for a double (number) value */
    public static class Dbl extends Token {
        /** The double value of this token */
        public final double value;

        /**
         * Construct by saving the value and forwarding to the Token
         * constructor
         *
         * @param tokenText The string that generated the token
         * @param line      The source line of code where tokenText appears
         * @param col       The column within the source line of code
         * @param value     The double value of the token
         */
        public Dbl(String tokenText, int line, int col, double value) {
            super(tokenText, line, col);
            this.value = value;
        }
    }

    /** A token for `define` (keyword / special form) */
    public static class Define extends Token {
        /**
         * Construct by forwarding to the Token constructor
         *
         * @param tokenText The string that generated the token
         * @param line      The source line of code where tokenText appears
         * @param col       The column within the source line of code
         */
        public Define(String tokenText, int line, int col) {
            super(tokenText, line, col);
        }
    }

    /** A token for `.` (cons construction) */
    public static class Dot extends Token {
        /**
         * Construct by forwarding to the Token constructor
         *
         * @param tokenText The string that generated the token
         * @param line      The source line of code where tokenText appears
         * @param col       The column within the source line of code
         */
        public Dot(String tokenText, int line, int col) {
            super(tokenText, line, col);
        }
    }

    /** A token to represent end-of-file / end-of-input */
    public static class Eof extends Token {
        /**
         * Construct by forwarding to the Token constructor
         *
         * @param tokenText The string that generated the token
         * @param line      The source line of code where tokenText appears
         * @param col       The column within the source line of code
         */
        public Eof(String tokenText, int line, int col) {
            super(tokenText, line, col);
        }
    }

    /**
     * A token for any identifier that isn't a value or keyword / special form
     * keyword. Note that for Identifiers, we don't bother to save the value.
     * The tokenText is good enough.
     */
    public static class Identifier extends Token {
        /**
         * Construct by forwarding to the Token constructor
         *
         * @param tokenText The string that generated the token
         * @param line      The source line of code where tokenText appears
         * @param col       The column within the source line of code
         */
        public Identifier(String tokenText, int line, int col) {
            super(tokenText, line, col);
        }
    }

    /** A token for `if` (keyword / special form) */
    public static class If extends Token {
        /**
         * Construct by forwarding to the Token constructor
         *
         * @param tokenText The string that generated the token
         * @param line      The source line of code where tokenText appears
         * @param col       The column within the source line of code
         */
        public If(String tokenText, int line, int col) {
            super(tokenText, line, col);
        }
    }

    /** A token for an integer (number) value */
    public static class Int extends Token {
        /** The integer value of this token */
        public final int value;

        /**
         * Construct by saving the value and forwarding to the Token
         * constructor
         *
         * @param tokenText The string that generated the token
         * @param line      The source line of code where tokenText appears
         * @param col       The column within the source line of code
         * @param value     The integer value of the token
         */
        public Int(String tokenText, int line, int col, int value) {
            super(tokenText, line, col);
            this.value = value;
        }
    }

    /** A token for `lambda` (keyword / special form) */
    public static class Lambda extends Token {
        /**
         * Construct by forwarding to the Token constructor
         *
         * @param tokenText The string that generated the token
         * @param line      The source line of code where tokenText appears
         * @param col       The column within the source line of code
         */
        public Lambda(String tokenText, int line, int col) {
            super(tokenText, line, col);
        }
    }

    /** A token for `(` (start of list/expression) */
    public static class LeftParen extends Token {
        /**
         * Construct by forwarding to the Token constructor
         *
         * @param tokenText The string that generated the token
         * @param line      The source line of code where tokenText appears
         * @param col       The column within the source line of code
         */
        public LeftParen(String tokenText, int line, int col) {
            super(tokenText, line, col);
        }
    }

    /** A token for `let` (keyword / special form) */
    public static class Let extends Token {
        /**
         * Construct by forwarding to the Token constructor
         *
         * @param tokenText The string that generated the token
         * @param line      The source line of code where tokenText appears
         * @param col       The column within the source line of code
         */
        public Let(String tokenText, int line, int col) {
            super(tokenText, line, col);
        }
    }

    /** A token for `or` (keyword / special form) */
    public static class Or extends Token {
        /**
         * Construct by forwarding to the Token constructor
         *
         * @param tokenText The string that generated the token
         * @param line      The source line of code where tokenText appears
         * @param col       The column within the source line of code
         */
        public Or(String tokenText, int line, int col) {
            super(tokenText, line, col);
        }
    }

    /** A token for `quote` (keyword / special form) */
    public static class Quote extends Token {
        /**
         * Construct by forwarding to the Token constructor
         *
         * @param tokenText The string that generated the token
         * @param line      The source line of code where tokenText appears
         * @param col       The column within the source line of code
         */
        public Quote(String tokenText, int line, int col) {
            super(tokenText, line, col);
        }
    }

    /** A token for `)` (end of vector/list/expression) */
    public static class RightParen extends Token {
        /**
         * Construct by forwarding to the Token constructor
         *
         * @param tokenText The string that generated the token
         * @param line      The source line of code where tokenText appears
         * @param col       The column within the source line of code
         */
        public RightParen(String tokenText, int line, int col) {
            super(tokenText, line, col);
        }
    }

    /** A token for `set!` (keyword / special form) */
    public static class Set extends Token {
        /**
         * Construct by forwarding to the Token constructor
         *
         * @param tokenText The string that generated the token
         * @param line      The source line of code where tokenText appears
         * @param col       The column within the source line of code
         */
        public Set(String tokenText, int line, int col) {
            super(tokenText, line, col);
        }
    }

    /** A token for a string (text) value */
    public static class Str extends Token {
        /** The string value of this token */
        public final String value;

        /**
         * Construct by saving the value and forwarding to the Token
         * constructor
         *
         * @param tokenText The string that generated the token
         * @param line      The source line of code where tokenText appears
         * @param col       The column within the source line of code
         * @param value     The string value of the token
         */
        public Str(String tokenText, int line, int col, String value) {
            super(tokenText, line, col);
            this.value = value;
        }
    }

    /** A token for `#(` (start of vector) */
    public static class Vec extends Token {
        /**
         * Construct by forwarding to the Token constructor
         *
         * @param tokenText The string that generated the token
         * @param line      The source line of code where tokenText appears
         * @param col       The column within the source line of code
         */
        public Vec(String tokenText, int line, int col) {
            super(tokenText, line, col);
        }
    }
}