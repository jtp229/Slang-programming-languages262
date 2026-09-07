# Scheme-Like Language (slang): Scanning

The following diagrams express the regular language that governs the scanner for slang.
They result in a non-deterministic finite automata (NDFA).

## Punctuation and Comments

This diagram expresses the main concepts of the NDFA format that we'll use to define slang.

- Circles (not filled): These represent transitional states that do not produce a token.
- Circles (filled): These states, when reached, produce a token
- Labels: These indicate which character of input must be present in order to make a transition.
- epsilon transitions: These transitions happen without consuming a character of input.
- Errors: If there is no way out of a state, then the scan should halt, and an error message should be produced.
- Token text: When a filled circle is reached, the relevant token text will be all characters that were observed since the last time the scanner was in the start state.
  Note that this rule allows for INCOMMENT to consume characters of input without creating a token.

With the above rules laid out, we can define the sub-NDFA for punctuation and comments as follows:

```mermaid
graph TD
    %%% These aren't tokens, but they are real states
    %% Each time around the while loop, we start here
    START((START)) 
    %% This is the end of scanning
    EOF((EOFTOKEN))

    %%% These are the states that correspond to Scanner tokens.
    %% Special symbols
    LPAREN((LPAREN))
    RPAREN((RPAREN))
    ABBREV((ABBREV))
    DOT((DOT))
    
    %%% These are the transient and non-accepting states
    %% In a comment
    INCOMMENT((INCOMMENT))
    %% This is our special transition for getting out of one token and into the 
    %% next.
    CLEANBREAK((CLEANBREAK))

    %%% Rules

    %% "Whitespace Doesn't Matter"
    START      --epsilon-->                         CLEANBREAK
    CLEANBREAK --"{' ', '\t', '\n', '\r'}"--> START
    %% The file has to end eventually
    CLEANBREAK --"\0"--> EOF
    
    %% Special Characters
    %% Abbrev never starts something else, never terminates something else
    START       --"'"-->  ABBREV
    ABBREV      --epsilon-->    START
    %% Dot can't start an identifier or terminate something else
    START       --"."-->  DOT
    DOT         --epsilon-->    CLEANBREAK
    %% LPAREN/RPAREN never start something else, but can terminate
    CLEANBREAK  --"("-->  LPAREN
    LPAREN      --epsilon-->    START
    CLEANBREAK  --")"-->  RPAREN
    RPAREN      --epsilon-->    START
    
    %% Comments
    CLEANBREAK --";"--> INCOMMENT
    INCOMMENT --"any character except \n or \0"--> INCOMMENT
    INCOMMENT --"\n"--> START
    %% File can end with a comment
    INCOMMENT  --"\0"--> EOF

    %% Mermaid Formatting: Make sure all the tokens have a special color, to 
    %% distinguish them
    style LPAREN fill:#999999
    style RPAREN fill:#999999
    style ABBREV fill:#999999
    style DOT fill:#999999
    style EOF fill:#999999
```

## Keywords

The following sub-NDFA describes how keywords are detected.
Note that this is something of a shorthand, because it specifies the *whole keyword*, rather than a set of intermediate states as each character is encountered.
The scanner must be able to distinguish between these keywords and an identifier.

```mermaid
graph TD
    %%% These aren't tokens, but they are real states
    %% Each time around the while loop, we start here
    START((START)) 

    %%% These are the states that correspond to Scanner tokens.
    %% Keywords
    AND((AND))
    BEGIN((BEGIN))
    COND((COND))
    DEFINE((DEFINE))
    IF((IF))
    LAMBDA((LAMBDA))
    OR((OR))
    QUOTE((QUOTE))
    SET((SET))
    LET((LET))
    APPLY((APPLY))
    
    %% This is our special transition for getting out of one token and into the 
    %% next.
    CLEANBREAK((CLEANBREAK))

    %%% Rules

    %% Keywords
    START   --"and"-->    AND
    AND     --epsilon--> CLEANBREAK
    START   --"begin"-->  BEGIN
    BEGIN   --epsilon--> CLEANBREAK
    START   --"cond"-->   COND
    COND    --epsilon--> CLEANBREAK
    START   --"define"--> DEFINE
    DEFINE  --epsilon--> CLEANBREAK
    START   --"if"-->     IF
    IF      --epsilon--> CLEANBREAK
    START   --"lambda"--> LAMBDA
    LAMBDA  --epsilon--> CLEANBREAK
    START   --"or"-->     OR
    OR      --epsilon--> CLEANBREAK
    START   --"quote"-->  QUOTE
    QUOTE   --epsilon--> CLEANBREAK
    START   --"set!"-->   SET
    SET     --epsilon--> CLEANBREAK
    START   --"let"-->    LET
    LET     --epsilon--> CLEANBREAK
    START   --"apply"-->  APPLY
    APPLY   --epsilon--> CLEANBREAK

    %% Mermaid Formatting: Make sure all the tokens have a special color, to 
    %% distinguish them
    style AND fill:#999999
    style BEGIN fill:#999999
    style COND fill:#999999
    style DEFINE fill:#999999
    style IF fill:#999999
    style LAMBDA fill:#999999
    style OR fill:#999999
    style QUOTE fill:#999999
    style SET fill:#999999
    style LET fill:#999999
    style APPLY fill:#999999
```

## Vector, Char, and Bool

The punctuation for the start of a vector shares a common starting token with the start of a bool or character.
Furthermore, there are a few special character tokens that require more than three characters to express.
This NDFA captures those possibilities.
Note that the keyword sub-NDFA is highly relevant to this sub-NDFA, because this, too, has shorthand for some multi-character sequences that produce a single token.

```mermaid
graph TD
    %%% These aren't tokens, but they are real states
    %% Each time around the while loop, we start here
    START((START)) 

    %%% These are the states that correspond to Scanner tokens.
    %% Special symbols
    VEC((VEC))
    %% Things with a value that matters
    BOOL((BOOL))
    CHAR((CHAR))
    
    %%% These are the transient and non-accepting states
    %% Start of a vector/character/bool
    VCB((VCB))
    %% We're in a character but it might not be valid
    PRECHAR((PRECHAR))
    %% This is our special transition for getting out of one token and into the 
    %% next.
    CLEANBREAK((CLEANBREAK))

    %%% Rules

    %% VCB == Vec or Char or Bool
    START --"#35;"--> VCB 
    %% Space/paren not needed after VEC symbol
    VCB     --"("-->      VEC
    VEC     --epsilon-->        START
    %% Space/paren is needed after a boolean
    VCB     --"{t, f}"--> BOOL
    BOOL    --epsilon-->        CLEANBREAK
    %% CHAR is tricky because of the special characters newline/space/tab
    VCB     --"\"-->      PRECHAR
    PRECHAR --"any character not in {' ', '\t', '\n', '\r', '\0'}"--> CHAR
    VCB     --"{'\newline', '\space', '\tab'}"--> CHAR
    CHAR    --epsilon--> CLEANBREAK
    
    %% Mermaid Formatting: Make sure all the tokens have a special color, to 
    %% distinguish them
    style VEC fill:#999999
    style BOOL fill:#999999
    style CHAR fill:#999999
```

## Identifiers and Numbers

Slang treats `+` and `-` as identifiers, not special tokens.
These characters can also begin a number.
This leads to a slight entanglement of the NDFAs for identifiers and numbers.

Numbers introduce a few more considerations.
First, we will not allow a number to begin with `.`.
There must always be a numerical value on the left hand side, even though it might be `0`.
Second, we do not validate that numerical constants fit within the bit sizes supported by the language.
A 70-bit integer constant might cause the scanner to fail in Java but not in Python, and you should know why.
Third, the constants `NaN` and `+/- inf` are not part of the grammar.
These are run-time constants.

```mermaid
graph TD
    %%% These aren't tokens, but they are real states
    %% Each time around the while loop, we start here
    START((START)) 

    %%% These are the states that correspond to Scanner tokens.
    IDENTIFIER((IDENTIFIER))
    DBL((DBL))
    INT((INT))
    
    %%% These are the transient and non-accepting states
    %% In an integer
    ININT((ININT))
    %% In a double
    PREDBL((PREDBL))
    INDBL((INDBL))
    %% Not yet sure if we've got +, -, or an identifier
    PM((PM))
    %% In the middle of an identifier
    INID((INID))
    %% This is our special transition for getting out of one token and into the 
    %% next.
    CLEANBREAK((CLEANBREAK))

    %%% Rules
   
    %% Identifiers and numbers are tricky because of +/-
    %% Simple identifiers don't start with +/-/[0-9]
    START       --"{!$%&*/:<=>?~_^} or {a-z} or {A-Z}"--> INID
    INID        --"{!$%&*/:<=>?~_^} or {0-9} or {a-z} or {A-Z} or {.+-}"--> INID
    INID        --epsilon--> IDENTIFIER
    IDENTIFIER  --epsilon--> CLEANBREAK
    %% +/- as a single character identifier
    START --"{+, -}"--> PM
    PM --epsilon--> IDENTIFIER
    %% +/- as the start of a number
    PM --"{0-9}"--> ININT
    %% Starting a number without +/-
    START   --"{0-9}"--> ININT
    ININT   --"{0-9}"--> ININT
    ININT   --epsilon--> INT
    INT     --epsilon--> CLEANBREAK
    %% A decimal point means it could be a double as long as we get another digit
    ININT   --"."--> PREDBL
    PREDBL  --"{0-9}"--> INDBL
    INDBL   --"{0-9}"--> INDBL
    INDBL   --epsilon--> DBL
    DBL     --epsilon--> CLEANBREAK

    %% Mermaid Formatting: Make sure all the tokens have a special color, to 
    %% distinguish them
    style IDENTIFIER fill:#999999
    style DBL fill:#999999
    style INT fill:#999999
```

## Strings

Strings are relatively straightforward, except that there are a few escape sequences.

```mermaid
graph TD
    %%% These aren't tokens, but they are real states
    %% Each time around the while loop, we start here
    START((START)) 

    %%% These are the states that correspond to Scanner tokens.
    %% Special symbols
    STR((STR))
    
    %%% These are the transient and non-accepting states
    %% In a string, awaiting quotation mark
    INSTR((INSTR))
    %% In an escape sequence within a string
    INSTR+((INSTR+))
    %% This is our special transition for getting out of one token and into the 
    %% next.
    CLEANBREAK((CLEANBREAK))

    %%% Rules

    %% Strings
    START   --"&quot;"--> INSTR
    INSTR   --"any character not in {&quot;, #92;, \n, \r, \t}"--> INSTR
    INSTR   --"\"--> INSTR+
    INSTR+  --"{&quot;, #92;, t, n, r}"--> INSTR
    INSTR   --"&quot;"--> STR
    STR     --epsilon--> CLEANBREAK

    %% Mermaid Formatting: Make sure all the tokens have a special color, to 
    %% distinguish them
    style STR fill:#999999
```

## Full Diagram

The full grammar is achieved by joining all of the preceding NDFAs.

```mermaid
graph TD
    %%% These aren't tokens, but they are real states
    %% Each time around the while loop, we start here
    START((START)) 
    %% This is the end of scanning
    EOF((EOFTOKEN))

    %%% These are the states that correspond to Scanner tokens.
    %% Special symbols
    LPAREN((LPAREN))
    RPAREN((RPAREN))
    ABBREV((ABBREV))
    DOT((DOT))
    VEC((VEC))
    %% Things with a value that matters
    BOOL((BOOL))
    IDENTIFIER((IDENTIFIER))
    STR((STR))
    CHAR((CHAR))
    DBL((DBL))
    INT((INT))
    %% Keywords
    AND((AND))
    BEGIN((BEGIN))
    COND((COND))
    DEFINE((DEFINE))
    IF((IF))
    LAMBDA((LAMBDA))
    OR((OR))
    QUOTE((QUOTE))
    SET((SET))
    LET((LET))
    APPLY((APPLY))
    
    %%% These are the transient and non-accepting states
    %% Start of a vector/character/bool
    VCB((VCB))
    %% We're in a character but it might not be valid
    PRECHAR((PRECHAR))
    %% In a comment
    INCOMMENT((INCOMMENT))
    %% In a string, awaiting quotation mark
    INSTR((INSTR))
    %% In an escape sequence within a string
    INSTR+((INSTR+))
    %% In an integer
    ININT((ININT))
    %% In a double
    PREDBL((PREDBL))
    INDBL((INDBL))
    %% Not yet sure if we've got +, -, or an identifier
    PM((PM))
    %% In the middle of an identifier
    INID((INID))
    %% This is our special transition for getting out of one token and into the 
    %% next.
    CLEANBREAK((CLEANBREAK))

    %%% Rules

    %% "Whitespace Doesn't Matter"
    START      --epsilon-->                         CLEANBREAK
    CLEANBREAK --"{' ', '\t', '\n', '\r'}"--> START
    %% The file has to end eventually
    CLEANBREAK --"\0"--> EOF
    
    %% Special Characters
    %% Abbrev never starts something else, never terminates something else
    START       --"'"-->  ABBREV
    ABBREV      --epsilon-->    START
    %% Dot can't start an identifier or terminate something else
    START       --"."-->  DOT
    DOT         --epsilon-->    CLEANBREAK
    %% LPAREN/RPAREN never start something else, but can terminate
    CLEANBREAK  --"("-->  LPAREN
    LPAREN      --epsilon-->    START
    CLEANBREAK  --")"-->  RPAREN
    RPAREN      --epsilon-->    START
    
    %% Comments
    CLEANBREAK --";"--> INCOMMENT
    INCOMMENT --"any character except \n or \0"--> INCOMMENT
    INCOMMENT --"\n"--> START
    %% File can end with a comment
    INCOMMENT  --"\0"--> EOF

    %% VCB == Vec or Char or Bool
    START --"#35;"--> VCB 
    %% Space/paren not needed after VEC symbol
    VCB     --"("-->      VEC
    VEC     --epsilon-->        START
    %% Space/paren is needed after a boolean
    VCB     --"{t, f}"--> BOOL
    BOOL    --epsilon-->        CLEANBREAK
    %% CHAR is tricky because of the special characters newline/space/tab
    VCB     --"\"-->      PRECHAR
    PRECHAR --"any character not in {' ', '\t', '\n', '\r', '\0'}"--> CHAR
    VCB     --"{'\newline', '\space', '\tab'}"--> CHAR
    CHAR    --epsilon--> CLEANBREAK
    
    %% Identifiers and numbers are tricky because of +/-
    %% Simple identifiers don't start with +/-/[0-9]
    START       --"{!$%&*/:<=>?~_^} or {a-z} or {A-Z}"--> INID
    INID        --"{!$%&*/:<=>?~_^} or {0-9} or {a-z} or {A-Z} or {.+-}"--> INID
    INID        --epsilon--> IDENTIFIER
    IDENTIFIER  --epsilon--> CLEANBREAK
    %% +/- as a single character identifier
    START --"{+, -}"--> PM
    PM --epsilon--> IDENTIFIER
    %% +/- as the start of a number
    PM --"{0-9}"--> ININT
    %% Starting a number without +/-
    START   --"{0-9}"--> ININT
    ININT   --"{0-9}"--> ININT
    ININT   --epsilon--> INT
    INT     --epsilon--> CLEANBREAK
    %% A decimal point means it could be a double as long as we get another digit
    ININT   --"."--> PREDBL
    PREDBL  --"{0-9}"--> INDBL
    INDBL   --"{0-9}"--> INDBL
    INDBL   --epsilon--> DBL
    DBL     --epsilon--> CLEANBREAK

    %% Keywords
    START   --"and"-->    AND
    AND     --epsilon--> CLEANBREAK
    START   --"begin"-->  BEGIN
    BEGIN   --epsilon--> CLEANBREAK
    START   --"cond"-->   COND
    COND    --epsilon--> CLEANBREAK
    START   --"define"--> DEFINE
    DEFINE  --epsilon--> CLEANBREAK
    START   --"if"-->     IF
    IF      --epsilon--> CLEANBREAK
    START   --"lambda"--> LAMBDA
    LAMBDA  --epsilon--> CLEANBREAK
    START   --"or"-->     OR
    OR      --epsilon--> CLEANBREAK
    START   --"quote"-->  QUOTE
    QUOTE   --epsilon--> CLEANBREAK
    START   --"set!"-->   SET
    SET     --epsilon--> CLEANBREAK
    START   --"let"-->    LET
    LET     --epsilon--> CLEANBREAK
    START   --"apply"-->  APPLY
    APPLY   --epsilon--> CLEANBREAK

    %% Strings
    START   --"&quot;"--> INSTR
    INSTR   --"any character not in {&quot;, #92;, \n, \r, \t}"--> INSTR
    INSTR   --"\"--> INSTR+
    INSTR+  --"{&quot;, #92;, t, n, r}"--> INSTR
    INSTR   --"&quot;"--> STR
    STR     --epsilon--> CLEANBREAK

    %% Mermaid Formatting: Make sure all the tokens have a special color, to 
    %% distinguish them
    style LPAREN fill:#999999
    style RPAREN fill:#999999
    style ABBREV fill:#999999
    style VEC fill:#999999
    style BOOL fill:#999999
    style IDENTIFIER fill:#999999
    style STR fill:#999999
    style CHAR fill:#999999
    style DBL fill:#999999
    style INT fill:#999999
    style AND fill:#999999
    style BEGIN fill:#999999
    style COND fill:#999999
    style DEFINE fill:#999999
    style IF fill:#999999
    style LAMBDA fill:#999999
    style OR fill:#999999
    style QUOTE fill:#999999
    style SET fill:#999999
    style LET fill:#999999
    style DOT fill:#999999
    style EOF fill:#999999
    style APPLY fill:#999999
```
