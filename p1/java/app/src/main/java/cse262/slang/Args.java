package cse262.slang;

import java.util.Arrays;

/** Args is an object for making sense of the command-line arguments. */
public class Args {
    /**
     * ArgType gives a programmatic name to each of the command-line options to
     * the program
     */
    public static enum Modes {
        HELP, SCAN
    }

    /**
     * Our program is a command-line program, and it can be configured by
     * passing arguments to it on the command line (such as `-help`). By
     * declaring each command-line option in one of these ArgDesc objects, we
     * can make our code a little less brittle to changes.
     *
     * Note that since this class is private to Args, we don't need to worry
     * about things like getters. This file should know how to use instances of
     * this object safely and correctly.
     */
    private static class ArgDesc {
        /** The text of the command-line argument (e.g., `-help`) */
        final String arg;

        /**
         * The text to display in a help message (e.g., `Display a help message`
         */
        final String desc;

        /** The programmatic identifier for this option */
        final Modes argId;

        /**
         * Construction an ArgDesc
         * 
         * @param arg   The argument (e.g., `-help`)
         * @param desc  The description of this command line option
         * @param argId A programmatic identifier for this command line option
         */
        ArgDesc(String arg, String desc, Modes argId) {
            this.arg = arg;
            this.desc = desc;
            this.argId = argId;
        }
    }

    /**
     * Options is a fixed array with the printable options for how to use the
     * program
     */
    private static final ArgDesc[] options = {
            new ArgDesc("-help", "Display this message and exit", Modes.HELP),
            new ArgDesc("-scan", "Scan source code, output tokens as XML", Modes.SCAN),
    };

    /** Display a help message that describes how to use this program */
    public static void printHelp() {
        System.out.println("slang -- An interpreter for a scheme-like language");
        System.out.println("  Usage: slang [mode] [filename]");
        System.out.println("    * The named file will be loaded and evaluated");
        System.out.println("  Modes:");
        // Pre-compute the widths of the options, so we can have nice-looking
        // output. We could do this with an explicit `for` loop, but the Java
        // Stream API is more concise. Note that performance doesn't matter
        // here. Also note that we can just get() on the Option returned by
        // `max`, since we know the list is nonempty.
        var max_len = Arrays.asList(options).stream().map(v -> v.arg.length()).max(Integer::compare).get();

        // Now print the options
        for (var o : options) {
            System.out.println(String.format("    %1$-" + max_len + "s  %2$10s", o.arg, o.desc));
        }
    }

    /** The selected mode of operation */
    public final Modes mode;

    /** The file with which to operate (required, except in HELP mode) */
    public final String fileName;

    /**
     * Use the command-line arguments to configure this object so that it
     * describes the desired behavior of the program.
     *
     * Note: If the user does not provide any valid arguments, then the help
     * option will be enabled automatically.
     *
     * @param args The command line arguments to the program.
     */
    public Args(String[] args) {
        // Temp results while we're evaluating the args
        Modes mode = Modes.HELP;
        String fileName = "";
        // Counters for making sure the usage is correct
        int numModes = 0, numNames = 0;

        // NB: This is O(n^2), but it's OK, since `n` will never be large
        outer: for (var arg : args) {
            for (var o : options) {
                if (arg.equals(o.arg)) {
                    mode = o.argId;
                    ++numModes;
                    continue outer;
                }
            }
            fileName = arg;
            ++numNames;
        }

        // Force us into help mode if the args were not valid. Every mode
        // other than HELP requires exactly one filename; HELP requires none.
        boolean validArgs = numModes == 1 && (mode == Modes.HELP ? numNames == 0 : numNames == 1);
        if (!validArgs) {
            this.fileName = "";
            this.mode = Modes.HELP;
        } else {
            this.mode = mode;
            this.fileName = fileName;
        }
    }
}
