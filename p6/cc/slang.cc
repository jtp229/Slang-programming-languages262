#include "args.h"
#include "env.h"
#include "evaluator.h"
#include "util.h"
#include "xml.h"

/// The entry point into our Scheme interpreter
int main(int argc, char **argv) {
  using namespace std;

  Args parsedArgs(argc, argv);

  // If help was requested, print help and then exit immediately
  if (parsedArgs.mode == Args::HELP) {
    parsedArgs.printHelp();
    return 0;
  }

  // Load the code to run
  auto c_e = getFile(parsedArgs.fileName.c_str());
  if (holds_alternative<Error>(c_e)) {
    cout << get<Error>(c_e).msg << endl;
    return -1;
  }
  string codeToRun = get<string>(c_e);

  // Create the environment, so we have it for all evaluating that we do
  auto defaultEnvironment = Env::makeDefault();

  // INTERPRET mode: evaluate each top-level expression in order, and print its
  // value (or error)
  auto forest = xmlToAst(codeToRun);
  for (auto &expr : forest) {
    auto res = evaluate(expr, defaultEnvironment);
    if (holds_alternative<Error>(res))
      cout << get<Error>(res).msg << endl;
    else
      cout << get<Value *>(res)->to_string() << endl;
  }
}
