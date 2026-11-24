#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast_nodes.h"
#include "env.h"
#include "gc.h"
#include "values.h"

// TODO: Implement the garbage collector.  Every function below is a stub.
//
// You will need some state, such as a list of all GC-managed objects, the
// shadow stack, the registered roots, and whatever your policy for deciding
// when to collect requires.  You will also need functions for marking each
// kind of object, and for sweeping.
//
// Remember that some objects own memory that is not managed by the GC (for
// example, a string's characters).  When the sweep reclaims such an object, it
// must free that memory too.

/// Push a frame onto the shadow stack
///
/// @param frame The GC root frame to push onto the shadow stack
void gc_push_roots(struct GcRootFrame *frame) {
  // TODO
}

/// Pop the most recently pushed frame from the shadow stack
///
/// @param frame The GC root frame to pop from the shadow stack
void gc_pop_roots(struct GcRootFrame *frame) {
  // TODO
}

/// Allocate a GC-managed block of `size` bytes for an object of the given
/// kind.  Returns a pointer to the usable object body, with all bytes set to
/// zero.  May trigger a collection before allocating.
///
/// @param kind The kind of object (Value, AstNode, or Env)
/// @param size The size of the object
///
/// @return a pointer to the payload of the allocated, gc-managed object
void *gc_alloc(GcObjKind kind, size_t size) {
  // TODO: This version just allocates the object, and never frees it.
  return calloc(1, size);
}

/// Register the global environment as a root
///
/// @param env  The environment to register as a root
void gc_register_env(struct Env *env) {
  // TODO
}

/// Unregister a previously registered environment
///
/// @param env  The environment to unregister
void gc_unregister_env(struct Env *env) {
  // TODO
}

/// Register a vector of AstNodes (forest) as a root
///
/// @param vec  The vector of AstNodes to register as a root
void gc_register_forest(struct AstNodeVec *vec) {
  // TODO
}

/// Unregister a previously registered AstNodeVec root
///
/// @param vec  The vector of AstNodes to unregister
void gc_unregister_forest(struct AstNodeVec *vec) {
  // TODO
}

/// Prevent collection from happening
void gc_suspend() {
  // TODO
}

/// Allow collection to happen again
void gc_resume() {
  // TODO
}

/// Run a full mark-sweep collection
void gc_collect() {
  // TODO
}
