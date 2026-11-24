#pragma once

#include <unordered_map>

#include "lib_helpers.h"

/// Populate the provided `map` with a standard set of string functions
void makeStringFuncs(
    std::unordered_map<std::string, Value *> &map) {
  using namespace std;
  // NB: `string-append` is variadic (0 or more STR args), unlike the binary
  // arithmetic ops, so it can't use the `variadic()` helper (which requires
  // at least one argument). It follows the same idiom manually: type-check
  // every argument, then fold them together.
  auto cat = new BuiltInFuncVal(
      "string-append",
      [=](vector<Value *> &args)
          -> variant<Value *, Error> {
        auto name = "string-append";
        if (auto v = ensureAllArgTypesIn(
                args, name, {STR_VAL},
                "Run-time Error: Argument is not a string");
            v.has_value())
          return v.value();
        string res;
        for (auto &a : args)
          res += static_cast<StrVal *>(a)->val;
        return static_cast<Value *>(new StrVal(res));
      });
  auto len = unary(
      "string-length",
      [=](Value *a) -> variant<Value *, Error> {
        return new IntVal(static_cast<StrVal *>(a)->val.length());
      },
      STR_VAL, "Run-time Error: Argument is not a string");

  // NB: The extra validation of arguments we need here necessitates not using a
  // helper.
  auto substr = new BuiltInFuncVal(
      "substring",
      [=](vector<Value *> &args)
          -> variant<Value *, Error> {
        auto name = "substring";
        if (auto v = ensureArgCountEquals(args, 3, name); v.has_value())
          return v.value();
        if (auto v =
                ensureArgTypeIn(args, 0, name, {STR_VAL},
                                "Run-time Error: Argument is not a string");
            v.has_value())
          return v.value();
        if (auto v =
                ensureArgTypeIn(args, 1, name, {INT_VAL},
                                "Run-time Error: Argument is not an integer");
            v.has_value())
          return v.value();
        if (auto v =
                ensureArgTypeIn(args, 2, name, {INT_VAL},
                                "Run-time Error: Argument is not an integer");
            v.has_value())
          return v.value();
        string s = static_cast<StrVal *>(args[0])->val;
        int i1 = static_cast<IntVal *>(args[1])->val;
        int i2 = static_cast<IntVal *>(args[2])->val;
        if (i1 < 0 || i1 > (int)s.length() || i2 < i1 || i2 > (int)s.length())
          return Error("Invalid argument to substring");
        auto res = s.substr(i1, i2 - i1);
        return static_cast<Value *>(new StrVal(res));
      });

  auto isStr = new BuiltInFuncVal(
      "string?",
      [=](vector<Value *> &args)
          -> variant<Value *, Error> {
        auto name = "string?";
        if (auto v = ensureArgCountEquals(args, 1, name); v.has_value())
          return v.value();
        return args[0]->type == STR_VAL
                   ? static_cast<Value *>(new BoolTrueVal())
                   : static_cast<Value *>(new BoolFalseVal());
      });

  for (auto f : {cat, len, substr, isStr})
    map.insert({f->name, f});
}