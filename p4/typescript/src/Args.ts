import 'process';

/**
 * Modes gives a programmatic name to each of the command-line options to the
 * program
 */
export enum Modes { HELP, INTERPRET }

/**
 * Slang is a command-line program, and it can be configured by passing
 * arguments to it on the command line (such as `-help`).  Encapsulating each
 * command-line option in one of these ArgDesc objects makes the code a little
 * less brittle to changes.
 *
 * Note that since the class is not exported, we don't need to worry about
 * public/private or final qualifiers.  This file should know how to use this
 * object.
 */
class ArgDesc {
  /**
   * Construct an ArgDesc
   *
   * @param arg   The text of the command-line argument (e.g., `-help`)
   * @param desc  The text to display in a help message (e.g., `Display a help
   *              message`)
   * @param argId The programmatic identifier for this command line option
   */
  constructor(readonly arg: string, readonly desc: string, readonly argId: Modes) { }
}

/** Args is an object for making sense of the command-line arguments */
export class Args {
  /**
   * options is a fixed array with the printable options for how to use the
   * program
   */
  options = [
    new ArgDesc('-help', 'Display this message and exit', Modes.HELP),
    new ArgDesc('-interpret', 'Interpret XML AST', Modes.INTERPRET)
  ];

  /**
   * Display a help message that describes how to use this program
   */
  printHelp() {
    console.log('slang -- An interpreter for XML ASTs of a Scheme-like language (TypeScript version)');
    console.log('  Usage: node dist/slang.js [mode] [filename]');
    console.log('    * The entire file will be loaded and evaluated');
    console.log('  Modes:');
    const max_len = this.options.reduce((m, o) => Math.max(m, o.arg.length), 0);
    this.options.forEach((o) => console.log(`    ${o.arg.padEnd(max_len, ' ')}  ${o.desc}`));
  }

  /** The selected mode of operation */
  mode: Modes = Modes.HELP;

  /** The (optional) file with which to operate */
  fileName: string = '';

  /**
   * Use the command-line arguments to configure this object so that it
   * describes the desired behavior of the program.
   *
   * Note: If the user does not provide any valid arguments, then the help
   * option will be enabled automatically.
   *
   * @param args The command line arguments to the program.
   */
  constructor() {
    // Temp results while we're evaluating the args
    let mode = Modes.HELP;
    let fileName = '';

    // Counters for making sure the usage is correct
    let numModes = 0,
      numNames = 0;

    // NB: This is O(n^2), but it's OK, since `n` will never be large
    for (let i = 2; i < process.argv.length; ++i) {
      let arg = process.argv[i];
      let found = false;
      for (let o of this.options) {
        if (o.arg === arg) {
          mode = o.argId;
          ++numModes;
          found = true;
          break;
        }
      }
      if (!found) {
        fileName = arg;
        ++numNames;
      }
    }

    // Only move out of help mode if the args were valid
    if (numModes == 1 && numNames == 1) {
      this.mode = mode;
      this.fileName = fileName;
    }
  }
}
