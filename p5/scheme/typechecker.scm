;;; This file implements the -typecheck mode for slang, per Part 1 of
;;; docs/OPTIMIZE_SPEC.md.  The checker walks each top-level expression and
;;; tags every node with its type (see ast-set-type!).  It works bottom-up: to
;;; check a node, it first checks the node's children, then uses their types
;;; to work out the node's own type.
;;;
;;; Every typecheck function returns a Pair (type . warning), where warning is
;;; #f if there is no warning.  Checking stops at the first warning.
;;;
;;; The optimizer trusts these tags, so a tag must never be wrong.  The one way
;;; a variable's type can change while the program runs is when the variable is
;;; given a new value, by `set!` or by a second `define`.  So, before checking,
;;; find-unstable-names finds every name where that might happen, and every use
;;; of those names has type RUNTIME.

;;; Type Environment structure: (vector 'TYPE_ENV outer bindings)
;;; bindings is an association list of (name . type).  The global (outermost)
;;; environment has one more slot: the program's unstable names (see
;;; find-unstable-names), every use of which has type RUNTIME.

;;; Create a new type environment with built-in constants and functions
;;;
;;; @param unstable The program's unstable names (a list of strings)
;;;
;;; @return a new type environment
(define (make-type-env unstable)
  (let ((env (vector 'TYPE_ENV #f '() unstable)))
    ;; Add constants
    (type-env-put! env "pi" 'DBL)
    (type-env-put! env "e" 'DBL)
    (type-env-put! env "tau" 'DBL)
    (type-env-put! env "inf+" 'DBL)
    (type-env-put! env "inf-" 'DBL)
    (type-env-put! env "nan" 'DBL)

    ;; Add built-ins
    (let ((builtins '("+" "-" "*" "/" "%" ">" "<" ">=" "<=" "=" "abs" "sqrt" "sin" "cos" "tan" "asin" "acos" "atan" "sinh" "cosh" "tanh" "log10" "loge" "pow" "integer?" "double?" "number?" "symbol?" "procedure?" "string?" "char?" "vector?" "list?" "null?" "nan?" "not" "integer->double" "double->integer" "integer->string" "double->string" "car" "cdr" "cons" "list" "set-car!" "set-cdr!" "string-append" "string-length" "substring" "string-char-at" "char-equal?" "char->integer" "integer->char" "char->string" "string->integer" "string->double" "vector" "vector-init" "vector-length" "vector-get" "vector-set!" "display")))
      (for-each (lambda (b) (type-env-put! env b 'LAMBDA)) builtins))
    env))

;;; Push a new scope onto the type environment
;;;
;;; @param env The current type environment
;;;
;;; @return a new type environment with the old one as outer
(define (type-env-push env)
  (vector 'TYPE_ENV env '()))

;;; Put a type into the current scope of the type environment
;;;
;;; @param env  The type environment
;;; @param key  The identifier name
;;; @param type The type symbol
(define (type-env-put! env key type)
  (vector-set! env 2 (cons (cons key type) (vector-ref env 2))))

;;; Get a type from the type environment, searching outer scopes
;;;
;;; @param env The type environment
;;; @param key The identifier name
;;;
;;; @return the type symbol, or 'RUNTIME if not found
(define (type-env-get env key)
  (let ((pair (assoc key (vector-ref env 2))))
    (if pair
        (cdr pair)
        (let ((outer (vector-ref env 1)))
          (if outer
              (type-env-get outer key)
              'RUNTIME)))))

;;; Check whether a name is unstable, by looking in the global environment
;;;
;;; @param env  The type environment
;;; @param name The identifier name
;;;
;;; @return #t if every use of `name` has type RUNTIME
(define (type-env-unstable? env name)
  (let ((outer (vector-ref env 1)))
    (if outer
        (type-env-unstable? outer name)
        (if (member name (vector-ref env 3)) #t #f))))

;;; Get the signature of a built-in function
;;;
;;; @param name The name of the built-in
;;;
;;; @return a vector representing the signature: (vector inputs variadic output),
;;;         where an output of 'ARG means "the type of the argument", and
;;;         an output of 'NUMERIC means "INT or DBL, depending on the
;;;         argument types"
;;;         or #f if not a built-in
(define (get-builtin-signature name)
  (let ((num '(INT DBL RUNTIME))
        (int '(INT RUNTIME))
        (dbl '(DBL RUNTIME))
        (cons-t '(CONS RUNTIME))
        (vec-t '(VEC RUNTIME))
        (str-t '(STR RUNTIME))
        (char-t '(CHAR RUNTIME))
        (any '(BOOL CHAR CONS DBL INT LAMBDA STR SYMBOL VEC RUNTIME)))
    (cond
     ((member name '("+" "-" "*" "/" "%"))
      (vector '() num 'NUMERIC))
     ((member name '(">" "<" ">=" "<=" "="))
      (vector '() num 'BOOL))
     ((string=? name "abs")
      (vector (list num) #f 'NUMERIC))
     ((member name '("sqrt" "sin" "cos" "tan" "asin" "acos" "atan" "sinh" "cosh" "tanh" "log10" "loge"))
      (vector (list num) #f 'DBL))
     ((string=? name "pow")
      (vector (list num num) #f 'DBL))
     ((member name '("integer?" "double?" "number?" "symbol?" "procedure?" "string?" "char?" "vector?" "list?" "null?" "nan?"))
      (vector (list any) #f 'BOOL))
     ((string=? name "not")
      (vector (list any) #f 'BOOL))
     ((string=? name "integer->double")
      (vector (list int) #f 'DBL))
     ((string=? name "double->integer")
      (vector (list dbl) #f 'INT))
     ((string=? name "integer->string")
      (vector (list int) #f 'STR))
     ((string=? name "double->string")
      (vector (list dbl) #f 'STR))
     ((member name '("car" "cdr"))
      (vector (list cons-t) #f 'RUNTIME))
     ((string=? name "cons")
      (vector (list any any) #f 'CONS))
     ((string=? name "list")
      (vector '() any 'CONS))
     ((member name '("set-car!" "set-cdr!"))
      (vector (list cons-t any) #f 'RUNTIME))
     ((string=? name "string-append")
      (vector '() str-t 'STR))
     ((string=? name "string-length")
      (vector (list str-t) #f 'INT))
     ((string=? name "substring")
      (vector (list str-t int int) #f 'STR))
     ((string=? name "string-char-at")
      (vector (list str-t int) #f 'CHAR))
     ((string=? name "char-equal?")
      (vector (list char-t char-t) #f 'BOOL))
     ((string=? name "char->integer")
      (vector (list char-t) #f 'INT))
     ((string=? name "integer->char")
      (vector (list int) #f 'CHAR))
     ((string=? name "char->string")
      (vector (list char-t) #f 'STR))
     ((string=? name "string->integer")
      (vector (list str-t) #f 'INT))
     ((string=? name "string->double")
      (vector (list str-t) #f 'DBL))
     ((string=? name "vector")
      (vector '() any 'VEC))
     ((string=? name "vector-init")
      (vector (list int any) #f 'VEC))
     ((string=? name "vector-length")
      (vector (list vec-t) #f 'INT))
     ((string=? name "vector-get")
      (vector (list vec-t int) #f 'RUNTIME))
     ((string=? name "vector-set!")
      (vector (list vec-t int any) #f 'RUNTIME))
     ((string=? name "display")
      ;; display returns its argument
      (vector (list any) #f 'ARG))
     (#t #f))))

;;; Find the names whose type can't be trusted, because the variable they
;;; name might be given a value of a different type while the program runs.
;;; The optimizer trusts the type checker's types, so a type must never be
;;; wrong.  A name is unstable if:
;;;   - it is the target of a `set!` anywhere in the program,
;;;   - it is defined by a `define` that is not itself a top-level expression.
;;;     In a function body, a `lambda` made before an inner `(define x ...)`
;;;     sees the new `x` when it runs, but the checker checked the `lambda`
;;;     using the old `x`, or
;;;   - it is defined by more than one top-level `define`.
;;;
;;; @param forest The program's top-level expressions
;;;
;;; @return a list of the unstable names (possibly with duplicates)
(define (find-unstable-names forest)
  ;; TODO: implement this
  '())

;;; Perform static type checking on an AST node
;;;
;;; @param expr The AST node
;;; @param env  The type environment
;;;
;;; @return a Pair (type . error-message) where error-message is #f if no error
(define (typecheck-ast expr env)
  ;; TODO: implement this
  (cons 'RUNTIME "typecheck-ast is not implemented"))
