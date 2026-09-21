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

    /**
     * Format a double value as a string, matching the reference Rust
     * implementation's behavior: whole numbers are printed with no decimal
     * point (e.g. 1.0 -> "1", 3.0 -> "3"), while non-whole numbers are
     * printed using Java's natural minimal decimal representation (e.g.
     * 2.5 -> "2.5"). NaN and Infinity are left as Java's default
     * `Double.toString` representation.
     *
     * @param val The double value to format
     * @return The formatted string
     */
    public static String formatDouble(double val) {
        if (!Double.isNaN(val) && !Double.isInfinite(val) && val == Math.floor(val)) {
            return Long.toString((long) val);
        }
        return Double.toString(val);
    }

    /**
     * Remove escape characters from a string when reading from XML
     * 
     * @param s The string to unescape
     * @return The unescaped string
     */
    public static String unEscape(String s) {
        // We need to go char by char to get it right
        var sb = new StringBuilder();
        boolean in_escape = false;
        for (char c : s.toCharArray()) {
            if (!in_escape) {
                if (c != '\\')
                    sb.append(c);
                else
                    in_escape = true;
            } else {
                if (c == '\\')
                    sb.append("\\");
                else if (c == 't')
                    sb.append("\t");
                else if (c == 'n')
                    sb.append("\n");
                else if (c == '\'')
                    sb.append("'");
                else {
                    System.err.println("Invalid string?!?");
                    System.exit(1);
                }
                in_escape = false;
            }
        }
        return sb.toString();
    }

}
