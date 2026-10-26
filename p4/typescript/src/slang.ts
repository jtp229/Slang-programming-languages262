import { Args, Modes } from './Args';
import { Env } from './Env';
import { XmlParseError, xmlToAst } from './parserXml';
import { interpretAst } from './interpretAst';
import { addStdLib } from './makeStdLib';
import { printValue } from './valuePrinter';
import { InterpretError } from './Values';

import * as fs from 'fs';

/**
 * Create the default environment
 *
 * @return An environment object that is populated with default values and
 *         default methods for the built-in functions
 */
function makeDefaultEnv() {
  let e = new Env();
  addStdLib(e);
  return e;
}

/**
 * The main entry point for the slang interpreter
 */
function main() {
  let parsedArgs = new Args();

  // If help was requested, print help and then exit immediately
  if (parsedArgs.mode == Modes.HELP) {
    parsedArgs.printHelp();
    return;
  }

  // Read the XML AST from the file
  let forest = xmlToAst(fs.readFileSync(parsedArgs.fileName).toString());
  if (forest instanceof XmlParseError) {
    console.log(forest);
    return;
  }

  // Interpret each top-level expression in turn, printing its value
  let env = makeDefaultEnv();
  for (let e of forest) {
    let result = interpretAst(e, env);
    console.log(printValue(result));
    // A run-time error ends the program
    if (result instanceof InterpretError) {
      return;
    }
  }
}

// Invoke main() to start the interpreter
main();
