package cse262.slang.Scanner;

/**
 * One of the most annoying and avoidable bugs in large-ish programs comes from
 * hard-coding strings into a program, and then mis-typing the string in one
 * place or another. To avoid that problem, XmlConstants defines all of the
 * string constants that will be used by XmlTokenWriter.
 */
public class XmlConstants {
    /**
     * StringConstant is a wrapper around a String value. This might seem like
     * overkill, but since we don't just define the constants as Strings,
     * methods in other places (like XmlTokenWriter) can deal with
     * StringConstants instead of Strings, and thus encourage the programmer not
     * to use Strings directly.
     */
    public static class StringConstant {
        /** The string constant being stored in this instance */
        public final String value;

        /**
         * Construct a StringConstant from a String
         * 
         * @param value The String value of the StringConstant
         */
        private StringConstant(String value) {
            this.value = value;
        }
    }

    // The following constants don't really need individual comments. The
    // "attr" constants are the names to use for attributes of Xml tags. The
    // "tag" constants are the names to use for the Xml tags themselves. The
    // "val" constants express common constants, like true and false.

    static final StringConstant attrLine = new StringConstant("line");
    static final StringConstant attrColumn = new StringConstant("col");
    static final StringConstant attrValue = new StringConstant("val");

    static final StringConstant tagRoot = new StringConstant("Tokens");
    static final StringConstant tagAbbrev = new StringConstant("AbbrevToken");
    static final StringConstant tagAnd = new StringConstant("AndToken");
    static final StringConstant tagApply = new StringConstant("ApplyToken");
    static final StringConstant tagBegin = new StringConstant("BeginToken");
    static final StringConstant tagBool = new StringConstant("BoolToken");
    static final StringConstant tagChar = new StringConstant("CharToken");
    static final StringConstant tagCond = new StringConstant("CondToken");
    static final StringConstant tagDbl = new StringConstant("DblToken");
    static final StringConstant tagDefine = new StringConstant("DefineToken");
    static final StringConstant tagDot = new StringConstant("DotToken");
    static final StringConstant tagEof = new StringConstant("EofToken");
    static final StringConstant tagIdentifier = new StringConstant("IdentifierToken");
    static final StringConstant tagIf = new StringConstant("IfToken");
    static final StringConstant tagInt = new StringConstant("IntToken");
    static final StringConstant tagLambda = new StringConstant("LambdaToken");
    static final StringConstant tagLParen = new StringConstant("LeftParenToken");
    static final StringConstant tagLet = new StringConstant("LetToken");
    static final StringConstant tagOr = new StringConstant("OrToken");
    static final StringConstant tagQuote = new StringConstant("QuoteToken");
    static final StringConstant tagRParen = new StringConstant("RightParenToken");
    static final StringConstant tagSet = new StringConstant("SetToken");
    static final StringConstant tagStr = new StringConstant("StrToken");
    static final StringConstant tagVector = new StringConstant("VecToken");

    static final StringConstant valTrue = new StringConstant("true");
    static final StringConstant valFalse = new StringConstant("false");
}
