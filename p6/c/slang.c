#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "env.h"
#include "evaluator.h"
#include "values.h"
#include "xml.h"

/// An enum to represent the different modes in which the program can run
typedef enum {
  HELP,
  INTERPRET,
} Mode;

/// A struct storing the result of parsing the arguments
typedef struct {
  Mode mode;
  char *fileName;
} Args;

/// Print a help message
void print_help() {
  printf("slang -- An interpreter for a Scheme-like language (C version)\n");
  printf("  Usage: slang [mode] [filename]\n");
  printf("  Modes:\n");
  printf("    -help              Display this message and exit\n");
  printf("    -interpret         Interpret XML AST\n");
}

/// Parse the command-line arguments
///
/// @param argc The number of arguments
/// @param argv The argument vector
///
/// @return An Args object representing the requested arguments.  If the request
///         is invalid, the returned Args ask for help.
Args parse_args(int argc, char **argv) {
  Args args = {HELP, NULL};
  int modes = 0, files = 0;
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-help") == 0) {
      args.mode = HELP;
      ++modes;
    } else if (strcmp(argv[i], "-interpret") == 0) {
      args.mode = INTERPRET;
      ++modes;
    } else {
      args.fileName = argv[i];
      ++files;
    }
  }
  if (modes != 1 || (args.mode != HELP && files != 1))
    args.mode = HELP;
  return args;
}

/// Read a file and return its contents as a string
///
/// @param path The path to the file
///
/// @return The file contents as a string, or NULL on error
char *read_file(const char *path) {
  FILE *f = fopen(path, "rb");
  if (!f)
    return NULL;
  fseek(f, 0, SEEK_END);
  long len = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *data = malloc(len + 1);
  if (fread(data, 1, len, f) != (size_t)len) {
    free(data);
    fclose(f);
    return NULL;
  }
  data[len] = '\0';
  fclose(f);
  return data;
}

/// The main routine of slang
int main(int argc, char **argv) {
  // Parse the arguments; print help and exit on error
  Args args = parse_args(argc, argv);
  if (args.mode == HELP) {
    print_help();
    return 0;
  }

  // Read the code from the file
  char *code = read_file(args.fileName);
  if (!code) {
    fprintf(stderr, "Could not read file\n");
    return 1;
  }

  // Make the global environment
  struct Env *global_env = make_default_env();

  // Load the AST
  struct AstNodeVec forest = xml_to_ast(code);
  free(code);

  // Evaluate each top-level expression in order, and print its value (or
  // error)
  for (int i = 0; i < forest.count; ++i) {
    struct Value *res = evaluate(forest.elements[i], global_env);
    if (EVAL_ERROR_MSG) {
      printf("%s\n", EVAL_ERROR_MSG);
      free(EVAL_ERROR_MSG);
      EVAL_ERROR_MSG = NULL;
    } else {
      char *s = value_to_string(res);
      printf("%s\n", s);
      fflush(stdout);
      free(s);
    }
  }
  free(forest.elements);
}
