
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
STATE_CLEAN_BREAK = 1
STATE_IN_COMMENT = 2
STATE_HASH = 3  # after '#': vector, character, or bool
STATE_PRE_CHAR = 4  # after '#\'
STATE_IN_CHAR_NAME = 5  # named character like newline/space/tab
STATE_CHAR_DONE = 6  # single-character literal, waiting for a delimiter
STATE_IN_STRING = 7
STATE_IN_STRING_ESCAPE = 8
STATE_IN_INT = 9
STATE_PRE_DOUBLE = 10  # digits, then '.', waiting for a fractional digit
STATE_IN_DOUBLE = 11
STATE_PLUS_MINUS = 12  # '+' or '-', identifier vs signed number
STATE_IN_IDENTIFIER = 13


#define character sets for transitions
WHITESPACE = set(" \t\n\r") #give whitepsace characters there own set
DELIMITERS = WHITESPACE | set("();\0")
DIGITS = set("0123456789")
LETTERS = set("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
ID_START = LETTERS | set("!$%&*/:<=>?~_^")
ID_CONT = ID_START | DIGITS | set(".+-")


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
    if text.endswith('.'):  # Reject numbers ending with dot
        raise ScanError(f"Invalid number: {text}")
    return Token(text, line, col, TOK_DBL, float(text))


def make_str(text, line, col):
   # print(f"DEBUG make_str: text='{text}' len={len(text)}")
    #process eescapes inside string qoutes
    raw = text[1:-1]  # Remove the surrounding quotes
    res = [] #make a list to hold the processed characters
    i = 0
    curr_line, curr_col = line, col + 1
    while i < len(raw):
        if raw[i] == '\\':
            if i + 1 >= len(raw):
                raise ScanError(f"Scan Error: line {curr_line}, col {curr_col}") #when the string ends with a backslash, raise an error
            nxt = raw[i + 1]
            #cases for the different escape sequences
            escapes = {'n': '\n', 't': '\t', '"': '"', '\\': '\\', 'r': '\r'}
            if nxt in escapes:
                res.append(escapes[nxt])
            else:
                raise ScanError(f"Scan Error: line {curr_line}, col {curr_col}") #when the escape sequence is not recognized, raise an error
            i += 2
        else:
            res.append(raw[i])
            i += 1
    return Token(text, line, col, TOK_STR, "".join(res))

def make_char(text, line, col):
    name = text[2:]  # Strip #\
    char_map = { #use a dictionary to map the character names to the actual characters
        "space": ' ',
        "newline": '\n',
        "tab": '\t',
        "backspace": '\b',
        "alarm": '\a',
        "null": '\0',
    }
    if name in char_map:
        val = char_map[name]
    elif len(name) == 1:
        val = name
    else:
        # Report error at start of character name (col + 2)
        raise ScanError(f"Scan Error: line {line}, col {col}")
    return Token(text, line, col, TOK_CHAR, val)

def make_simple(tok_type, val=None):  #function to make simple tokens like parens, dot, abbrev, etc.
    return lambda text, line, col: Token(
        text, line, col, tok_type, val if val is not None else text )
def make_eof(text, line, col):
    return Token("", line, col, TOK_EOF, "")

# All of the transitions in our scanner.  Order matters.
transitions = [
    # Whitespace and comments
    # START: tokens that do not require a preceding clean break, then epsilon
    Transition(STATE_START, STATE_START, {"'"}, False, True, True, make_simple(TOK_ABBREV)), #consume qoute and emit the '
    Transition(STATE_START, STATE_CLEAN_BREAK, {"."}, False, True, True, make_simple(TOK_DOT)), #consume . and see what follows
    Transition(STATE_START, STATE_HASH, {"#"}, False, True, True, None), #move to state hash if we see #
    Transition(STATE_START, STATE_IN_STRING, {'"'}, False, True, True, None), #seeing opening " transitistions into state string
    Transition(STATE_START, STATE_PLUS_MINUS, {"+", "-"}, False, True, True, None), #consume + and transistion to state plus
    Transition(STATE_START, STATE_IN_INT, DIGITS, False, True, True, None),
    Transition(STATE_START, STATE_IN_IDENTIFIER, ID_START, False, True, True, None), #check for identity char
    Transition(STATE_START, STATE_CLEAN_BREAK, None, False, False, False, None),
    # CLEAN_BREAK: whitespace, EOF, parens, comments
    Transition(STATE_CLEAN_BREAK, STATE_START, WHITESPACE, False, False, True, None),
    Transition(STATE_CLEAN_BREAK, STATE_CLEAN_BREAK, {"\0"}, False, False, False, make_eof),
    # delimiters and abbreviations
    Transition(STATE_CLEAN_BREAK, STATE_START, {"("}, False, True, True, make_simple(TOK_LEFT_PAREN)), #make a token for left paren
    Transition(STATE_CLEAN_BREAK, STATE_START, {")"}, False, True, True, make_simple(TOK_RIGHT_PAREN)), #make a right paren token
    Transition(STATE_CLEAN_BREAK, STATE_IN_COMMENT, {";"}, False, False, True, None),
    # Comments
    Transition(STATE_IN_COMMENT, STATE_IN_COMMENT, {"\n", "\0"}, True, False, True, None), #need two transitions to handle comments, one to consume the comment and one to consume the newline
    Transition(STATE_IN_COMMENT, STATE_START, {"\n"}, False, False, True, None), # and one to transition back to start state after the comment is done
    Transition(STATE_IN_COMMENT, STATE_CLEAN_BREAK, {"\0"}, False, False, False, make_eof),

    # Hash Tokens
    Transition(STATE_HASH, STATE_START, {"("}, False, True, True, make_simple(TOK_VECTOR)),
    Transition(STATE_HASH, STATE_CLEAN_BREAK, {"t"}, False, True, True, make_simple(TOK_BOOL, "true")),
    Transition(STATE_HASH, STATE_CLEAN_BREAK, {"f"}, False, True, True, make_simple(TOK_BOOL, "false")), #emit boolean/vector and reset to state start
    Transition(STATE_HASH, STATE_PRE_CHAR, {"\\"}, False, True, True, None),
    Transition(STATE_PRE_CHAR, STATE_IN_CHAR_NAME, LETTERS, False, True, True, None),
    Transition(STATE_PRE_CHAR,STATE_CHAR_DONE, WHITESPACE | {"\0"}, True, True, True,None,), #combine whitespace and \0 into one set
    Transition(STATE_IN_CHAR_NAME, STATE_IN_CHAR_NAME, LETTERS, False, True, True, None),
    Transition(STATE_IN_CHAR_NAME, STATE_CLEAN_BREAK, DELIMITERS, False, False, False, make_char),
    Transition(STATE_CHAR_DONE, STATE_CLEAN_BREAK, DELIMITERS, False, False, False, make_char),

    #  Plus and Minus
    #  check IDENTITY_CHARS (excluding DIGITS) before falling back
    Transition(STATE_PLUS_MINUS, STATE_IN_INT, DIGITS, False, True, True, None), #if followed by +-, treat as multi character identifier
    Transition(STATE_PLUS_MINUS, STATE_CLEAN_BREAK, DELIMITERS, False, False, False, make_identifier), #use make identifier

    # Identifiers
    Transition(STATE_IN_IDENTIFIER, STATE_IN_IDENTIFIER, ID_CONT, False, True, True, None),
    Transition(STATE_IN_IDENTIFIER, STATE_CLEAN_BREAK, DELIMITERS, False, False, False, make_identifier), #if we hit delimiter do make identifier then return to state start

    # Numbers
    Transition(STATE_IN_INT, STATE_IN_INT, DIGITS, False, True, True, None),
    Transition(STATE_IN_INT, STATE_PRE_DOUBLE, {"."}, False, True, True, None), #if seeing . in state number, transistion to state decimal
    Transition(STATE_IN_INT, STATE_CLEAN_BREAK, DELIMITERS, False, False, False, make_int), #if we see a delimiter, run make int or make dbl and return to state start
    Transition(STATE_PRE_DOUBLE, STATE_IN_DOUBLE, DIGITS, False, True, True, None), #scan floating point numbers
    Transition(STATE_IN_DOUBLE, STATE_IN_DOUBLE, DIGITS, False, True, True, None),
    Transition(STATE_IN_DOUBLE, STATE_CLEAN_BREAK, DELIMITERS, False, False, False, make_dbl),

    #  Strings
    Transition(STATE_IN_STRING, STATE_IN_STRING_ESCAPE, {"\\"}, False, True, True, None), #if we see / transistion to string escape
    Transition(STATE_IN_STRING, STATE_CLEAN_BREAK, {'"'}, False, True, True, make_str), # if we get a non escaped closing qoute execute makestr and return to start state
    Transition(STATE_IN_STRING,STATE_IN_STRING,{'"', "\\", "\n", "\r", "\t", "\0"},True,True,True,None,),  # Anything except '"' and EOF combine all the characters into one set
    Transition(STATE_IN_STRING_ESCAPE, STATE_IN_STRING, {'"', "\\", "t", "n", "r"}, False, True, True, None),  # Catch-all after '\'
]





def scan_tokens(source):
    #print(f"DEBUG: source length={len(source)}")
    #for i, c in enumerate(source):
        #print(f"DEBUG: source[{i}] = '{c}' (ord={ord(c)})")
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
    def col_at(pos): #convert a 0 index string into a column number based on current line
        return pos - line_start_char + 1
    try:
        while True:
     #loop through source character by character
            ch = source[current] if current < len(source) else '\0'
            matched = False

            for tr in transitions:
                if tr.from_state != state: #if the from state does not match the active state, skip
                 continue
                if tr.peek_set is not None: #if tr.invert_peek is False it only atches if ch is in the peek set.  If its true it only matches if its NOT in the peek set.  Feels kinda backwards
                    in_set = ch in tr.peek_set 
                    if tr.invert_peek and in_set:
                        continue
                    if not tr.invert_peek and not in_set:
                        continue
                matched = True #if all satisfied, then it matches


                if len(text) == 0 and tr.consume:
                    start_line = line
                    start_col = col_at(current)

                if tr.consume: #if consume is true, add the character to text
                    text += ch

                if tr.advance:
                    if ch == '\n': #update the line counter if we have '\n'
                        line += 1
                        line_start_char = current + 1
                    current += 1

                if tr.maker is not None:
                    tok = tr.maker(text, start_line, start_col) #build a token if maker is present
                    tokens.append(tok)
                    text = "" #reset buffer for the next token
                    if tok.tok_type == TOK_EOF:
                        return tokens

                state = tr.to_state
                break
                
            if not matched:
                if text:
                    err_line, err_col = start_line, start_col
                else:
                    err_line, err_col = line, col_at(current)
                raise ScanError(f"Scan Error: line {err_line}, col {err_col}")
    except ScanError as err:
        msg = str(err)
    # If the maker already formatted a full "Scan Error:" message, use it directly
        if not msg.startswith("Scan Error:"):
            msg = f"Scan Error: line {line}, col {col_at(current)}"
        return [Token(msg, line, col_at(current), TOK_ERROR, "")]




    
