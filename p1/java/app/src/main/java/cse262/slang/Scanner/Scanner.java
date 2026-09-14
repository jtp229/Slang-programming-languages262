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

      private void scanIdentifier(List<Tokens.Token> tokens) throws ScanError {
    // Keep consuming valid identifier characters
    while (!isAtEnd(source) && isIdentifierpart(peek(source))) {
        advance(source);
    }
    
    if (!peekIsDelimiter()) { // Check if the next character is a delimiter
        throw new ScanError("Invalid character in or following identifier");
    }

    String text = source.substring(start, current);
    tokens.add(classifyIdentifier(text, current_line, col())); // Classify and add the identifier token
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
    //method to post process identifiers
    private Tokens.Token classifyIdentifier(String text, int line, int col){
        switch(text){ //check if the identifier is a keyword in the diagram
            case "and": return new Tokens.And(text, line, col);
            case "begin": return new Tokens.Begin(text, line, col);
            case "cond": return new Tokens.Cond(text, line, col);
            case "define": return new Tokens.Define(text, line, col);
            case "if": return new Tokens.If(text, line, col);
            case "lambda": return new Tokens.Lambda(text, line, col);
            case "or": return new Tokens.Or(text, line, col);
            case "quote": return new Tokens.Quote(text, line, col);
            case "set!": return new Tokens.Set(text, line, col);
            case "let": return new Tokens.Let(text, line, col);
            case "apply": return new Tokens.Apply(text, line, col);
            // If its not one of these default to general identifier
            default: return new Tokens.Identifier(text, line, col);
    }
}
    private void scanNumber(List<Tokens.Token> tokens) throws ScanError { //method for scanning integers and doubles
        boolean isDouble = false; //flag to check if the number is a double

        while (!isAtEnd(source) && Character.isDigit(peek(source))) {
            advance(source); //consume digit
    }
    
        if (!isAtEnd(source) && peek(source) == '.') { //check for decimal point         
                if (current + 1 < source.length() && Character.isDigit(source.charAt(current + 1))) { //check if the next character is a digit
                    isDouble = true; //set flag to true if we see a decimal point
                    advance(source); //consume '.'

                    //consume fractional digit
                while (!isAtEnd(source) && Character.isDigit(peek(source))) {
                    advance(source); //consume digit
                }

            }
            
            }
           if (!peekIsDelimiter()) {
    advance(source); 
    throw new ScanError("Invalid character following numeric literal");
}
            String text = source.substring(start, current); //get the number text
            if (isDouble) {
    double val = Double.parseDouble(text); //parse the number as a double
    tokens.add(new Tokens.Dbl(text, current_line, col(), val));
} else {
    int val = Integer.parseInt(text); //parse the number as an integer
    tokens.add(new Tokens.Int(text, current_line, col(), val));
}

        }

       
private void scanHash(List<Tokens.Token> tokens) throws ScanError { //method for scanning hash tokens
    if (isAtEnd(source)) {
        throw new ScanError("Unexpected end of input after #");
    }
    char c = advance(source); //consume the character after the hash
    String text = source.substring(start, current); //get the text of the token
    switch (c) {
        case 't': 
            tokens.add(new Tokens.Bool(text, current_line, col(), true));
            break;
        case 'f':
            tokens.add(new Tokens.Bool(text, current_line, col(), false));
            break;
        case '(': // Handle vector literal
            // Scan until we find the closing parenthesis
            tokens.add(new Tokens.Vec(text, current_line, col()));
            break;
        case '\\':
            if (isAtEnd(source) || peek(source) == '\n' || peek(source) == '\r') {
                throw new ScanError("Expected character or character name after #\\");
            }
            //read ahead to see if we have a named character
            int charStart = current;
            while (!isAtEnd(source) && Character.isLetter(peek(source))) {
                advance(source);
            }
            //if no letters consumed advance at least one character
            if(current == charStart && !isAtEnd(source)) {
                advance(source);
            }
            String charName = source.substring(charStart, current); //get the character name
            char parsedChar; 
            switch (charName) { 
                case "space": parsedChar = ' '; break; 
                case "newline": parsedChar = '\n'; break;
                case "tab": parsedChar = '\t'; break; 
                default:
                    if (charName.length() == 1) { 
                        parsedChar = charName.charAt(0); //parse single character
                    } else {
                        throw new ScanError("invalid character name after #\\: " + charName);
                    }
                    break;
            }
            if (!peekIsDelimiter()) {
                throw new ScanError("Invalid character following character literal");
            }
            text = source.substring(start, current);
            tokens.add(new Tokens.Char(text, current_line, col(), parsedChar)); // Add the character token to the list
            break;
        default:
            throw new ScanError("invalid character after #: " + c);
    }
}
private boolean isDelimiter(char c) { //method to check if a character is a delimiter
    return Character.isWhitespace(c) || c == '(' || c == ')' 
        || c == ';' || c == '"' || c == '\0'; 
}

private boolean peekIsDelimiter() { //method to check if the next character is a delimiter
    if (isAtEnd(source)) return true;
    return isDelimiter(peek(source));
}

private void scanString(List<Tokens.Token> tokens) throws ScanError {  //method for scanning string tokens
    int tokenStartLine = current_line; // Save the line number where the string starts
    int tokenStartCol = col(); // Save the column number where the string starts

    StringBuilder sb = new StringBuilder();

    while (!isAtEnd(source) && peek(source) != '"') {
        char c = peek(source);

        if (c == '\n') { // Handle multi-line strings
            current_line++;
            advance(source);
            line_start_char = current;
            sb.append('\n');
            continue;
        }

        if (c == '\\') { // Handle escape sequences
            advance(source); // Consume '\'
            if (isAtEnd(source)) {
                current_line = tokenStartLine;
                start = line_start_char + (tokenStartCol - 1);
                throw new ScanError("Unterminated string escape");
            }
            char escaped = advance(source); // Consume escaped char
            switch (escaped) {
                case 'n':  sb.append('\n'); break;
                case 't':  sb.append('\t'); break;
                case '"':  sb.append('"');  break;
                case '\\': sb.append('\\'); break;
                default:
                    throw new ScanError("Invalid string escape \\" + escaped);
            }
            continue;
        }

        sb.append(advance(source)); // Appends 's' on first iteration
    }

    if (isAtEnd(source)) { // Handle unterminated string
        current_line = tokenStartLine;
        start = line_start_char + (tokenStartCol - 1);
        throw new ScanError("Unterminated string");
    }

    advance(source); // Consume closing quote "
    String text = source.substring(start, current);
    tokens.add(new Tokens.Str(text, tokenStartLine, tokenStartCol, sb.toString()));
}
private boolean isIdentifierpart(char c) {
        if (Character.isLetterOrDigit(c)) {
        return true;
    }
    //cases for symbol characters
    switch(c) {
       case '!': case '$': case '%': case '&': case '*':
        case '/': case ':': case '<': case '=': case '>':
        case '?': case '^': case '_': case '~': case '+':
        case '-': case '.':
            return true;
        default:
            return false;
    }
}
  

   private void scanToken(List<Tokens.Token> tokens) throws ScanError { //method for scanning a single token
    int startCol = current - line_start_char + 1; //
    char c = advance(source);

    switch (c) {
        case '(': 
            tokens.add(new Tokens.LeftParen(source.substring(start, current), current_line, col()));
            break;
        case ')':
            tokens.add(new Tokens.RightParen(source.substring(start, current), current_line, col()));
            break;
        case '.': //handle dot token, check if the next character is a delimiter, if not throw an error
            if (!peekIsDelimiter()) {
                start = current; 
                throw new ScanError("Invalid character after dot");
            }
            tokens.add(new Tokens.Dot(source.substring(start, current), current_line, startCol));
            break;
        case '\'': 
            tokens.add(new Tokens.Abbrev(source.substring(start, current), current_line, col()));
            break;
        case '+': 
        case '-':
            if (Character.isDigit(peek(source))) { //if the next character is a digit, then this is a number token, so we call scanNumber
        scanNumber(tokens);
    } else if (peekIsDelimiter()) {
        scanIdentifier(tokens); // Single '+' or '-' is a valid identifier
    } else {
        throw new ScanError("Invalid token starting with " + c);
    }
    break;

        case ';':
            while (!isAtEnd(source) && peek(source) != '\n') {
                advance(source);
            }
            break;
        case '"':
            scanString(tokens);
            break;
        case '#':
            scanHash(tokens);
            break;
//newline cases and whitespace
        case ' ':
        case '\r':
        case '\t':
            break;
        case '\n':
            current_line++;
            line_start_char = current;
            break;
        default:
            if (Character.isDigit(c)) {
                scanNumber(tokens);
            } else if (isIdentifierpart(c)) {
                scanIdentifier(tokens);
            } else {
                throw new ScanError("Unexpected character: " + c);
            }
            break;
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

        while (!isAtEnd(source)) {
            start = current;
            scanToken(tokens);
        }
        //add EOF
        tokens.add(new Tokens.Eof("", current_line, col()));
        return new TokenStream(tokens);
}}