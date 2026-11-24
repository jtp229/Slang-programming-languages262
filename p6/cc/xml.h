#pragma once

#include <string>
#include <vector>

#include <tinyxml2.h>

#include "ast_nodes.h"
#include "util.h"

const char *TAG_ACTIONS = "Actions";
const char *TAG_AND = "And";
const char *TAG_BEGIN = "Begin";
const char *TAG_BINDING = "Binding";
const char *TAG_BINDINGS = "Bindings";
const char *TAG_BOOL = "Bool";
const char *TAG_CALL = "Call";
const char *TAG_CHAR = "Char";
const char *TAG_COND = "Cond";
const char *TAG_CONDITION = "Condition";
const char *TAG_CONS = "Cons";
const char *TAG_DBL = "Dbl";
const char *TAG_DEFINE_FUNC = "DefineFunc";
const char *TAG_DEFINE_VAR = "DefineVar";
const char *TAG_EMPTY = "Empty";
const char *TAG_EXPRESSIONS = "Expressions";
const char *TAG_FORMALS = "Formals";
const char *TAG_IDENTIFIER = "Identifier";
const char *TAG_IDENTIFIERS = "Identifiers";
const char *TAG_IF = "If";
const char *TAG_INT = "Int";
const char *TAG_LAMBDA = "Lambda";
const char *TAG_LET = "Let";
const char *TAG_OR = "Or";
const char *TAG_QUOTE = "Quote";
const char *TAG_SET = "Set";
const char *TAG_STR = "Str";
const char *TAG_SYMBOL = "Symbol";
const char *TAG_TEST = "Test";
const char *TAG_TICK = "Tick";
const char *TAG_VECTOR = "Vector";

/**
 * Transform an XML node into an ASTNode
 *
 * `node` -- A node from the xml parser.  Note that there isn't much type
 * information here to help us, and we don't have an XML schema, so we're going
 * to have to do some manual error checking.
 */
AstNode *xmlToNode(tinyxml2::XMLElement *node) {
  std::string name = node->Name();
  // TODO: This doesn't do much error checking...
  if (name == TAG_IDENTIFIER) {
    auto val = unescape(node->Attribute("val"));
    return new IdentifierNode(val);
  }
  if (name == TAG_BOOL) {
    if (node->Attribute("val") == std::string("false"))
      return new BoolFalseNode();
    return new BoolTrueNode();
  }
  if (name == TAG_INT) {
    return new IntNode(std::stoi(node->Attribute("val")));
  }
  if (name == TAG_DBL) {
    return new DblNode(std::stod(node->Attribute("val")));
  }
  if (name == TAG_CHAR) {
    return new CharNode(unescape(node->Attribute("val"))[0]);
  }
  if (name == TAG_STR) {
    return new StrNode(unescape(node->Attribute("val")));
  }
  if (name == TAG_SYMBOL) {
    auto val = unescape(node->Attribute("val"));
    return new SymbolNode(val);
  }
  if (name == TAG_DEFINE_VAR) {
    auto c = node->FirstChildElement();
    auto id = static_cast<IdentifierNode *>(xmlToNode(c));
    auto expr = xmlToNode(c->NextSiblingElement());
    return new DefineVarNode(static_cast<IdentifierNode *>(id),
                             static_cast<AstNode *>(expr));
  }
  if (name == TAG_IF) {
    auto c = node->FirstChildElement();
    auto cond = xmlToNode(c);
    c = c->NextSiblingElement();
    auto ifTrue = xmlToNode(c);
    auto ifFalse = xmlToNode(c->NextSiblingElement());
    return new IfNode(static_cast<AstNode *>(cond),
                      static_cast<AstNode *>(ifTrue),
                      static_cast<AstNode *>(ifFalse));
  }
  if (name == TAG_SET) {
    auto c = node->FirstChildElement();
    auto id = static_cast<IdentifierNode *>(xmlToNode(c));
    auto expr = xmlToNode(c->NextSiblingElement());
    return new SetNode(static_cast<IdentifierNode *>(id),
                       static_cast<AstNode *>(expr));
  }
  if (name == TAG_QUOTE) {
    auto datum = static_cast<Datum *>(xmlToNode(node->FirstChildElement()));
    return new QuoteNode(static_cast<Datum *>(datum));
  }
  if (name == TAG_TICK) {
    auto datum = static_cast<Datum *>(xmlToNode(node->FirstChildElement()));
    return new TickNode(static_cast<Datum *>(datum));
  }
  if (name == TAG_AND) {
    std::vector<AstNode *> exprs;
    for (auto c = node->FirstChildElement(); c; c = c->NextSiblingElement()) {
      auto expr = xmlToNode(c);
      exprs.push_back(static_cast<AstNode *>(expr));
    }
    return new AndNode(std::move(exprs));
  }
  if (name == TAG_LAMBDA) {
    std::vector<IdentifierNode *> formals;
    auto f = node->FirstChildElement();
    if (f->Name() != std::string(TAG_FORMALS)) {
      std::terminate();
    }
    for (auto c = f->FirstChildElement(); c; c = c->NextSiblingElement()) {
      auto ff = static_cast<IdentifierNode *>(xmlToNode(c));
      formals.push_back(static_cast<IdentifierNode *>(ff));
    }
    // Now advance past formals to the body (expressions)
    std::vector<AstNode *> body;
    auto e = f->NextSiblingElement();
    if (e->Name() != std::string(TAG_EXPRESSIONS)) {
      std::terminate();
    }
    for (auto c = e->FirstChildElement(); c; c = c->NextSiblingElement()) {
      auto x = xmlToNode(c);
      body.push_back(static_cast<AstNode *>(x));
    }
    return new LambdaDefNode(std::move(formals), std::move(body));
  }
  if (name == TAG_CALL) {
    std::vector<AstNode *> exprs;
    for (auto c = node->FirstChildElement(); c; c = c->NextSiblingElement()) {
      auto e = xmlToNode(c);
      exprs.push_back(static_cast<AstNode *>(e));
    }
    return new CallNode(std::move(exprs));
  }
  if (name == TAG_OR) {
    std::vector<AstNode *> exprs;
    for (auto c = node->FirstChildElement(); c; c = c->NextSiblingElement()) {
      auto expr = xmlToNode(c);
      exprs.push_back(static_cast<AstNode *>(expr));
    }
    return new OrNode(std::move(exprs));
  }
  if (name == TAG_BEGIN) {
    std::vector<AstNode *> exprs;
    for (auto c = node->FirstChildElement(); c; c = c->NextSiblingElement()) {
      auto expr = xmlToNode(c);
      exprs.push_back(static_cast<AstNode *>(expr));
    }
    return new BeginNode(std::move(exprs));
  }
  if (name == TAG_VECTOR) {
    std::vector<Datum *> data;
    for (auto c = node->FirstChildElement(); c; c = c->NextSiblingElement()) {
      auto datum = static_cast<Datum *>(xmlToNode(c));
      data.push_back(static_cast<Datum *>(datum));
    }
    return new VecNode(std::move(data));
  }
  if (name == TAG_COND) {
    std::vector<Condition> conds;
    // Each child node should be a Condition
    for (auto c = node->FirstChildElement(); c; c = c->NextSiblingElement()) {
      if (c->Name() != std::string(TAG_CONDITION)) {
        std::terminate();
      }
      // Get the first child, which should be Test, and get its lone expression
      auto t = c->FirstChildElement();
      if (t->Name() != std::string(TAG_TEST)) {
        std::terminate();
      }
      auto tt = static_cast<AstNode *>(xmlToNode(t->FirstChildElement()));
      auto test = static_cast<AstNode *>(tt);

      // Now advance past Test to the Actions, and put them into an array
      std::vector<AstNode *> actions;
      auto a = t->NextSiblingElement();
      if (a->Name() != std::string(TAG_ACTIONS)) {
        std::terminate();
      }
      for (auto e = a->FirstChildElement(); e; e = e->NextSiblingElement()) {
        auto x = xmlToNode(e);
        actions.push_back(static_cast<AstNode *>(x));
      }
      // Make the Condition
      conds.emplace_back(test, std::move(actions));
    }
    return new CondNode(std::move(conds));
  }
  if (name == TAG_EMPTY) {
    return new EmptyConsNode();
  }
  if (name == TAG_CONS) {
    auto c = node->FirstChildElement();
    auto car = static_cast<Datum *>(xmlToNode(c));
    auto cdr = static_cast<Datum *>(xmlToNode(c->NextSiblingElement()));
    return new ConsNode(static_cast<Datum *>(car),
                        static_cast<Datum *>(cdr));
  }
  if (name == TAG_DEFINE_FUNC) {
    std::vector<IdentifierNode *> ids;
    auto i = node->FirstChildElement();
    if (i->Name() != std::string(TAG_IDENTIFIERS)) {
      std::terminate();
    }
    for (auto c = i->FirstChildElement(); c; c = c->NextSiblingElement()) {
      auto ff = static_cast<IdentifierNode *>(xmlToNode(c));
      ids.push_back(static_cast<IdentifierNode *>(ff));
    }
    // Now advance past formals to the body (expressions)
    std::vector<AstNode *> body;
    auto e = i->NextSiblingElement();
    if (e->Name() != std::string(TAG_EXPRESSIONS)) {
      std::terminate();
    }
    for (auto c = e->FirstChildElement(); c; c = c->NextSiblingElement()) {
      auto x = xmlToNode(c);
      body.push_back(static_cast<AstNode *>(x));
    }
    return new DefineFuncNode(std::move(ids), std::move(body));
  }
  if (name == TAG_LET) {
    std::vector<LetDef> vars;
    auto b = node->FirstChildElement();
    if (b->Name() != std::string(TAG_BINDINGS)) {
      std::terminate();
    }
    for (auto ld = b->FirstChildElement(); ld; ld = ld->NextSiblingElement()) {
      if (ld->Name() != std::string(TAG_BINDING)) {
        std::terminate();
      }
      auto c = ld->FirstChildElement();
      auto id = static_cast<IdentifierNode *>(xmlToNode(c));
      auto val = xmlToNode(c->NextSiblingElement());
      vars.emplace_back(static_cast<IdentifierNode *>(id),
                        static_cast<AstNode *>(val));
    }
    std::vector<AstNode *> body;
    b = b->NextSiblingElement();
    if (b->Name() != std::string(TAG_ACTIONS)) {
      std::terminate();
    }
    for (auto c = b->FirstChildElement(); c; c = c->NextSiblingElement()) {
      auto x = xmlToNode(c);
      body.push_back(static_cast<AstNode *>(x));
    }
    return new LetNode(std::move(vars), std::move(body));
  }
  // unreachable!
  std::terminate();
}

/// Convert an XML string into a forest of AST nodes
///
/// @param xml The XML string to parse
///
/// @return A vector of AstNodes reconstituted from the XML
std::vector<AstNode *> xmlToAst(std::string xml) {
  // Parse the document
  tinyxml2::XMLDocument doc;
  doc.Parse(xml.c_str(), xml.length());
  auto root = doc.RootElement();
  // Each top-level element should become an entry in the forest
  std::vector<AstNode *> forest;
  for (auto *e = root->FirstChildElement(); e; e = e->NextSiblingElement()) {
    forest.push_back(static_cast<AstNode *>(xmlToNode(e)));
  }
  return forest;
}
