// You should not need to edit this file

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xml.h"

/// Remove escape characters from a string when reading from XML
///
/// @param s The string to unescape
///
/// @return A new string that unescapes `s`
static char *xml_unescape(const char *s) {
  if (!s)
    return strdup(""); // new string!
  // NB: it's safe to over-allocate, so just use the string size even though
  // that's an upper bound
  int len = strlen(s);
  char *res = malloc(len + 1);
  int j = 0;
  for (int i = 0; i < len; ++i) {
    if (s[i] == '&') {
      if (strncmp(s + i, "&lt;", 4) == 0) {
        res[j++] = '<';
        i += 3;
      } else if (strncmp(s + i, "&gt;", 4) == 0) {
        res[j++] = '>';
        i += 3;
      } else if (strncmp(s + i, "&amp;", 5) == 0) {
        res[j++] = '&';
        i += 4;
      } else if (strncmp(s + i, "&quot;", 6) == 0) {
        res[j++] = '"';
        i += 5;
      } else {
        abort();
      }
    } else if (s[i] == '\\') {
      if (strncmp(s + i, "\\'", 2) == 0) {
        res[j++] = '\'';
        i += 1;
      } else if (strncmp(s + i, "\\\\", 2) == 0) {
        res[j++] = '\\';
        i += 1;
      } else if (strncmp(s + i, "\\t", 2) == 0) {
        res[j++] = '\t';
        i += 1;
      } else if (strncmp(s + i, "\\n", 2) == 0) {
        res[j++] = '\n';
        i += 1;
      } else {
        abort();
      }
    } else {
      res[j++] = s[i];
    }
  }
  res[j] = '\0';
  return res;
}

/// An XML "parser"
typedef struct {
  /// The string of XML-formatted text to parse
  const char *xml;

  /// The current position in the string
  int pos;
} XmlParser;

/// Advance forward until the next non-whitespace is reached
///
/// @param p The XML parser
static void skip_whitespace(XmlParser *p) {
  while (p->xml[p->pos] && (p->xml[p->pos] == ' ' || p->xml[p->pos] == '\n' ||
                            p->xml[p->pos] == '\r' || p->xml[p->pos] == '\t'))
    p->pos++;
}

/// Report if the next set of characters matches the given string.  Also
/// advances past that string if there is a match.
///
/// @param p The XML parser
/// @param s The string to match
///
/// @return True on a match, false otherwise
static bool consume_on_match(XmlParser *p, const char *s) {
  size_t len = strlen(s);
  if (strncmp(p->xml + p->pos, s, len) == 0) {
    p->pos += len;
    return true;
  }
  return false;
}

/// Lightweight function for extracting an attribute's value from an XML
/// attribute (`a="b"`)
///
/// @param tag  The tag from which to find/extract an attribute
/// @param attr The attribute to find ane extract
///
/// @return The value of the attribute.  Aborts if not found
static char *get_attr(const char *tag, const char *attr) {
  char search[64]; // NB: Assumes attribute names aren't too large
  snprintf(search, sizeof(search), "%s=\"", attr);
  const char *p = strstr(tag, search);
  if (!p) {
    abort();
  }
  p += strlen(search);
  const char *end = strchr(p, '"');
  if (!end) {
    abort();
  }
  return strndup(p, end - p);
}

/// Transform an XML fragment into an AstNode.  Recursively calls itself to
/// handle nested children.
///
/// @param p  The XML parser object, with a pointer to the next token
///
/// @return The index of the node that was created
static struct AstNode *xml_to_node(XmlParser *p) {
  skip_whitespace(p);
  if (p->xml[p->pos] != '<')
    abort();
  const char *tag_start = p->xml + p->pos;
  const char *tag_end = strchr(tag_start, '>');
  if (!tag_end)
    abort();
  int tag_len = tag_end - tag_start + 1;
  char *tag = malloc(tag_len + 1);
  strncpy(tag, tag_start, tag_len);
  tag[tag_len] = '\0';
  p->pos += tag_len;

  struct AstNode *res = NULL;

  if (strstr(tag, "<Identifier")) {
    char *val = get_attr(tag, "val");
    char *un = xml_unescape(val);
    res = alloc_astnode(IDENTIFIER_NODE);
    res->data.identifier_node = un;
    free(val);
  } else if (strstr(tag, "<Bool")) {
    char *val = get_attr(tag, "val");
    res = make_bool_node(val && strcmp(val, "true") == 0);
    free(val);
  } else if (strstr(tag, "<Int")) {
    char *val = get_attr(tag, "val");
    res = make_int_node(atoi(val));
    free(val);
  } else if (strstr(tag, "<Dbl")) {
    char *val = get_attr(tag, "val");
    res = make_dbl_node(atof(val));
    free(val);
  } else if (strstr(tag, "<Str")) {
    char *val = get_attr(tag, "val");
    char *un = xml_unescape(val);
    res = make_str_node(un);
    free(val);
    free(un);
  } else if (strstr(tag, "<Symbol")) {
    char *val = get_attr(tag, "val");
    char *un = xml_unescape(val);
    res = make_symbol_node(un);
    free(val);
    free(un);
  } else if (strstr(tag, "<Char")) {
    char *val = get_attr(tag, "val");
    char *un = xml_unescape(val);
    res = make_char_node(un[0]);
    free(val);
    free(un);
  } else if (strstr(tag, "<Vector")) {
    struct AstNodeVec elements = {.capacity = 8,
                                  .count = 0,
                                  .elements =
                                      malloc(8 * sizeof(struct AstNode *))};
    if (!strstr(tag, "/>")) {
      while (true) {
        skip_whitespace(p);
        if (consume_on_match(p, "</Vector>"))
          break;
        AstNodeVec_push(&elements, xml_to_node(p));
      }
    }
    res = alloc_astnode(VEC_DATUM);
    res->data.vec_node = elements;
  } else if (strstr(tag, "<Empty")) {
    res = make_empty_cons_node();
  } else if (strstr(tag, "<Call")) {
    struct AstNodeVec exprs = {.capacity = 8,
                               .count = 0,
                               .elements =
                                   malloc(8 * sizeof(struct AstNode *))};
    while (true) {
      skip_whitespace(p);
      if (consume_on_match(p, "</Call>"))
        break;
      struct AstNode *child = xml_to_node(p);
      AstNodeVec_push(&exprs, child);
    }
    res = alloc_astnode(CALL_NODE);
    res->data.call_node = exprs;
  } else if (strstr(tag, "<And")) {
    struct AstNodeVec exprs = {.capacity = 8,
                               .count = 0,
                               .elements =
                                   malloc(8 * sizeof(struct AstNode *))};
    while (true) {
      skip_whitespace(p);
      if (consume_on_match(p, "</And>"))
        break;
      AstNodeVec_push(&exprs, xml_to_node(p));
    }
    res = alloc_astnode(AND_NODE);
    res->data.and_node = exprs;
  } else if (strstr(tag, "<Or")) {
    struct AstNodeVec exprs = {.capacity = 8,
                               .count = 0,
                               .elements =
                                   malloc(8 * sizeof(struct AstNode *))};
    while (true) {
      skip_whitespace(p);
      if (consume_on_match(p, "</Or>"))
        break;
      AstNodeVec_push(&exprs, xml_to_node(p));
    }
    res = alloc_astnode(OR_NODE);
    res->data.or_node = exprs;
  } else if (strstr(tag, "<Begin")) {
    struct AstNodeVec exprs_begin = {.capacity = 8,
                                     .count = 0,
                                     .elements =
                                         malloc(8 * sizeof(struct AstNode *))};
    while (true) {
      skip_whitespace(p);
      if (consume_on_match(p, "</Begin>"))
        break;
      AstNodeVec_push(&exprs_begin, xml_to_node(p));
    }
    res = alloc_astnode(BEGIN_NODE);
    res->data.begin_node = exprs_begin;
  } else if (strstr(tag, "<If")) {
    struct AstNode *cond = xml_to_node(p);
    struct AstNode *if_true = xml_to_node(p);
    struct AstNode *if_false = xml_to_node(p);
    skip_whitespace(p);
    consume_on_match(p, "</If>");
    res = alloc_astnode(IF_NODE);
    res->data.if_node.cond = cond;
    res->data.if_node.if_true = if_true;
    res->data.if_node.if_false = if_false;
  } else if (strstr(tag, "<DefineVar")) {
    struct AstNode *id = xml_to_node(p);
    struct AstNode *expr = xml_to_node(p);
    skip_whitespace(p);
    consume_on_match(p, "</DefineVar>");
    res = alloc_astnode(DEFINE_VAR_NODE);
    res->data.define_var_node.id = id;
    res->data.define_var_node.expr = expr;
  } else if (strstr(tag, "<DefineFunc")) {
    struct AstNodeVec ids = {.capacity = 8,
                             .count = 0,
                             .elements = malloc(8 * sizeof(struct AstNode *))};
    skip_whitespace(p);
    if (!consume_on_match(p, "<Identifiers/>")) {
      consume_on_match(p, "<Identifiers>");
      while (true) {
        skip_whitespace(p);
        if (consume_on_match(p, "</Identifiers>"))
          break;
        AstNodeVec_push(&ids, xml_to_node(p));
      }
    }
    struct AstNodeVec body = {.capacity = 8,
                              .count = 0,
                              .elements = malloc(8 * sizeof(struct AstNode *))};
    skip_whitespace(p);
    if (!consume_on_match(p, "<Expressions/>")) {
      consume_on_match(p, "<Expressions>");
      while (true) {
        skip_whitespace(p);
        if (consume_on_match(p, "</Expressions>"))
          break;
        AstNodeVec_push(&body, xml_to_node(p));
      }
    }
    skip_whitespace(p);
    consume_on_match(p, "</DefineFunc>");
    res = alloc_astnode(DEFINE_FUNC_NODE);
    res->data.define_func_node.ids = ids;
    res->data.define_func_node.body = body;
  } else if (strstr(tag, "<Lambda")) {
    struct AstNodeVec formals = {.capacity = 8,
                                 .count = 0,
                                 .elements =
                                     malloc(8 * sizeof(struct AstNode *))};
    skip_whitespace(p);
    if (!consume_on_match(p, "<Formals/>")) {
      consume_on_match(p, "<Formals>");
      while (true) {
        skip_whitespace(p);
        if (consume_on_match(p, "</Formals>"))
          break;
        AstNodeVec_push(&formals, xml_to_node(p));
      }
    }
    struct AstNodeVec lbody = {.capacity = 8,
                               .count = 0,
                               .elements =
                                   malloc(8 * sizeof(struct AstNode *))};
    skip_whitespace(p);
    if (!consume_on_match(p, "<Expressions/>")) {
      consume_on_match(p, "<Expressions>");
      while (true) {
        skip_whitespace(p);
        if (consume_on_match(p, "</Expressions>"))
          break;
        AstNodeVec_push(&lbody, xml_to_node(p));
      }
    }
    skip_whitespace(p);
    consume_on_match(p, "</Lambda>");
    res = alloc_astnode(LAMBDA_DEF_NODE);
    res->data.lambda_def_node.formals = formals;
    res->data.lambda_def_node.body = lbody;
  } else if (strstr(tag, "<Quote")) {
    struct AstNode *datum = xml_to_node(p);
    skip_whitespace(p);
    consume_on_match(p, "</Quote>");
    res = alloc_astnode(QUOTE_NODE);
    res->data.quote_node = datum;
  } else if (strstr(tag, "<Tick")) {
    struct AstNode *datum = xml_to_node(p);
    skip_whitespace(p);
    consume_on_match(p, "</Tick>");
    res = alloc_astnode(TICK_NODE);
    res->data.tick_node = datum;
  } else if (strstr(tag, "<Cons")) {
    struct AstNode *car = xml_to_node(p);
    struct AstNode *cdr = xml_to_node(p);
    skip_whitespace(p);
    consume_on_match(p, "</Cons>");
    res = make_cons_node(car, cdr);
  } else if (strstr(tag, "<Set")) {
    struct AstNode *id = xml_to_node(p);
    struct AstNode *expr = xml_to_node(p);
    skip_whitespace(p);
    consume_on_match(p, "</Set>");
    res = alloc_astnode(SET_NODE);
    res->data.set_node.id = id;
    res->data.set_node.expr = expr;
  } else if (strstr(tag, "<Let")) {
    struct AstNodeVec vars = {.capacity = 8,
                              .count = 0,
                              .elements = malloc(8 * sizeof(struct AstNode *))};
    skip_whitespace(p);
    if (!consume_on_match(p, "<Bindings/>")) {
      consume_on_match(p, "<Bindings>");
      while (true) {
        skip_whitespace(p);
        if (consume_on_match(p, "</Bindings>"))
          break;
        AstNodeVec_push(&vars, xml_to_node(p));
      }
    }
    struct AstNodeVec body = {.capacity = 8,
                              .count = 0,
                              .elements = malloc(8 * sizeof(struct AstNode *))};
    skip_whitespace(p);
    if (!consume_on_match(p, "<Actions/>")) {
      consume_on_match(p, "<Actions>");
      while (true) {
        skip_whitespace(p);
        if (consume_on_match(p, "</Actions>"))
          break;
        AstNodeVec_push(&body, xml_to_node(p));
      }
    }
    skip_whitespace(p);
    consume_on_match(p, "</Let>");
    res = alloc_astnode(LET_NODE);
    res->data.let_node.vars = vars;
    res->data.let_node.body = body;
  } else if (strstr(tag, "<Condition")) {
    skip_whitespace(p);
    consume_on_match(p, "<Test>");
    struct AstNode *test = xml_to_node(p);
    skip_whitespace(p);
    consume_on_match(p, "</Test>");
    struct AstNodeVec exprs = {.capacity = 8,
                               .count = 0,
                               .elements =
                                   malloc(8 * sizeof(struct AstNode *))};
    skip_whitespace(p);
    if (!consume_on_match(p, "<Actions/>")) {
      consume_on_match(p, "<Actions>");
      while (true) {
        skip_whitespace(p);
        if (consume_on_match(p, "</Actions>"))
          break;
        AstNodeVec_push(&exprs, xml_to_node(p));
      }
    }
    skip_whitespace(p);
    consume_on_match(p, "</Condition>");
    res = alloc_astnode(CONDITION_OBJ);
    res->data.condition_obj.test = test;
    res->data.condition_obj.exprs = exprs;
  } else if (strstr(tag, "<Cond")) {
    struct AstNodeVec conditions = {.capacity = 8,
                                    .count = 0,
                                    .elements =
                                        malloc(8 * sizeof(struct AstNode *))};
    while (true) {
      skip_whitespace(p);
      if (consume_on_match(p, "</Cond>"))
        break;
      AstNodeVec_push(&conditions, xml_to_node(p));
    }
    res = alloc_astnode(COND_NODE);
    res->data.cond_node = conditions;
  } else if (strstr(tag, "<Binding")) {
    struct AstNode *id = xml_to_node(p);
    struct AstNode *val = xml_to_node(p);
    skip_whitespace(p);
    consume_on_match(p, "</Binding>");
    res = alloc_astnode(LET_DEF_OBJ);
    res->data.let_def_obj.id = id;
    res->data.let_def_obj.val = val;
  } else {
    abort();
  }

  free(tag);
  return res;
}

/// Convert an XML string back into an AstForest.
///
/// @param xml The XML string
///
/// @return The reconstituted AstForest (arena + roots)
struct AstNodeVec xml_to_ast(const char *xml) {
  struct AstNodeVec forest = {.capacity = 64,
                              .count = 0,
                              .elements = malloc(64 * sizeof(struct AstNode))};

  XmlParser p = {xml, 0};
  skip_whitespace(&p);
  if (!consume_on_match(&p, "<Ast xmlns=\"ast\">"))
    return forest;

  while (true) {
    skip_whitespace(&p);
    if (consume_on_match(&p, "</Ast>"))
      break;
    struct AstNode *node = xml_to_node(&p);
    if (node < 0)
      break;
    if (forest.count == forest.capacity) {
      forest.capacity *= 2;
      forest.elements =
          realloc(forest.elements, forest.capacity * sizeof(struct AstNode *));
    }
    forest.elements[forest.count++] = node;
  }

  return forest;
}
