#pragma once

#include <unordered_map>

#include "lib_helpers.h"

/// Populat e the provided `map` with a standard set of vector functions
///
/// @param map The map to populate with vector functions
void makeVectorFuncs(
    std::unordered_map<std::string, Value *> &map) {
  using namespace std;
  auto len = new BuiltInFuncVal(
      "vector-length",
      [=](vector<Value *> &args)
          -> variant<Value *, Error> {
        auto name = "vector-length";
        if (auto v = ensureArgCountEquals(args, 1, name); v.has_value())
          return v.value();
        if (auto v =
                ensureArgTypeIn(args, 0, name, {VEC_VAL},
                                "Run-time Error: Argument is not a vector");
            v.has_value())
          return v.value();
        return static_cast<Value *>(
            new IntVal(static_cast<VecVal *>(args[0])->items.size()));
      });

  auto get = new BuiltInFuncVal(
      "vector-get",
      [=](vector<Value *> &args)
          -> variant<Value *, Error> {
        auto name = "vector-get";
        if (auto v = ensureArgCountEquals(args, 2, name); v.has_value())
          return v.value();
        if (auto v =
                ensureArgTypeIn(args, 0, name, {VEC_VAL},
                                "Run-time Error: Argument is not a vector");
            v.has_value())
          return v.value();
        if (auto v =
                ensureArgTypeIn(args, 1, name, {INT_VAL},
                                "Run-time Error: Argument is not an integer");
            v.has_value())
          return v.value();
        auto vec = static_cast<VecVal *>(args[0]);
        auto idx = static_cast<IntVal *>(args[1])->val;
        if (idx < 0 || idx >= (int)vec->items.size())
          return Error("Vector index out of bounds");
        return vec->items[idx];
      });

  auto set = new BuiltInFuncVal(
      "vector-set!",
      [=](vector<Value *> &args)
          -> variant<Value *, Error> {
        auto name = "vector-set!";
        if (auto v = ensureArgCountEquals(args, 3, name); v.has_value())
          return v.value();
        if (auto v =
                ensureArgTypeIn(args, 0, name, {VEC_VAL},
                                "Run-time Error: Argument is not a vector");
            v.has_value())
          return v.value();
        if (auto v =
                ensureArgTypeIn(args, 1, name, {INT_VAL},
                                "Run-time Error: Argument is not an integer");
            v.has_value())
          return v.value();
        auto vec = static_cast<VecVal *>(args[0]);
        auto idx = static_cast<IntVal *>(args[1])->val;
        if (idx < 0 || idx >= (int)vec->items.size())
          return Error("Vector index out of bounds");
        vec->items[idx] = args[2];
        return args[2];
      });

  auto makeVec =
      new BuiltInFuncVal("vector",
                         [=](vector<Value *> &args)
                             -> variant<Value *, Error> {
                           return static_cast<Value *>(new VecVal(move(args)));
                         });

  auto isVec = new BuiltInFuncVal(
      "vector?",
      [=](vector<Value *> &args)
          -> variant<Value *, Error> {
        auto name = "vector?";
        if (auto v = ensureArgCountEquals(args, 1, name); v.has_value())
          return v.value();
        return (args[0]->type == VEC_VAL)
                   ? static_cast<Value *>(new BoolTrueVal())
                   : static_cast<Value *>(new BoolFalseVal());
      });

  for (auto f : {len, get, set, makeVec, isVec})
    map.insert({f->name, f});
}