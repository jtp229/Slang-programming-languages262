package cse262.slang.Scanner;

/** A wrapper for some helpful XML-related string processing functions */
public class XmlHelpers {
    /**
     * Escape a string for outputting it to XML as an attribute
     * 
     * @param s The string to escape
     * @return The escaped string
     */
    public static String escape(String s) {
        var sb = new StringBuilder();
        for (char c : s.toCharArray()) {
            if (c == '\\')
                sb.append("\\\\");
            else if (c == '\t')
                sb.append("\\t");
            else if (c == '\n')
                sb.append("\\n");
            else if (c == '\'')
                sb.append("\\'");
            else
                sb.append(c);
        }
        return sb.toString();
    }

}
