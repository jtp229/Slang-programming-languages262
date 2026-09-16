
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
KEYWORDS = {
    "and": TOK_AND,
    "begin": TOK_BEGIN,
    "cond": TOK_COND,
    "define": TOK_DEFINE,
    "if": TOK_IF,
    "lambda": TOK_LAMBDA,
    "or": TOK_OR,
    "quote": TOK_QUOTE,
    "set!": TOK_SET,
    "let": TOK_LET,
    "apply": TOK_APPLY,
}

def make_identifier(text, line, col):
    tok_type = KEYWORDS.get(text, TOK_IDENTIFIER)
    return Token(text, line, col, tok_type, text)

def  make_int(text, line, col):
    return Token(text, line, col, TOK_INT, int(text))

def make_dbl(text, line, col):
    return Token(text, line, col, TOK_DBL, float(text))


def make_str(text, line, col):
    #process eescapes inside string qoutes
    raw = text[1:-1]  # Remove the surrounding quotes
    res = [] #make a list to hold the processed characters
    i = 0
    while i < len(raw):
        if raw[i] == '\\':
            if i + 1 >= len(raw):
                raise ScanError("Unterminated string escape") #when the string ends with a backslash, raise an error
            nxt = raw[i + 1]
            #cases for the different escape sequences
            if nxt == 'n': res.append('\n')
            elif nxt == 't': res.append('\t')
            elif nxt == '"': res.append('"')
            elif nxt == '\\': res.append('\\')
            else:
                raise ScanError(f"Invalid string escape \\{nxt}") #when the escape sequence is not recognized, raise an error
            i += 2
        else:
            res.append(raw[i])
            i += 1
    return Token(text, line, col, TOK_STR, "".join(res))


def make_char(text, line, col):
    name = text[2:] #strip #\
    #handle special cases for character names
    if name == "space": val = ' '
    elif name == "newline": val = '\n'
    elif name == "tab": val = '\t'
    elif len(name) == 1: val = name[0]
    else: 
        raise ScanError(f"Invalid character  name after #\\: {name}") #if the name is not recognized raise an error
    return Token(text, line, col, TOK_CHAR, val)

def make_simple(tok_type, val =None):  #function to make simple tokens like parens, dot, abbrev, etc.
    return lambda text, line, col: Token(text, line, col, tok_type, val if val is not None else text)

# All of the transitions in our scanner.  Order matters.
transitions = [
#start with white space and comments
    Transition(STATE_START, STATE_START, {' ', '\r', '\t', '\n'}, False, False, True, None),
    Transition(STATE_START, STATE_COMMENT, {';'}, False, False, True, None),
    Transition(STATE_COMMENT, STATE_COMMENT, '\n', True, False, True, None), #need two transitions to handle comments, one to consume the comment and one to consume the newline
    Transition(STATE_COMMENT, STATE_START, {'\n'}, False, False, True, None), # and one to transition back to start state after the comment is done



    # Single character delimiters and abbreviations
    Transition(STATE_START, STATE_START, {'('}, False, True, True, make_simple(TOK_LEFT_PAREN)), #make a token for left paren
    Transition(STATE_START, STATE_START, {')'}, False, True, True, make_simple(TOK_RIGHT_PAREN)), #make a right paren token
    Transition(STATE_START, STATE_START, {'\''}, False, True, True, make_simple(TOK_ABBREV)), #consume qoute and emit the '

    #  Plus and Minus
    Transition(STATE_START, STATE_PLUS, {'+'}, False, True, True, None), #consume + and transistion to state plus
    Transition(STATE_PLUS, STATE_NUMBER, DIGITS, False, True, True, None),
    Transition(STATE_PLUS, STATE_START, DELIMITERS, False, False, False, make_identifier), #use make identifier
    Transition(STATE_PLUS, STATE_IDENTIFIER, IDENTITY_CHARS, False, True, True, None), #if followed by +-, treat as multi character identifier

    Transition(STATE_START, STATE_MINUS, {'-'}, False, True, True, None),
    Transition(STATE_MINUS, STATE_NUMBER, DIGITS, False, True, True, None), #same logic as +
    Transition(STATE_MINUS, STATE_START, DELIMITERS, False, False, False, make_identifier),
    Transition(STATE_MINUS, STATE_IDENTIFIER, IDENTITY_CHARS, False, True, True, None),

    # Dot and Decimal numbers
    Transition(STATE_START, STATE_DOT, {'.'}, False, True, True, None), #consume . and see what follows
    Transition(STATE_DOT, STATE_DECIMAL, DIGITS, False, True, True, None), #scan floating point numbers
    Transition(STATE_DOT, STATE_START, DELIMITERS, False, False, False, make_simple(TOK_DOT)), #if followed by delimiter emit a dot token

    # Numbers
    Transition(STATE_START, STATE_NUMBER, DIGITS, False, True, True, None),
    Transition(STATE_NUMBER, STATE_NUMBER, DIGITS, False, True, True, None),
    Transition(STATE_NUMBER, STATE_DECIMAL, {'.'}, False, True, True, None), #if seeing . in state number, transistion to state decimal
    Transition(STATE_DECIMAL, STATE_DECIMAL, DIGITS, False, True, True, None),
    Transition(STATE_NUMBER, STATE_START, DELIMITERS, False, False, False, make_int), #if we see a delimiter, run make int or make dbl and return to state start
    Transition(STATE_DECIMAL, STATE_START, DELIMITERS, False, False, False, make_dbl),

    # Identifiers
    Transition(STATE_START, STATE_IDENTIFIER, IDENTITY_CHARS, False, True, True, None), #check for identity char
    Transition(STATE_IDENTIFIER, STATE_IDENTIFIER, IDENTITY_CHARS, False, True, True, None),
    Transition(STATE_IDENTIFIER, STATE_START, DELIMITERS, False, False, False, make_identifier), #if we hit delimiter do make identifier then return to state start

    #  Strings
    Transition(STATE_START, STATE_STRING, {'"'}, False, True, True, None), #seeing opening " transitistions into state string
    Transition(STATE_STRING, STATE_STRING_ESCAPE, {'\\'}, False, True, True, None), #if we see / transistion to string escape
    Transition(STATE_STRING_ESCAPE, STATE_STRING, set(), False, True, True, None),  # Catch-all after '\'
    Transition(STATE_STRING, STATE_START, {'"'}, False, True, True, make_str), # if we get a non escaped closing qoute execute makestr and return to start state
    Transition(STATE_STRING, STATE_STRING, {'"'}, True, True, True, None),  # Anything except '"'

    # Hash Tokens
    Transition(STATE_START, STATE_HASH, {'#'}, False, True, True, None), #move to state hash if we see #
    Transition(STATE_HASH, STATE_START, {'t'}, False, True, True, make_simple(TOK_BOOL, True)), #emit boolean/vector and reset to state start
    Transition(STATE_HASH, STATE_START, {'f'}, False, True, True, make_simple(TOK_BOOL, False)),
    Transition(STATE_HASH, STATE_START, {'('}, False, True, True, make_simple(TOK_VECTOR, "#(")),
    Transition(STATE_HASH, STATE_CHAR, {'\\'}, False, True, True, None), #if followed by \, enter state_char and emit tokchar with make char once a delimiter is seen
    Transition(STATE_CHAR, STATE_CHAR, LETTERS, False, True, True, None),
    Transition(STATE_CHAR, STATE_START, DELIMITERS, False, False, False, make_char),
]




def scan_tokens(source):
    """Work through `source` and transform it into a list of tokens.

    This will always put an EOF token at the end, unless there is an error.
    If there is an error, then it returns an array with exactly one entry.

    Args:
        source: The source code of the program or a line of REPL input.

    Returns:
        A list of Tokens.
    """

    #initialization

    tokens = []
    current = 0 
    line = 1
    line_start_char = 0
    state = STATE_START
    text = ""

    start_line = 1
    start_col = 1

    #loop through source character by character
    while current <= len:
        ch = source[current] if current < len else '\0'
        matched = false

        for tr in transitions:



    raise ScanError("scan_tokens not implemented")
