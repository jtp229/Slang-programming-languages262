package main

import (
	"fmt"
	"os"
)

// getFile reads a file and returns its contents as a string
func getFile(filename string) (string, error) {
	code, err := os.ReadFile(filename)
	if err != nil {
		return "", err
	}
	return string(code), nil
}

// printHelp prints the help message for the slang interpreter
func printHelp() {
	fmt.Println("slang -- An interpreter for a Scheme-like language (Go version)")
	fmt.Println("  Usage: slang [mode] [filename]")
	fmt.Println("  Modes:")
	fmt.Println("    -help             Display this message and exit")
	fmt.Println("    -parse            Turn XML tokens into an XML AST")
}

// main is the entry point for the slang interpreter
func main() {
	// Parse the command-line arguments. Make sure exactly one valid mode is
	// given, and exactly one filename
	var filename, mode string
	numModes, numFiles := 0, 0

	for _, a := range os.Args[1:] {
		switch a {
		case "-help", "-parse":
			mode = a
			numModes++
		default:
			filename = a
			numFiles++
		}
	}

	if numModes != 1 || numFiles != 1 || mode == "-help" {
		printHelp()
		return
	}

	codeToRun, err := getFile(filename)
	if err != nil {
		fmt.Println(err.Error())
		return
	}

	// PARSE mode: read XML tokens, turn them into an AST, print the AST as XML
	if mode == "-parse" {
		tokens, err := readTokensFromXml(codeToRun)
		if err != nil {
			fmt.Println(err.Error())
			return
		}
		append_EOF(tokens)
		forest, err := parse_program(&TokenStream{0, tokens})
		if err != nil {
			fmt.Println(err)
		} else {
			xml, err := astToXml(forest)
			if err != nil {
				fmt.Println(err)
			} else {
				fmt.Println(xml)
			}
		}
	}
}
