// You should not need to edit this file

#pragma once

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

/// Args is an object for making sense of the command-line arguments
struct Args {
  /// Modes gives a programmatic name to each of the command-line options to the
  /// program
  enum Modes {
    HELP,
    INTERPRET,
  };

  /// Our program is a command-line program, and it can be configured by passing
  /// arguments to it on the command line (such as `-help`). By encapsulating
  /// each command-line option in one of these ArgDesc objects, we can make our
  /// code a little less brittle to changes.
  ///
  /// Note that since the class is private to Args, we don't need to worry about
  /// public/private or final qualifiers. This file should know how to use this
  /// object.
  struct ArgDesc {
    /// The text of the command-line argument (e.g., `-help`)
    std::string arg;

    /// The text to display in a help message (e.g., `Display a help message`)
    std::string desc;

    /// The programmatic identifier for this option
    Modes argId;

    /// Construction an ArgDesc
    ///
    /// @param arg   The argument (e.g., `-help`)
    /// @param desc  The description of this command line option
    /// @param argId A programmatic identifier for this command line option
    ArgDesc(std::string arg, std::string desc, Modes argId)
        : arg(arg), desc(desc), argId(argId) {}
  };

  /// options is a fixed array with the printable options for how to use the
  /// program
  std::vector<ArgDesc> options = {
      ArgDesc("-help", "Display this message and exit", HELP),
      ArgDesc("-interpret", "Interpret XML AST", INTERPRET),
  };

  /// Display a help message that describes how to use this program
  void printHelp() {
    using namespace std;
    cout << "slang -- An interpreter for a Scheme-like language (C++ version)"
         << endl
         << "  Usage: slang [mode] [filename]" << endl
         << "  Modes:" << endl;
    size_t max_len = 0;
    for (auto o : options)
      max_len = max(max_len, o.arg.length());
    for (auto o : options)
      cout << "    " << setw(max_len) << left << o.arg << "  " << o.desc
           << endl;
  }

  /// The selected mode of operation
  Modes mode;

  /// The (optional) file with which to operate
  std::string fileName;

  /// Use the command-line arguments to configure this object so that it
  /// describes the desired behavior of the program.
  ///
  /// Note: If the user does not provide any valid arguments, then the help
  /// option will be enabled automatically.
  ///
  /// @param argc The number of command-line arguments provided
  /// @param argv An array of strings representing the command-line arguments
  Args(int argc, char **argv) {
    // Temp results while we're evaluating the args
    auto mode = HELP;
    std::string fileName = "";
    // Counters for making sure the usage is correct
    int numModes = 0, numNames = 0;

    // NB: This is O(n^2), but it's OK, since `n` will never be large
    for (int i = 1; i < argc; ++i) {
      bool found = false;
      for (auto o : options) {
        if (o.arg == argv[i]) {
          mode = o.argId;
          ++numModes;
          found = true;
          break;
        }
      }
      if (!found) {
        fileName = argv[i];
        ++numNames;
      }
    }

    // Force us into help mode if the args were not valid
    if (numModes != 1 || (mode != HELP && numNames != 1)) {
      this->fileName = "";
      this->mode = HELP;
    } else {
      this->mode = mode;
      this->fileName = fileName;
    }
  }
};