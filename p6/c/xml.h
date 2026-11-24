// You should not need to edit this file

#pragma once

#include "ast_nodes.h"

/**
 * @brief Convert an XML string of an AST back into an AST forest.
 * @param xml The XML string.
 * @return The reconstituted AstForest (arena + roots).
 */
struct AstNodeVec xml_to_ast(const char *xml);
