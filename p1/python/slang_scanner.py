
# We'll use this as a simple approximation of an enum for the different Token
# types
(
    TOK_ABBREV,
    TOK_AND,
    TOK_APPLY,
    TOK_BEGIN,
    TOK_BOOL,
    TOK_CHAR,
    TOK_COND,
    TOK_DBL,
    TOK_DEFINE,
    TOK_DOT,
    TOK_EOF,
    TOK_IDENTIFIER,
    TOK_IF,
    TOK_INT,
    TOK_LAMBDA,
    TOK_LEFT_PAREN,
    TOK_LET,
    TOK_OR,
    TOK_QUOTE,
    TOK_RIGHT_PAREN,
    TOK_SET,
    TOK_STR,
    TOK_VECTOR,
    TOK_ERROR,
) = range(24)


class Token:
    """A single scanned entity in the source code."""

    def __init__(self, text, line, col, tok_type, lit):
        """Create a Token object.

        Args:
            text: The source program characters that led to this token being
                made.
            line: The line within the source code where `text` appears.
            col: The column within `line` where `text` appears.
            tok_type: The type of the token.
            lit: The literal (bool, int, number, str, or None).
        """
        self.text = text
        self.line = line
        self.col = col
        self.tok_type = tok_type
        self.literal = lit


class ScanError(Exception):
    """Raised by a maker function when its text isn't a valid literal."""


class Transition:
    """A transition within the scanner.

    We transition from `from_state` to `to_state` only if:
    - the next character is in `peek_set`; or
    - the next character is not in `peek_set` and `invert_peek` is true; or
    - `peek_set` is undefined

    Note that it is possible to consume without producing a Token (e.g., to
    clear out a comment)
    """

    def __init__(
        self, from_state, to_state, peek_set, invert_peek, consume, advance, maker
    ):
        """Create a Transition object.

        Args:
            from_state: The state we must be in for this rule to be valid.
            to_state: The state to transition to.
            peek_set: Values to check against the next character.
            invert_peek: Is peek the set of invalid characters?
            consume: Should this transition consume a token string.
            advance: Should the transition cause the next peek() to see a
                new character?
            maker: A function that explains how to produce a value and
                token type.
        """
        self.from_state = from_state
        self.to_state = to_state
        self.peek_set = peek_set
        self.invert_peek = invert_peek
        self.consume = consume
        self.advance = advance
        self.maker = maker

#SCANNER STATES

#scanner states
STATE_START = 0
STATE_IDENTIFIER = 1
STATE_NUMBER = 2
STATE_STRING = 3
STATE_COMMENT = 4
STATE_HASH = 5
STATE_DECIMAL = 6
STATE_CHAR = 7
STATE_PLUS = 8
STATE_MINUS = 9
STATE_DOT = 10
STATE_STRING_ESCAPE = 11


#define character sets for transitions
DELIMITERS = set(" ()\";\r\n\t\0")
DIGITS = set("0123456789")
LETTERS = set("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
SPECIAL_CHARS = set("!$%&*+-./:<=>?^_~")
IDENTITY_CHARS = LETTERS | DIGITS | SPECIAL_CHARS


#define maker functinos for transitions

def make_identifier():

def  make_int():

def make_dbl():

def make_str():

def make_char():

def make_simple():


# All of the transitions in our scanner.  Order matters.
transitions = []


def scan_tokens(source):
    """Work through `source` and transform it into a list of tokens.

    This will always put an EOF token at the end, unless there is an error.
    If there is an error, then it returns an array with exactly one entry.

    Args:
        source: The source code of the program or a line of REPL input.

    Returns:
        A list of Tokens.
    """

    raise ScanError("scan_tokens not implemented")
