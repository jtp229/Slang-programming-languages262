#pragma once

#include <cmath>
#include <functional>
#include <unordered_map>
#include <vector>

#include "lib_helpers.h"

/// A helper function for turning a basic arithmetic operation into something
/// that operates with Node*s and preserves the input type for the return value
///
/// @param f The mathematical operation to wrap
///
/// @return A function that takes a vector of Values and returns a Value or an
///         Error
std::function<std::variant<Value *, Error>(
    std::vector<Value *> &)>
makeCheckedVarNtoN(std::function<double(double, double)> f) {
  return [=](std::vector<Value *> &args)
             -> std::variant<Value *, Error> {
    auto anyDoubles = checkAnyArgMatches(DBL_VAL, args);
    if (!anyDoubles) {
      int res = static_cast<IntVal *>(args[0])->val;
      for (size_t i = 1; i < args.size(); ++i)
        res = (int)f(res, static_cast<IntVal *>(args[i])->val);
      return new IntVal(res);
    }
    double res = (args[0]->type == INT_VAL)
                     ? static_cast<IntVal *>(args[0])->val
                     : static_cast<DblVal *>(args[0])->val;
    for (size_t i = 1; i < args.size(); ++i)
      res = f(res, (args[i]->type == INT_VAL)
                       ? static_cast<IntVal *>(args[i])->val
                       : static_cast<DblVal *>(args[i])->val);
    return new DblVal(res);
  };
}

/// A helper function for turning a numerical comparison into something that
/// operates with Node*s and returns a Node*
///
/// @param f The comparison operation to wrap
///
/// @return A function that takes a vector of Values and returns a boolean Value
///         or an Error
std::function<std::variant<Value *, Error>(
    std::vector<Value *> &)>
makeCheckedVarNtoB(std::function<double(double, double)> f) {
  return [=](std::vector<Value *> &args)
             -> std::variant<Value *, Error> {
    auto first = args[0];
    for (size_t i = 1; i < args.size(); ++i) {
      auto next = args[i];
      if (!f((first->type == INT_VAL) ? static_cast<IntVal *>(first)->val
                                      : static_cast<DblVal *>(first)->val,
             (next->type == INT_VAL) ? static_cast<IntVal *>(next)->val
                                     : static_cast<DblVal *>(next)->val))
        return static_cast<Value *>(new BoolFalseVal());
    }
    return new BoolTrueVal;
  };
}

/// Populate the provided `map` with a standard set of mathematical functions
///
/// @param map The map to populate with mathematical functions
void makeMathFuncs(
    std::unordered_map<std::string, Value *> &map) {
  using namespace std;
  // Basic arithmetic operations: add, subtract, multiply, divide, mod
  auto add = variadic(
      "+", makeCheckedVarNtoN([=](double a, double b) { return a + b; }),
      INT_VAL, DBL_VAL, "Run-time Error: Argument is not a number");
  auto subtract = variadic(
      "-", makeCheckedVarNtoN([=](double a, double b) { return a - b; }),
      INT_VAL, DBL_VAL, "Run-time Error: Argument is not a number");
  auto multiply = variadic(
      "*", makeCheckedVarNtoN([=](double a, double b) { return a * b; }),
      INT_VAL, DBL_VAL, "Run-time Error: Argument is not a number");
  auto divide = variadic(
      "/", makeCheckedVarNtoN([=](double a, double b) { return a / b; }),
      INT_VAL, DBL_VAL, "Run-time Error: Argument is not a number");
  auto mod = variadic(
      "%",
      makeCheckedVarNtoN([=](double a, double b) { return (int)a % (int)b; }),
      INT_VAL, DBL_VAL, "Run-time Error: Argument is not a number");
  for (auto f : {add, subtract, multiply, divide, mod})
    map.insert({f->name, f});

  // Basic numerical comparisons: ==, >, >=, <, <=
  auto eq = variadic(
      "=", makeCheckedVarNtoB([=](double a, double b) { return a == b; }),
      INT_VAL, DBL_VAL, "Run-time Error: Argument is not a number");
  auto lt = variadic(
      "<", makeCheckedVarNtoB([=](double a, double b) { return a < b; }),
      INT_VAL, DBL_VAL, "Run-time Error: Argument is not a number");
  auto le = variadic(
      "<=", makeCheckedVarNtoB([=](double a, double b) { return a <= b; }),
      INT_VAL, DBL_VAL, "Run-time Error: Argument is not a number");
  auto gt = variadic(
      ">", makeCheckedVarNtoB([=](double a, double b) { return a > b; }),
      INT_VAL, DBL_VAL, "Run-time Error: Argument is not a number");
  auto ge = variadic(
      ">=", makeCheckedVarNtoB([=](double a, double b) { return a >= b; }),
      INT_VAL, DBL_VAL, "Run-time Error: Argument is not a number");
  for (auto f : {eq, gt, ge, lt, le})
    map.insert({f->name, f});

  // Unary operators: abs
  auto abs = unary(
      "abs",
      [=](Value *a) -> Value * {
        if (a->type == INT_VAL)
          return new IntVal(std::abs(static_cast<IntVal *>(a)->val));
        return new DblVal(std::abs(static_cast<DblVal *>(a)->val));
      },
      INT_VAL, DBL_VAL, "Run-time Error: Argument is not a number");
  for (auto f : {abs})
    map.insert({f->name, f});

  auto _not = new BuiltInFuncVal(
      "not",
      [=](vector<Value *> &args)
          -> variant<Value *, Error> {
        auto name = "not";
        if (auto v = ensureArgCountEquals(args, 1, name); v.has_value())
          return v.value();
        return (args[0]->type == FALSE_VAL)
                   ? static_cast<Value *>(new BoolTrueVal())
                   : static_cast<Value *>(new BoolFalseVal());
      });

  auto display = new BuiltInFuncVal(
      "display",
      [=](vector<Value *> &args)
          -> variant<Value *, Error> {
        auto name = "display";
        if (auto v = ensureArgCountEquals(args, 1, name); v.has_value())
          return v.value();
        cout << args[0]->to_string();
        return args[0];
      });

  for (auto f : {_not, display})
    map.insert({f->name, f});

}