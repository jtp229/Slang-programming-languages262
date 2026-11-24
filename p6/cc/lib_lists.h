#pragma once

#include <unordered_map>

#include "lib_helpers.h"

/// Populate the provided `map` with a standard set of list functions
///
/// @param map The map to populate with list functions
void makeListFuncs(std::unordered_map<std::string, Value *> &map) {
  using namespace std;
  auto car = unary(
      "car",
      [=](Value *cons) -> variant<Value *, Error> {
        if (cons->type == EMPTY_VAL)
          return Error("Pair expected");
        return static_cast<ConsVal *>(cons)->car;
      },
      CONS_VAL, EMPTY_VAL, "Run-time Error: Argument is not a cons cell");

  auto cdr = unary(
      "cdr",
      [=](Value *cons) -> variant<Value *, Error> {
        if (cons->type == EMPTY_VAL)
          return Error("Pair expected");
        return static_cast<ConsVal *>(cons)->cdr;
      },
      CONS_VAL, EMPTY_VAL, "Run-time Error: Argument is not a cons cell");

  auto cons = uncheckedBinary(
      "cons",
      [=](Value *a1,
          Value *a2) -> variant<Value *, Error> {
        return static_cast<Value *>(ConsVal::makeCons(a1, a2));
      });

  auto list = new BuiltInFuncVal(
      "list",
      [=](vector<Value *> &args)
          -> variant<Value *, Error> {
        return (args.size() == 0)
                   ? static_cast<Value *>(new EmptyConsVal)
                   : static_cast<Value *>(ConsVal::makeConsList(args));
      });

  auto isList = new BuiltInFuncVal(
      "list?",
      [=](vector<Value *> &args)
          -> variant<Value *, Error> {
        if (auto v = ensureArgCountEquals(args, 1, "list?"); v.has_value())
          return v.value();
        auto arg = args[0];
        // NB: '() is a list
        if (arg->type == EMPTY_VAL)
          return static_cast<Value *>(new BoolTrueVal());
        if (arg->type != CONS_VAL)
          return static_cast<Value *>(new BoolFalseVal());
        auto cons = static_cast<ConsVal *>(arg);
        if (cons->cdr->type == EMPTY_VAL || cons->cdr->type == CONS_VAL)
          return static_cast<Value *>(new BoolTrueVal());
        return static_cast<Value *>(new BoolFalseVal());
      });

  auto setCar = new BuiltInFuncVal(
      "set-car!",
      [=](vector<Value *> &args)
          -> variant<Value *, Error> {
        if (auto v = ensureArgCountEquals(args, 2, "set-car!"); v.has_value())
          return v.value();
        auto arg1 = args[0];
        if (arg1->type == CONS_VAL) {
          static_cast<ConsVal *>(arg1)->car = args[1];
          return args[1];
        } else if (arg1->type == EMPTY_VAL) {
          return Error("Cannot call set-car! on an empty cons cell");
        } else {
          return Error("Run-time Error: Argument is not a cons cell");
        }
      });

  auto setCdr = new BuiltInFuncVal(
      "set-cdr!",
      [=](vector<Value *> &args)
          -> variant<Value *, Error> {
        if (auto v = ensureArgCountEquals(args, 2, "set-cdr!"); v.has_value())
          return v.value();
        auto arg1 = args[0];
        if (arg1->type == CONS_VAL) {
          static_cast<ConsVal *>(arg1)->cdr = args[1];
          return args[1];
        } else if (arg1->type == EMPTY_VAL) {
          return Error("Cannot call set-cdr! on an empty cons cell");
        } else {
          return Error("Run-time Error: Argument is not a cons cell");
        }
      });

  auto isNull = new BuiltInFuncVal(
      "null?",
      [=](vector<Value *> &args)
          -> variant<Value *, Error> {
        if (auto v = ensureArgCountEquals(args, 1, "null?"); v.has_value())
          return v.value();
        return (args[0]->type == EMPTY_VAL)
                   ? static_cast<Value *>(new BoolTrueVal())
                   : static_cast<Value *>(new BoolFalseVal());
      });

  for (auto f : {car, cdr, cons, list, isList, isNull, setCar, setCdr})
    map.insert({f->name, f});
}