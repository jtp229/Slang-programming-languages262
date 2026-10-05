import sys
import slang_parser_xml
import slang_evaluator
import slang_env


def getFile(filename):
    """
    Read a file and return its contents as a single string

    @param filename The path to the file

    @return The contents of the file as a string
    """
    try:
        source_file = open(filename, "r")
        code = source_file.read()
        source_file.close()
        return code
    except FileNotFoundError:
        print("Error: file not found", file=sys.stderr)
        return ""


def printHelp():
    """
    Print a help message
    """
    print("slang -- An interpreter for a Scheme-like language (Python version)")
    print("  Usage: slang [mode] [filename]")
    print("    * The named file will be loaded and evaluated")
    print("  Modes:")
    print("    -help             Display this message and exit")
    print("    -interpret        Interpret XML AST")


def main(args):
    """
    Run the Scheme interpreter

    @param args Command-line arguments
    """
    # Parse the command-line arguments.  Make sure exactly one valid mode is
    # given, and exactly one filename
    filename, mode, numModes, numFiles = "", "", 0, 0
    for a in args:
        if a in ["-help", "-interpret"]:
            mode = a
            numModes += 1
        else:
            filename = a
            numFiles += 1
    if numModes != 1 or numFiles != 1 or mode == "-help":
        return printHelp()

    defaultEnv = slang_env.makeDefaultEnv()

    # Load the file and run it.  A run-time error stops the program, after
    # printing the error message.
    forest = slang_parser_xml.readForestFromXml(getFile(filename))
    for n in forest:
        res = slang_evaluator.interpret_ast(n, defaultEnv)
        print(slang_evaluator.val_to_scheme(res))
        if res["type"] == slang_evaluator.VAL_ERROR:
            break


# In python, this is how we get main() to run when we invoke this program via
# `python3 slang.py ...`
if __name__ == "__main__":
    main(sys.argv[1:])
