#pragma once

#include <string>
#include <unordered_map>

#include "lib_lists.h"
#include "lib_math.h"
#include "lib_string.h"
#include "lib_vector.h"
#include "util.h"
#include "values.h"

/// Env is the environment, or scope. It is a mapping from names to IValues, and
/// Envs can be stitched together in a cactus shape.
class Env {
  /// The mapping from names to expressions
  std::unordered_map<std::string, Value *> map;

  /// The enclosing environment
  Env *outer = nullptr;

  /// The default-constructed environment can only be made by calling
  /// makeDefault().
  Env() {}

public:
  /// Add a key/value pair to the Environment
  ///
  /// @param key  The string name
  /// @param expr The expression that serves as the value
  void put(std::string key, Value *expr) {
    map.insert({key, expr});
  }

  /// Get the value for a given key. If the key isn't mapped in this scope,
  /// look in outer scopes.
  ///
  /// @param key The key to look up
  ///
  /// @return The value associated with the key
  std::variant<Value *, Error> get(std::string &key) {
    auto res = map.find(key);
    if (res != map.end())
      return res->second;
    if (!outer)
      return Error("Run-time Error: Invalid identifier");
    return outer->get(key);
  }

  /// Update the value for a given key, in this environment or the nearest
  /// enclosing environment that maps it.
  ///
  /// @param key  The key to update
  /// @param expr The new value for the key
  ///
  /// @return true if the key was found and updated, false otherwise
  bool update(std::string &key, Value *expr) {
    if (map.find(key) != map.end()) {
      map.at(key) = expr;
      return true;
    }
    if (!outer)
      return false;
    return outer->update(key, expr);
  }

  /// Create the default environment
  ///
  /// @return An environment object that is populated with default methods for
  ///         the built-in functions
  static Env *makeDefault() {
    auto e = new Env();
    makeMathFuncs(e->map);
    makeListFuncs(e->map);
    makeStringFuncs(e->map);
    makeVectorFuncs(e->map);
    return static_cast<Env *>(e);
  }

  /// Create a nested (inner) environment, suitable for a lambda
  ///
  /// @param outer The enclosing environment
  ///
  /// @return A new environment that points to `outer` as its parent
  static Env *makeInner(Env *outer) {
    auto e = new Env();
    e->outer = outer;
    return static_cast<Env *>(e);
  }
};