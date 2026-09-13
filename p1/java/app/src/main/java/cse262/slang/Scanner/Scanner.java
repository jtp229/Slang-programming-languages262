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

      private void scanIdentifier(List<Tokens.Token> tokens) {
        //keep moving as long as character is valid
        while (!isAtEnd(source) && isIdentifierpart(peek(source))) {
            advance(source);
        }
        String text = source.substring(start, current); //get the identifier text
        tokens.add(classifyIdentifier(text, current_line, col()));

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
    private void scanNumber(List<Tokens.Token> tokens) { //method for scanning integers and doubles
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
        case '(':
            tokens.add(new Tokens.Vec(text, current_line, col()));
            break;
        case '\\':
            if (isAtEnd(source)) {
                throw new ScanError("Unexpected end of input after #\\");
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
            String charName = source.substring(charStart, current);
            char parsedChar;
            switch (charName) {
                case "space": parsedChar = ' '; break;
                case "newline": parsedChar = '\n'; break;
                case "tab": parsedChar = '\t'; break;
                default:
                    if (charName.length() == 1) {
                        parsedChar = charName.charAt(0);
                    } else {
                        throw new ScanError("invalid character name after #\\: " + charName);
                    }
                    break;
            }
            text = source.substring(start, current);
            tokens.add(new Tokens.Char(text, current_line, col(), parsedChar));
            break;
        default:
            throw new ScanError("invalid character after #: " + c);
    }
}
private void scanString(List<Tokens.Token> tokens) throws ScanError { //method for scanning string tokens
    while (!isAtEnd(source) && peek(source) != '"') {
        if (peek(source) == '\n') {
            current_line++; //increment line number if we see a newline
            line_start_char = current; //reset line_start_char
        }
        //handle escape sequences
        if (peek(source) == '\\'){
            advance(source); //consume the backslash
            if (isAtEnd(source)) {
                throw new ScanError("undended escape sequence in string");
            }

        }
        advance(source); 
    }
    if (isAtEnd(source)) {
        throw new ScanError("unterminated string literal");
    }
    //consume closing quote
    advance(source);
    //full text w/ qoutes
    String text = source.substring(start, current);

    //string content 
    StringBuilder valueBuilder = new StringBuilder();
    for (int i = 1; i < current - 1; i++) { //skip the first and last quote
        char c = source.charAt(i);
        if (c == '\\' && i + 1 < current - 1) { //handle escape sequences
            char next = source.charAt(i+1);
            switch (next) {
                case '"':  valueBuilder.append('"'); i++; break;
                case '\\': valueBuilder.append('\\'); i++; break;
                case 'n':  valueBuilder.append('\n'); i++; break;
                case 't':  valueBuilder.append('\t'); i++; break;
                case 'r':  valueBuilder.append('\r'); i++; break;
                default:   valueBuilder.append(c); break;
            }
        } else {
            valueBuilder.append(c);
        }
    }

    tokens.add(new Tokens.Str(text, current_line, col(), valueBuilder.toString()));
}
    



    private boolean isIdentifierpart(char c) {
        if (Character.isLetterOrDigit(c)) {
        return true;
    }
    //cases for symbol characters
    switch(c) {
       case '!': case '$': case '%': case '&': 
        case '*': case '/': case ':': case '<': 
        case '=': case '>': case '?': case '~': 
        case '_': case '^': case '-': case '+':
            return true;
        default:
            return false;
    }
}
  

    private void scanToken(List<Tokens.Token> tokens) throws ScanError { //helper method to scan tokens
        char c = advance(source);

        switch(c) {
            //single character tokens
            case '(': 
            tokens.add(new Tokens.LeftParen(source.substring(start, current), current_line, col()));
            break;
            case ')':
            tokens.add(new Tokens.RightParen(source.substring(start, current), current_line, col()));
            break;
            case '\'':
            tokens.add(new Tokens.Abbrev(source.substring(start, current), current_line, col()));
            break;
            case '+':
            case '-':
                if (Character.isDigit(peek(source))) {
                    scanNumber(tokens); //if the next character is a digit, scan as a number
                } else {
                    scanIdentifier(tokens); //otherwise scan as identifier + or -
                }
                break;

            case ';': //case for single line comments
             while (!isAtEnd(source) && peek(source) != '\n') {
            advance(source); //consume characters until we reach a newline
        }
        break;
            case '"':
                scanString(tokens); //scan string token
                break;
            case '#':
                scanHash(tokens); //scan hash token
                break;
            case '.': //case for standalone dot as in not double or decimal point
                tokens.add(new Tokens.Dot(source.substring(start, current), current_line, col()));
                break;



            //whitespace & new lines
            case ' ':
            case '\r':
            case '\t': //we want to ignore whitespace
                break;
            case '\n':
                current_line++;
                line_start_char = current;  //reset line_start_char
                break;
            default:
    if (Character.isDigit(c)) { //check if the character is a digit
        scanNumber(tokens);
    } else
    if (isIdentifierpart(c)) { //check if the character is a valid identifier part
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