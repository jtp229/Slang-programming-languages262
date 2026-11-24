#pragma once

#include <stdbool.h>
#include <stdlib.h>

/// This file is the interface to the garbage collector.  Every function
/// declared here has a stub in gc.c, which you must implement.  You may add to
/// this interface (or change it) if your design needs something different.

struct Value;
struct AstNode;
struct AstNodeVec;
struct Env;

/// The kinds of objects managed by the GC.
///
/// NB: These are allocated through gc_alloc in alloc_value() (evaluator.c),
///     alloc_astnode() (ast_nodes.h), and make_env() (env.c).
typedef enum {
  GC_OBJ_VALUE,
  GC_OBJ_ASTNODE,
  GC_OBJ_ENV,
} GcObjKind;

/// A suggested header for every GC-managed allocation.  The idea is that
/// gc_alloc allocates room for a header plus the object, and returns a pointer
/// to the bytes just after the header.  Given an object pointer `p`, its header
/// is then at `((struct GcHeader *)p) - 1`.
struct GcHeader {
  /// Intrusive singly-linked list of all GC allocations
  struct GcHeader *next;
  /// Which kind of object follows this header
  GcObjKind kind;
  /// Set during the mark phase; cleared during sweep
  bool marked;
};

/// A shadow-stack frame.  Shadow stack frames are organized in a list.  They
/// hold GC roots, and live on the C stack.  Code that holds a Value* or
/// AstNode* in a local variable, at a time when that object might not be
/// reachable from any other root, should put the address of that variable in a
/// frame, and push the frame before doing anything that could allocate (and
/// thus trigger a collection).
///
/// NB: There are four types of roots.  While we could have a tagged union and
///     only have one in a frame, there are times where we want more than one,
///     so it's easiest to simply have all four types in the frame record,
///     expecting that often most will be null.
struct GcRootFrame {
  /// An array of addresses of stack variables of type Value*
  struct Value ***vals;
  /// Number of vals
  int num_vals;
  /// An array of Value* objects, such as a builtin's args[]
  struct Value **varrs;
  /// The number of varrs
  int num_varrs;
  /// An array of addresses of stack variables of type AstNode*
  struct AstNode ***nodes;
  /// The number of nodes
  int num_nodes;
  /// An array of addresses of stack variables of type Env*
  struct Env ***envs;
  /// The number of envs
  int num_envs;
  /// The next shadow stack frame
  struct GcRootFrame *prev;
};

/// Push a frame onto the shadow stack
///
/// @param frame  The frame to push
void gc_push_roots(struct GcRootFrame *frame);

/// Pop the most recently pushed frame from the shadow stack
///
/// @param frame  The frame to pop (must be the most recently pushed)
void gc_pop_roots(struct GcRootFrame *frame);

/// Allocate a GC-managed block of `size` bytes for an object of the given
/// kind.  Returns a pointer to the usable object body, with all bytes set to
/// zero.  May trigger a collection before allocating.
///
/// @param kind The kind of object (Value, AstNode, or Env)
/// @param size The size of the object
///
/// @return a pointer to the payload of the allocated, gc-managed object
void *gc_alloc(GcObjKind kind, size_t size);

/// Register the global environment, which contains mappings for all built-in
/// functions and all top-level definitions, as a root.  Only allows protecting
/// one environment at a time.
///
/// @param env  The environment to register as a root
void gc_register_env(struct Env *env);

/// Unregister a previously registered environment.
///
/// @param env  The environment to unregister
void gc_unregister_env(struct Env *env);

/// Register a vector of AstNodes (the forest of top-level expressions) as a
/// root, to prevent any nodes in the forest from being collected.  Only allows
/// protecting one forest at a time.
///
/// @param vec  The vector of AstNodes to register as a root
void gc_register_forest(struct AstNodeVec *vec);

/// Unregister a previously registered AstNodeVec root.
///
/// @param vec  The vector of AstNodes to unregister
void gc_unregister_forest(struct AstNodeVec *vec);

/// Prevent collection from happening (e.g., while building the global
/// environment or the AST, before they are registered as roots)
void gc_suspend();

/// Allow collection to happen again (only makes sense when paired with
/// `gc_suspend`)
void gc_resume();

/// Run a full mark-sweep collection.
void gc_collect();
