package cse262.slang;

import java.nio.file.Files;
import java.nio.file.Path;

import cse262.slang.Scanner.Scanner;
import cse262.slang.Scanner.XmlTokenWriter;
import cse262.slang.Scanner.Scanner.ScanError;

/**
 * Slang implements an interpreter for a Scheme-like language
 *
 * NB: This is phase 1, so there's just a scanner
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

        // Load the source file
        String codeToRun = getFile(parsedArgs.fileName);

        // SCAN mode: read the source code, turn it into tokens, print the
        // tokens as XML.
        if (parsedArgs.mode == Args.Modes.SCAN) {
            try {
                var tokens = new Scanner().scanTokens(codeToRun);
                var writer = new XmlTokenWriter();
                writer.writeXmlToStream(tokens, System.out);
            } catch (ScanError se) {
                System.out.println(se.getMessage());
            }
        }
    }
}
