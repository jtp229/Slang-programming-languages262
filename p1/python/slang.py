import sys
import slang_scanner
import slang_scanner_xml


def get_file(filename):
    """Read a file and return its contents as a single string.

    Args:
        filename: The path to the file to read.

    Returns:
        The contents of the file, or an empty string if not found.
    """
    try:
        source_file = open(filename, "r")
        code = source_file.read()
        source_file.close()
        return code
    except FileNotFoundError:
        print("Error: file not found", file=sys.stderr)
        return ""


def print_help():
    """Print a help message"""
    print("slang -- An interpreter for a Scheme-like language (Python version)")
    print("  Usage: slang [mode] [filename]")
    print("    * The named file will be loaded and evaluated")
    print("  Modes:")
    print("    -help             Display this message and exit")
    print("    -scan             Turn slang code into XML tokens")


def main(args):
    """Run the Scheme interpreter.

    Args:
        args: The command-line arguments.
    """
    # Parse the command-line arguments.  Make sure exactly one valid mode is
    # given, and (for every mode but -help) exactly one filename
    filename, mode, num_modes, num_files = "", "", 0, 0
    for a in args:
        if a in ["-help", "-scan"]:
            mode = a
            num_modes += 1
        else:
            filename = a
            num_files += 1
    if num_modes != 1 or (mode == "-help" and num_files != 0) or (mode != "-help" and num_files != 1):
        return print_help()

    code_to_run = get_file(filename)

    if mode == "-scan":
        tokens = slang_scanner.scan_tokens(code_to_run)
        print(slang_scanner_xml.to_xml_string(tokens))


# In python, this is how we get main() to run when we invoke this program via
# `python3 slang.py ...`
if __name__ == "__main__":
    main(sys.argv[1:])
