#pragma once

#include <string>
#include <vector>

#include "ast_nodes.h"
#include "env.h"

/// Interpret an AstNode in an environment to produce a value
///
/// @param expr The root Abstract Syntax Tree (AST) node to evaluate
/// @param env  The environment in which to evaluate `expr`
///
/// @return A variant containing either the successfully evaluated Value or an
///         Error
std::variant<Value *, Error>
evaluate(AstNode *expr, Env *env) {
  using namespace std;

  if (expr->type == IDENTIFIER_NODE) {
    auto n = static_cast<IdentifierNode *>(expr);
    return env->get(n->id);
  } else if (expr->type == DEFINE_VAR_NODE) {
    auto n = static_cast<DefineVarNode *>(expr);
    auto val = evaluate(n->expr, env);
    if (holds_alternative<Error>(val))
      return val;
    env->put(n->id->id, get<Value *>(val));
    return val;
  } else if (expr->type == SET_NODE) {
    auto n = static_cast<SetNode *>(expr);
    auto val = evaluate(n->expr, env);
    if (holds_alternative<Error>(val))
      return val;
    if (!env->update(n->id->id, get<Value *>(val)))
      return Error("Cannot set unbound variable");
    return val;
  } else if (expr->type == BOOL_FALSE_NODE) {
    return new BoolFalseVal();
  } else if (expr->type == BOOL_TRUE_NODE) {
    return new BoolTrueVal();
  } else if (expr->type == INT_NODE) {
    return new IntVal(static_cast<IntNode *>(expr)->val);
  } else if (expr->type == DBL_NODE) {
    return new DblVal(static_cast<DblNode *>(expr)->val);
  } else if (expr->type == IF_NODE) {
    auto n = static_cast<IfNode *>(expr);
    auto arm = evaluate(n->cond, env);
    if (holds_alternative<Error>(arm))
      return arm;
    if (get<Value *>(arm)->type == FALSE_VAL)
      return evaluate(n->if_false, env);
    return evaluate(n->if_true, env);
  } else if (expr->type == AND_NODE) {
    auto n = static_cast<AndNode *>(expr);
    for (size_t i = 0; i + 1 < n->exprs.size(); ++i) {
      auto res = evaluate(n->exprs[i], env);
      if (holds_alternative<Error>(res) ||
          get<Value *>(res)->type == FALSE_VAL)
        return res;
    }
    return evaluate(n->exprs.back(), env);
  } else if (expr->type == OR_NODE) {
    auto n = static_cast<OrNode *>(expr);
    for (size_t i = 0; i + 1 < n->exprs.size(); ++i) {
      auto res = evaluate(n->exprs[i], env);
      if (holds_alternative<Error>(res) ||
          get<Value *>(res)->type != FALSE_VAL)
        return res;
    }
    return evaluate(n->exprs.back(), env);
  } else if (expr->type == BEGIN_NODE) {
    auto n = static_cast<BeginNode *>(expr);
    for (size_t i = 0; i + 1 < n->exprs.size(); ++i) {
      auto res = evaluate(n->exprs[i], env);
      if (holds_alternative<Error>(res))
        return res;
    }
    return evaluate(n->exprs.back(), env);
  } else if (expr->type == CHAR_NODE) {
    return new CharVal(static_cast<CharNode *>(expr)->val);
  } else if (expr->type == CONS_NODE) {
    auto n = static_cast<ConsNode *>(expr);
    auto car = evaluate(n->car, env);
    if (holds_alternative<Error>(car))
      return car;
    auto cdr = evaluate(n->cdr, env);
    if (holds_alternative<Error>(cdr))
      return cdr;
    return static_cast<Value *>(ConsVal::makeCons(get<Value *>(car),
                                               get<Value *>(cdr)));
  } else if (expr->type == EMPTY_CONS_NODE) {
    return new EmptyConsVal();
  } else if (expr->type == STR_NODE) {
    return new StrVal(static_cast<StrNode *>(expr)->val);
  } else if (expr->type == SYMBOL_NODE) {
    return new SymbolVal(static_cast<SymbolNode *>(expr)->name);
  } else if (expr->type == VEC_NODE) {
    auto n = static_cast<VecNode *>(expr);
    vector<Value *> items;
    for (auto &i : n->items) {
      auto item = evaluate(i, env);
      if (holds_alternative<Error>(item))
        return item;
      items.push_back(get<Value *>(item));
    }
    return new VecVal(move(items));
  } else if (expr->type == CALL_NODE) {
    auto n = static_cast<CallNode *>(expr);

    auto ff = evaluate(n->exprs[0], env);
    if (holds_alternative<Error>(ff))
      return ff;
    auto func = get<Value *>(ff);
    if (func->type != BUILT_IN_FUNC_VAL && func->type != LAMBDA_VAL)
      return Error("Operator is not a procedure");

    vector<Value *> args;
    for (unsigned i = 1; i < n->exprs.size(); ++i) {
      auto v = evaluate(n->exprs[i], env);
      if (holds_alternative<Error>(v))
        return v;
      args.push_back(get<Value *>(v));
    }

    if (func->type == BUILT_IN_FUNC_VAL)
      return static_cast<BuiltInFuncVal *>(func)->func(args);

    // Lambdas require us to put the args into the lambda scope
    auto lambda = static_cast<LambdaVal *>(func);
    if (args.size() != lambda->lambdaDef->formals.size())
      return Error("Wrong number of arguments to lambda: expect " +
                   to_string(lambda->lambdaDef->formals.size()));

    auto inner_env = Env::makeInner(lambda->env);
    for (unsigned i = 0; i < args.size(); ++i)
      inner_env->put(lambda->lambdaDef->formals[i]->id, args[i]);

    // Evaluate the lambda body, and return the value of the last expression
    for (size_t i = 0; i + 1 < lambda->lambdaDef->body.size(); ++i) {
      auto res = evaluate(lambda->lambdaDef->body[i], inner_env);
      if (holds_alternative<Error>(res))
        return res;
    }
    return evaluate(lambda->lambdaDef->body.back(), inner_env);
  } else if (expr->type == COND_NODE) {
    auto n = static_cast<CondNode *>(expr);
    for (auto cond : n->conditions) {
      auto res = evaluate(cond.test, env);
      if (holds_alternative<Error>(res))
        return res;

      if (get<Value *>(res)->type != FALSE_VAL) {
        for (size_t i = 0; i + 1 < cond.exprs.size(); ++i) {
          auto r = evaluate(cond.exprs[i], env);
          if (holds_alternative<Error>(r))
            return r;
        }
        return evaluate(cond.exprs.back(), env);
      }
    }
    return Error("All cond arms evaluated false");
  } else if (expr->type == DEFINE_FUNC_NODE) {
    auto n = static_cast<DefineFuncNode *>(expr);
    std::vector<IdentifierNode *> args(n->ids.begin() + 1,
                                                      n->ids.end());
    auto name = n->ids[0];
    auto body = n->body;
    LambdaDefNode *def(new LambdaDefNode(move(args), move(body)));
    Value *func(new LambdaVal(env, def));
    env->put(name->id, func);
    return func;
  } else if (expr->type == LAMBDA_DEF_NODE) {
    auto n = static_cast<LambdaDefNode *>(expr);
    return new LambdaVal(Env::makeInner(env), n);
  } else if (expr->type == QUOTE_NODE) {
    return evaluate(static_cast<QuoteNode *>(expr)->datum, env);
  } else if (expr->type == TICK_NODE) {
    return evaluate(static_cast<TickNode *>(expr)->datum, env);
  } else if (expr->type == LET_NODE) {
    auto n = static_cast<LetNode *>(expr);
    auto inner = Env::makeInner(env);
    for (auto &b : n->vars) {
      auto d = evaluate(b.val, env);
      if (holds_alternative<Error>(d))
        return d;
      inner->put(b.id->id, get<Value *>(d));
    }
    for (size_t i = 0; i + 1 < n->body.size(); ++i) {
      auto res = evaluate(n->body[i], inner);
      if (holds_alternative<Error>(res))
        return res;
    }
    return evaluate(n->body.back(), inner);
  }
  terminate(); // This should be unreachable!
}
