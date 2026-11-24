#pragma once

#include <format>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "ast_nodes.h"

// Forward declaration of the Environment type, which is mutually dependent on
// the Value type
struct Env;

enum ValueType {
  EMPTY_VAL,        ///<empty cons cell
  TRUE_VAL,         ///<boolean true
  FALSE_VAL,        ///<boolean false
  CHAR_VAL,         ///<character
  DBL_VAL,          ///<double
  INT_VAL,          ///<integer
  STR_VAL,          ///<string
  SYMBOL_VAL,       ///<symbol
  VEC_VAL,          ///<vector
  BUILT_IN_FUNC_VAL,///<built-in function
  CONS_VAL,         ///<cons cell
  LAMBDA_VAL,       ///<lambda
};

/// Value is the base class for all values produced by the interpreter
struct Value {
  ValueType type; // The type of this Value

  /// Construct a Value
  ///
  /// @param type The type of value to construct
  explicit Value(ValueType type) : type(type) {}

  /// Produce a string representation of the value
  virtual std::string to_string() = 0;
};

/// EmptyCons is used to represent an empty Cons cell
struct EmptyConsVal : public Value {
  /// Construct an EmptyConsVal
  EmptyConsVal() : Value(EMPTY_VAL) {}
  /// Produce a string representation of the value
  virtual std::string to_string() { return "()"; }
};

/// BoolTrue is used to represent the value `true`
struct BoolTrueVal : public Value {
  /// Construct a BoolTrueVal
  BoolTrueVal() : Value(TRUE_VAL) {}
  /// Produce a string representation of the value
  virtual std::string to_string() { return "#t"; }
};

/// BoolFalse is used to represent the value `false`
struct BoolFalseVal : public Value {
  /// Construct a BoolFalseVal
  BoolFalseVal() : Value(FALSE_VAL) {}
  /// Produce a string representation of the value
  virtual std::string to_string() { return "#f"; }
};

/// Char is used to represent character values
struct CharVal : public Value {
  const char val; // The character value
  /// Construct a CharVal from a value
  ///
  /// @param v The character value
  CharVal(char v) : Value(CHAR_VAL), val(v) {}
  /// Produce a string representation of the value
  std::string to_string() {
    if (val == ' ')
      return "#\\space";
    else if (val == '\n')
      return "#\\newline";
    else if (val == '\t')
      return "#\\tab";
    std::string res = "#\\";
    res += val;
    return res;
  }
};

/// Dbl is used to represent double values
struct DblVal : public Value {
  const double val; // The double value
  /// Construct a DblVal from a value
  ///
  /// @param v The double value
  DblVal(double v) : Value(DBL_VAL), val(v) {}
  /// Produce a string representation of the value
  std::string to_string() { return std::format("{}", val); }
};

/// Int is used to represent integer values
struct IntVal : public Value {
  const int val; // The int value
  /// Construct an IntVal from a value
  ///
  /// @param v The integer value
  IntVal(int v) : Value(INT_VAL), val(v) {}
  /// Produce a string representation of the value
  std::string to_string() { return std::to_string(val); }
};

/// Str is used to represent String values
struct StrVal : public Value {
  const std::string val; // The string value
  /// Construct a StrVal from a value
  ///
  /// @param v The string value
  StrVal(std::string_view v) : Value(STR_VAL), val(v) {}
  /// Produce a string representation of the value
  std::string to_string() { return val; }
};

/// Symbol represents a symbol value
///
/// NB: Since Scheme is homo-iconic, Symbol and Identifier look identical.
/// However, a Symbol *is* a value, and an Identifier *is not*
struct SymbolVal : public Value {
  const std::string name; // The name associated with this symbol
  /// Construct a SymbolVal from a value
  ///
  /// @param v The name associated with this symbol
  SymbolVal(std::string_view v) : Value(SYMBOL_VAL), name(v) {};
  /// Produce a string representation of the value
  std::string to_string() { return "'" + name; }
};

/// Vec represents a Vector value
struct VecVal : public Value {
  std::vector<Value *> items; // The values in this vector
  /// Construct a VecVal from a vector of values
  ///
  /// @param v The vector of values
  VecVal(std::vector<Value *> &&v) : Value(VEC_VAL), items(v) {}
  /// Produce a string representation of the value
  std::string to_string() {
    std::string res = "#(";
    for (unsigned i = 0; i < items.size(); ++i) {
      if (i > 0)
        res += " ";
      res += items[i]->to_string();
    }
    res += ")";
    return res;
  }
};

/// BuiltInFunc represents a built-in function
struct BuiltInFuncVal : public Value {
  /// The name of the function
  std::string name;
  /// The code to run to execute this function
  std::function<std::variant<Value *, Error>(
      std::vector<Value *> &)>
      func;
  /// Construct a BuiltInFuncVal from a name and a lambda
  ///
  /// @param name The name of the function
  /// @param func The function to execute
  BuiltInFuncVal(std::string_view name,
                 std::function<std::variant<Value *, Error>(
                     std::vector<Value *> &)>
                     func)
      : Value(BUILT_IN_FUNC_VAL), name(name), func(func) {}
  /// Produce a string representation of the value
  std::string to_string() { return "Built-in Function: " + name; }
};

/// Cons represents a cons cell
struct ConsVal : public Value {
  Value *car; // The first value of the pair
  Value *cdr; // The second value of the pair

  /// Create a Cons list from a list of values
  ///
  /// @param items The vector of values
  /// @param start The starting index
  ///
  /// @return A Cons list from a list of values
  static Value *
  makeConsList(std::vector<Value *> &items, unsigned start = 0) {
    if (items.size() <= start)
      return new EmptyConsVal();
    if (items.size() == start + 1)
      return new ConsVal(
          items[start], static_cast<Value *>(new EmptyConsVal()));
    return new ConsVal(items[start], makeConsList(items, start + 1));
  }
  /// Create a Cons cell from two values
  ///
  /// @param car The first value of the pair
  /// @param cdr The second value of the pair
  ///
  /// @return A Cons cell from two values
  static Value *makeCons(Value *car,
                                         Value *cdr) {
    return new ConsVal(car, cdr);
  }
  /// Produce a string representation of the value
  std::string to_string() {
    return "(" + car->to_string() + " . " + cdr->to_string() + ")";
  }

private:
  /// Construct a ConsVal from two values
  ///
  /// @param car The first value of the pair
  /// @param cdr The second value of the pair
  ConsVal(Value *car, Value *cdr)
      : Value(CONS_VAL), car(car), cdr(cdr) {}
};

/// LambdaVal represents a lambda value, which we get by binding a LambdaDef to
/// an environment.
struct LambdaVal : public Value {
  Env *env;                 // The environment
  LambdaDefNode *lambdaDef; // The code of the lambda to execute
  /// Construct a LambdaVal from an environment and a lambda definition
  ///
  /// @param env The environment
  /// @param lambdaDef The lambda definition
  ///
  /// @return A LambdaVal from an environment and a lambda definition
  LambdaVal(Env *env, LambdaDefNode *lambdaDef)
      : Value(LAMBDA_VAL), env(env), lambdaDef(lambdaDef) {}
  /// Produce a string representation of the value
  std::string to_string() { return "Lambda Function"; };
};