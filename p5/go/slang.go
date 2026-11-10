package main

import (
	"fmt"
	"os"
)

// Read a file and return its contents as a string
func getFile(filename string) (string, error) {
	code, err := os.ReadFile(filename)
	if err != nil {
		return "", err
	}
	return string(code), nil
}

// Print the help message for the slang interpreter
func printHelp() {
	fmt.Println("slang -- An interpreter for a Scheme-like language (Go version)")
	fmt.Println("  Usage: slang [mode] [filename]")
	fmt.Println("  Modes:")
	fmt.Println("    -help             Display this message and exit")
	fmt.Println("    -typecheck        Typecheck XML AST")
	fmt.Println("    -optimize         Typecheck and optimize XML AST")
}

// The entry point for the slang interpreter
func main() {
	// Ensure exactly one valid mode is given, and exactly one filename
	var filename, mode string
	numModes, numFiles := 0, 0

	for _, a := range os.Args[1:] {
		switch a {
		case "-help", "-typecheck", "-optimize":
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

	forest, err := readNodesFromXml(codeToRun)
	if err != nil {
		fmt.Println(err.Error())
		return
	}
	defaultTypeEnv := makeDefaultTypeEnv()
	unstable := findUnstableNames(forest)

	// Read an XML AST, typecheck it, and print the first type warning (if any)
	if mode == "-typecheck" {
		for _, n := range forest {
			_, err := typecheckAst(n, defaultTypeEnv, unstable)
			if err != nil {
				fmt.Println(err)
				break
			}
		}
	}

	// Read an XML AST, typecheck it, and if there is a warning print the first
	// one.  Otherwise, optimize it and print the optimized AST as XML
	if mode == "-optimize" {
		hadError := false
		for _, n := range forest {
			_, err := typecheckAst(n, defaultTypeEnv, unstable)
			if err != nil {
				fmt.Println(err)
				hadError = true
				break
			}
		}
		if !hadError {
			forest = optimizeForest(forest, unstable)
			xml, err := astToXml(forest)
			if err != nil {
				fmt.Println(err)
			} else {
				fmt.Println(xml)
			}
		}
	}
}
