#pragma once

#include <initializer_list>
#include <optional>
#include <string>
#include <vector>

#include "util.h"
#include "values.h"

/// Ensure that the function's nth argument is one of the given types
///
/// @param args    The vector of argument values being checked
/// @param n       The zero-based index of the argument to check
/// @param name    The name of the function (used for error reporting)
/// @param classes A list of acceptable ValueTypes
/// @param errmsg  The error message to return if the check fails
///
/// @return An Error if the type check fails, else None[[nodiscard]]
static std::optional<Error>
ensureArgTypeIn(std::vector<Value *> &args, int n,
                std::string_view name, std::initializer_list<ValueType> classes,
                std::string_view errmsg) {
  for (auto t : classes)
    if (args[n]->type == t)
      return {};
  return Error(std::string(errmsg));
}

/// Ensure that all args to the function are one of the given types
///
/// @param args   The vector of argument values being checked
/// @param name   The name of the function (used for error reporting)
/// @param T      A list of of acceptable ValueTypes
/// @param errmsg The error message to return if the check fails
///
/// @return An Error if the type check fails, else None
[[nodiscard]]
static std::optional<Error>
ensureAllArgTypesIn(std::vector<Value *> &args,
                    std::string_view name, std::initializer_list<ValueType> T,
                    std::string_view errmsg) {
  for (size_t i = 0; i < args.size(); ++i)
    if (auto v = ensureArgTypeIn(args, i, name, T, errmsg); v.has_value())
      return v;
  return {};
}

/// Ensure that the provided function has been given at least `min` arguments
///
/// @param args The vector of argument values being checked
/// @param min  The minimum number of arguments that can be accepted
/// @param name The name of the function (used for error reporting)
///
/// @return An error if the check fails, else None
[[nodiscard]]
static std::optional<Error>
ensureArgCountAtLeast(std::vector<Value *> &args, size_t min,
                      std::string_view name) {
  if (args.size() < min)
    return Error(std::string("Function requires at least " +
                             std::to_string(min) + " argument(s)"));
  return {};
}

/// Ensure that the provided function has been given exactly `min` arguments
///
/// @param args The vector of argument values being checked
/// @param num  The number of arguments that must be present
/// @param name The name of the function (used for error reporting)
///
/// @return An error if the check fails, else None
[[nodiscard]]
static std::optional<Error>
ensureArgCountEquals(std::vector<Value *> &args, size_t num,
                     std::string_view name) {
  if (args.size() != num)
    return Error(std::string("Run-time Error: Wrong number of arguments"));
  return {};
}

/// Check if any arg in `args` is of type T
///
/// @param T    The ValueType to check for
/// @param args The vector of argument values to check
///
/// @return True if at least one argument matches the type, else false
[[nodiscard]]
static bool checkAnyArgMatches(ValueType T,
                               std::vector<Value *> &args) {
  for (auto a : args)
    if (a->type == T)
      return true;
  return false;
}

/// Produce a BuiltInFunc that takes exactly one argument
///
/// @param name   The name of the function
/// @param op     The operation to perform
/// @param T      The expected type of the argument
/// @param errmsg The error message to return if the type check fails
///
/// @return A new BuiltInFuncVal that takes exactly one argument
[[nodiscard]]
static BuiltInFuncVal *
unary(std::string name,
      std::function<std::variant<Value *, Error>(
          Value *)> op,
      ValueType T, std::string_view errmsg) {
  return new BuiltInFuncVal(
      name,
      [=](std::vector<Value *> &args)
          -> std::variant<Value *, Error> {
        if (auto v = ensureArgCountEquals(args, 1, name); v.has_value())
          return v.value();
        if (auto v = ensureAllArgTypesIn(args, name, {T}, errmsg);
            v.has_value())
          return v.value();
        return op(args[0]);
      });
}

/// Produce a BuiltInFunc that takes exactly one argument
///
/// @param name   The name of the function
/// @param op     The operation to perform
/// @param T1     The first expected type of the argument
/// @param T2     The second expected type of the argument
/// @param errmsg The error message to return if the type check fails
///
/// @return A new BuiltInFuncVal that takes exactly one argument
[[nodiscard]]
static BuiltInFuncVal *
unary(std::string name,
      std::function<std::variant<Value *, Error>(
          Value *)> op,
      ValueType T1, ValueType T2, std::string_view errmsg) {
  return new BuiltInFuncVal(
      name,
      [=](std::vector<Value *> &args)
          -> std::variant<Value *, Error> {
        if (auto v = ensureArgCountEquals(args, 1, name); v.has_value())
          return v.value();
        if (auto v = ensureAllArgTypesIn(args, name, {T1, T2}, errmsg);
            v.has_value())
          return v.value();
        return op(args[0]);
      });
}

/// Produce a BuiltInFunc that takes two arguments, and doesn't type check
///
/// @param name The name of the function
/// @param op   The operation to perform
///
/// @return A new BuiltInFuncVal that takes two arguments
[[nodiscard]]
static BuiltInFuncVal *
uncheckedBinary(std::string name,
                std::function<std::variant<Value *, Error>(
                    Value *, Value *)> op) {
  return new BuiltInFuncVal(
      name,
      [=](std::vector<Value *> &args)
          -> std::variant<Value *, Error> {
        if (auto v = ensureArgCountEquals(args, 2, name); v.has_value())
          return v.value();
        return op(args[0], args[1]);
      });
}

/// Produce a BuiltInFunc that takes variable arguments
///
/// @param name   The name of the function
/// @param op     The operation to perform
/// @param C1     The first expected type of the argument
/// @param C2     The second expected type of the argument
/// @param errmsg The error message to return if the type check fails
///
/// @return A new BuiltInFuncVal that takes variable arguments
[[nodiscard]]
static BuiltInFuncVal *
variadic(std::string name,
         std::function<std::variant<Value *, Error>(
             std::vector<Value *> &)> op,
         ValueType C1, ValueType C2, std::string_view errmsg) {
  return new BuiltInFuncVal(
      name,
      [=](std::vector<Value *> &args)
          -> std::variant<Value *, Error> {
        if (auto v = ensureArgCountAtLeast(args, 1, name); v.has_value())
          return v.value();
        if (auto v = ensureAllArgTypesIn(args, name, {C1, C2}, errmsg);
            v.has_value())
          return v.value();
        return op(args);
      });
}