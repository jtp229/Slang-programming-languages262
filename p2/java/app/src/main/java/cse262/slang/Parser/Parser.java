package cse262.slang.Parser;

import java.util.List;
import java.util.ArrayList;
import cse262.slang.Scanner.Tokens;
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
    private TokenStream stream;

    
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
        this.stream = tokens;
        List<AstNodes.AstNode> results = new ArrayList<>();

        while(hasNext() && !isEOF()){
            results.add(parseExpression());
        }
        return results;

    }
    private AstNodes.AstNode parseExpression() throws ParseError{
        if (peek(Tokens.Abbrev.class)){
            stream.popAny();
            AstNodes.Datum datum = parseDatum();
            return new AstNodes.Tick(datum);
        }
        if(isConstant() || peek(Tokens.Identifier.class)){
            return parseConstantOrIdentifier();
        }
        if(!peek(Tokens.LeftParen.class)) {
            throw new ParseError(stream);
        }
        stream.popAny(); // consumelparen

    }

    private AstNodes.AstNode parseConstantOrIdentifier() throws ParseError {
        Tokens.Token t = stream.nextToken();

        if(t instanceof Tokens.Identifier){
            stream.popAny();
            return new AstNodes.Identifier(((Tokens.Identifier) t).tokenText);

        }
        else if(t instanceof Tokens.Int){
            stream.popAny();
            return new AstNodes.Int(((Tokens.Int) t).value);
        }
        else if (t instanceof Tokens.Dbl){
            stream.popAny();
            return new AstNodes.Dbl(((Tokens.Dbl) t).value);
        }
        else if (t instanceof Tokens.Bool){
            stream.popAny();
            return ((Tokens.Bool) t).value ? new AstNodes.BoolTrue() : new AstNodes.BoolFalse();
        } 
        else if (t instanceof Tokens.Str){
            stream.popAny();
            return new AstNodes.Str(((Tokens.Str) t).value);

        }
        else if (t instanceof Tokens.Char){
            stream.popAny();
            return new AstNodes.Char(((Tokens.Char) t).value);
        }
        throw new ParseError("Invalid constant");
    }
    private AstNodes.AstNode parseDefine() throws ParseError{
        stream.popToken(Tokens.Define.class);
        //check if function is shorthand or simple define
        if (peek(Tokens.LeftParen.class)){
            //Functino shorthand: (DEFINE ( name args .. . .)body .. .)
            stream.popAny(); //consume Lparen
            List<AstNodes.Identifier> names = new ArrayList<>();
        
        //parse function name and arguments
        while (!peek(Tokens.RightParen.class)){
            Tokens.Token t = stream.nextToken();
            if (!(t instanceof Tokens.Identifier)){
                throw new ParseError("Expected identifier in define");
            }
            stream.popAny();
            names.add(new AstNodes.Identifier(((Tokens.Identifier) t).tokenText));

        }
        stream.popToken(Tokens.RightParen.class);

        //parse body expressions
        List<AstNodes.AstNode> body = new ArrayList<>();
        while (!peek(Tokens.RightParen.class)) {
            body.add(parseExpression());
        }

        stream.popToken(Tokens.RightParen.class);
        return new AstNodes.DefineFunc(names, body);
    }else{
        //Simple define: DEFINE name value
        Tokens.Token name = stream.nextToken();
        if (!(name instanceof Tokens.Identifier)){
            throw new ParseError("Expected identifier in define");
        }
        stream.popAny();
        AstNodes.AstNode value = parseExpression();
        stream.popToken(Tokens.RightParen.class);
        return new AstNodes.DefineVar(new AstNodes.Identifier(((Tokens.Identifier) name).tokenText), value);

    }
    }
    //time to parse specific expressions

    //IF <expression> <expression> <expression>

    private AstNodes.AstNode parseIf() throws ParseError {
        stream.popToken(Tokens.If.class);
        AstNodes.AstNode test = parseExpression();
        AstNodes.AstNode consequent = parseExpression();
        AstNodes.AstNode alternate = parseExpression();
        stream.popToken(Tokens.RightParen.class);
        return new AstNodes.If(test, consequent, alternate);
    }
    //set <identifier> <expression>
    private AstNodes.AstNode parseSet() throws ParseError{
        stream.popToken(Tokens.Set.class);
        Tokens.Token name = stream.nextToken();
        if (!(name instanceof Tokens.Identifier)){
            throw new ParseError("Expected identifier in set!");
        }
        stream.popAny();
        AstNodes.AstNode value = parseExpression();
        stream.popToken(Tokens.RightParen.class);
        return new AstNodes.Set(new AstNodes.Identifier(((Tokens.Identifier) name).tokenText), value);

    }
    //and expression
    private AstNodes.AstNode parseAnd() throws ParseError {
        stream.popToken(Tokens.And.class);
        List<AstNodes.AstNode> exprs = new ArrayList<>();
        while(!peek(Tokens.RightParen.class)){
            exprs.add(parseExpression());

        }
        stream.popToken(Tokens.RightParen.class);
        return new AstNodes.And(exprs);
    }

    //or expression



    //begin expression





    private AstNodes.AstNode parseDatum() throws ParseError {
        if(isConstant()) {
            return parseConstantOrIdentifier();
        }
    }
    private boolean isConstant() {
        return peek(Tokens.Int.class) || peek(Tokens.Dbl.class) || peek(Tokens.Bool.class) || peek(Tokens.Str.class) || peek(Tokens.Char.class);

    }






    private boolean peek(Class<?> type){
        return hasNext() && type.isInstance(stream);
    }



    private boolean isEOF(){
        return peek(Tokens.Eof.class);
    }

    private boolean hasNext(){
        return stream.hasNext();
    }
}
