package cse262.slang;

import java.nio.file.Files;
import java.nio.file.Path;

import cse262.slang.Parser.Parser;
import cse262.slang.Parser.XmlNodeWriter;
import cse262.slang.Parser.INodeVisitor.INodeVisitorError;
import cse262.slang.Parser.Parser.ParseError;
import cse262.slang.Scanner.XmlTokenReader;

/**
 * Slang implements an interpreter for a Scheme-like language
 *
 * NB: This is phase 2, so there's just a parser. Its input is the XML token
 * stream produced by a scanner, and its output is an XML AST.
 */
public class Slang {
    /**
     * Read the entire contents of a file as a String. On error, a message
     * will be printed, and an empty string will be returned.
     *
     * @param fileName The name of the file to open
     *
     * @return An empty string (`""`) on any error. Otherwise, the contents of
     *         the file
     */
    private static String getFile(String fileName) {
        try {
            Path path = Path.of(fileName);
            return Files.readString(path);
        } catch (Exception ex) {
            System.err.println("Error: Unable to open " + fileName);
            return "";
        }
    }

    public static void main(String[] args) {
        // Get the command-line arguments. If help is requested or needed,
        // print help and exit immediately.
        var parsedArgs = new Args(args);
        if (parsedArgs.mode == Args.Modes.HELP) {
            Args.printHelp();
            return;
        }

        // Load the input file
        String codeToRun = getFile(parsedArgs.fileName);

        // PARSE mode: read an XML file of tokens, turn it into an AST, print
        // the AST as XML.
        if (parsedArgs.mode == Args.Modes.PARSE) {
            try {
                var reader = new XmlTokenReader();
                var tokens = reader.readTokensFromXml(codeToRun);
                var ast = new Parser().parse(tokens);
                var writer = new XmlNodeWriter();
                writer.astToXml(ast, System.out);
            } catch (ParseError pe) {
                System.out.println(pe.getMessage());
            } catch (INodeVisitorError ie) {
                ie.printStackTrace();
            }
        }
    }
}
