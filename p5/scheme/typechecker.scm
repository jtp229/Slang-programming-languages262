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

(define (typecheck-sequence exprs env)
  (if (null? exprs)
      (cons 'RUNTIME #f)
      (let ((result (typecheck-ast (car exprs) env)))
        (if (cdr result)
            result
            (if (null? (cdr exprs))
                result
                (typecheck-sequence (cdr exprs) env))))))

(define (typecheck-lambda-body formals body argument-types env)
  (let ((local (type-env-push env)))
    (let loop ((ids formals) (types argument-types))
      (if (not (null? ids))
          (let ((type (if (null? types) 'RUNTIME (car types))))
            (ast-set-type! (car ids) type)
            (type-env-put! local (vector-ref (car ids) 1) type)
            (loop (cdr ids) (if (null? types) '() (cdr types))))))
    (typecheck-sequence body local)))

(define (type-allowed? actual allowed)
  (or (eq? actual 'RUNTIME) (memq actual allowed)))

(define (warning-result message) (cons 'RUNTIME message))

(define (format-type type) (symbol->string type))

(define (typecheck-builtin-call name signature arguments env)
  (let* ((inputs (vector-ref signature 0))
         (variadic (vector-ref signature 1))
         (required (length inputs))
         (count (length arguments)))
    (if (or (< count required) (and (not variadic) (not (= count required))))
        (warning-result
         (string-append "Warning: Wrong number of arguments for " name
                        ": expected " (number->string required)
                        ", found " (number->string count)))
        (let check-arguments ((args arguments) (index 0) (types '()))
          (if (null? args)
              (let ((output (vector-ref signature 2)))
                (cond
                 ((eq? output 'ARG)
                  (cons (car (reverse types)) #f))
                 ((eq? output 'NUMERIC)
                  (if (memq 'RUNTIME types)
                      (cons 'RUNTIME #f)
                      (if (memq 'DBL types) (cons 'DBL #f) (cons 'INT #f))))
                 (else (cons output #f))))
              (let* ((result (typecheck-ast (car args) env))
                     (actual (car result))
                     (allowed (if (< index required)
                                  (list-ref inputs index)
                                  variadic)))
                (if (cdr result)
                    result
                    (if (not (type-allowed? actual allowed))
                        (warning-result
                         (string-append "Warning: Type mismatch in call to " name
                                        ": argument " (number->string (+ index 1))
                                        " expected " (format-type (car allowed))
                                        ", found " (format-type actual)))
                        (check-arguments (cdr args) (+ index 1)
                                         (cons actual types))))))))))

(define (typecheck-call expr env)
  (let* ((expressions (vector-ref expr 1))
         (operator (car expressions))
         (arguments (cdr expressions)))
    (if (eq? (vector-ref operator 0) 'AST_IDENTIFIER)
        (let ((signature (get-builtin-signature (vector-ref operator 1))))
          (if signature
              (let ((operator-result (typecheck-ast operator env)))
                (if (cdr operator-result)
                    operator-result
                    (typecheck-builtin-call (vector-ref operator 1) signature arguments env)))
              (typecheck-nonbuiltin-call expr operator arguments env)))
        (typecheck-nonbuiltin-call expr operator arguments env))))

(define (typecheck-nonbuiltin-call expr operator arguments env)
  (if (eq? (vector-ref operator 0) 'AST_LAMBDA_DEF)
      (let ((operator-result (typecheck-ast operator env)))
        (if (cdr operator-result)
            operator-result
            (let ((formals (vector-ref operator 1)))
              (if (not (= (length arguments) (length formals)))
                  (warning-result
                   (string-append "Warning: Wrong number of arguments for lambda: expected "
                                  (number->string (length formals))
                                  ", found " (number->string (length arguments))))
                  (let check-arguments ((args arguments) (types '()))
                    (if (null? args)
                        (let ((body-result
                               (typecheck-lambda-body formals (vector-ref operator 2)
                                                      (reverse types) env)))
                          (if (cdr body-result)
                              body-result
                              (begin
                                (ast-set-type! operator 'LAMBDA)
                                body-result)))
                        (let ((result (typecheck-ast (car args) env)))
                          (if (cdr result)
                              result
                              (check-arguments (cdr args) (cons (car result) types))))))))))
      (let ((operator-result (typecheck-ast operator env)))
        (if (cdr operator-result)
            operator-result
            (let ((arguments-result (typecheck-sequence arguments env)))
              (if (cdr arguments-result)
                  arguments-result
                  (cons 'RUNTIME #f)))))))

;;; Find unstable names with the same conservative rules as the Go checker.
(define (unstable-walk! node top-level state)
  (let ((kind (vector-ref node 0)))
    (define (walk-list nodes)
      (for-each (lambda (item) (unstable-walk! item #f state)) nodes))
    (define (record-definition name)
      (if top-level
          (let* ((counts (vector-ref state 2))
                 (entry (assoc name counts)))
            (vector-set! state 2
                         (if entry
                             (cons (cons name (+ 1 (cdr entry)))
                                   (let remove ((rest counts))
                                     (cond ((null? rest) '())
                                           ((string=? (caar rest) name) (cdr rest))
                                           (else (cons (car rest) (remove (cdr rest)))))))
                             (cons (cons name 1) counts))))
          (if (not (member name (vector-ref state 1)))
              (vector-set! state 1 (cons name (vector-ref state 1))))))
    (cond
     ((eq? kind 'AST_CONS)
      (unstable-walk! (vector-ref node 1) #f state)
      (unstable-walk! (vector-ref node 2) #f state))
     ((memq kind '(AST_VEC AST_AND AST_CALL AST_BEGIN AST_COND AST_OR))
      (walk-list (vector-ref node 1)))
     ((eq? kind 'CONDITION)
      (unstable-walk! (vector-ref node 1) #f state)
      (walk-list (vector-ref node 2)))
     ((eq? kind 'AST_DEFINE_VAR)
      (record-definition (vector-ref (vector-ref node 1) 1))
      (unstable-walk! (vector-ref node 2) #f state))
     ((eq? kind 'AST_DEFINE_FUNC)
      (let ((ids (vector-ref node 1)))
        (if (not (null? ids)) (record-definition (vector-ref (car ids) 1))))
      (walk-list (vector-ref node 2)))
     ((eq? kind 'AST_IF)
      (unstable-walk! (vector-ref node 1) #f state)
      (unstable-walk! (vector-ref node 2) #f state)
      (unstable-walk! (vector-ref node 3) #f state))
     ((eq? kind 'AST_LAMBDA_DEF) (walk-list (vector-ref node 2)))
     ((eq? kind 'AST_QUOTE) (unstable-walk! (vector-ref node 1) #f state))
     ((eq? kind 'AST_SET)
      (let ((name (vector-ref (vector-ref node 1) 1)))
        (if (not (member name (vector-ref state 0)))
            (vector-set! state 0 (cons name (vector-ref state 0)))))
      (unstable-walk! (vector-ref node 2) #f state))
     ((eq? kind 'AST_TICK) (unstable-walk! (vector-ref node 1) #f state))
     ((eq? kind 'LET_DEF) (unstable-walk! (vector-ref node 2) #f state))
     ((eq? kind 'AST_LET)
      (walk-list (vector-ref node 1))
      (walk-list (vector-ref node 2)))
     ((eq? kind 'AST_APPLY)
      (unstable-walk! (vector-ref node 1) #f state)
      (unstable-walk! (vector-ref node 2) #f state)))))

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
  (let ((state (vector '() '() '())))
    (for-each (lambda (expr) (unstable-walk! expr #t state)) forest)
    (let ((multiple-defines
           (let collect ((counts (vector-ref state 2)))
             (if (null? counts)
                 '()
                 (if (> (cdar counts) 1)
                     (cons (caar counts) (collect (cdr counts)))
                     (collect (cdr counts)))))))
      (append (vector-ref state 0) (vector-ref state 1) multiple-defines))))

;;; Perform static type checking on an AST node
;;;
;;; @param expr The AST node
;;; @param env  The type environment
;;;
;;; @return a Pair (type . error-message) where error-message is #f if no error
(define (typecheck-ast expr env)
  (let ((kind (vector-ref expr 0)))
    (define (finish type) (ast-set-type! expr type) (cons type #f))
    (define (check-child child)
      (typecheck-ast child env))
    (cond
     ((eq? kind 'AST_INT) (finish 'INT))
     ((eq? kind 'AST_DBL) (finish 'DBL))
     ((memq kind '(AST_BOOL_TRUE AST_BOOL_FALSE)) (finish 'BOOL))
     ((eq? kind 'AST_CHAR) (finish 'CHAR))
     ((eq? kind 'AST_STR) (finish 'STR))
     ((eq? kind 'AST_SYMBOL) (finish 'SYMBOL))
     ((eq? kind 'AST_EMPTY_CONS) (finish 'CONS))
     ((eq? kind 'AST_IDENTIFIER)
      (finish (if (type-env-unstable? env (vector-ref expr 1))
                  'RUNTIME
                  (type-env-get env (vector-ref expr 1)))))
     ((eq? kind 'AST_CONS)
      (let ((first (check-child (vector-ref expr 1))))
        (if (cdr first)
            first
            (let ((second (check-child (vector-ref expr 2))))
              (if (cdr second) second (finish 'CONS))))))
     ((eq? kind 'AST_VEC)
      (let ((result (typecheck-sequence (vector-ref expr 1) env)))
        (if (cdr result) result (finish 'VEC))))
     ((memq kind '(AST_QUOTE AST_TICK)) (finish 'RUNTIME))
     ((memq kind '(AST_AND AST_OR))
      (let ((expressions (vector-ref expr 1)))
        (if (null? expressions)
            (finish 'RUNTIME)
            (let ((result (typecheck-sequence expressions env)))
              (if (cdr result)
                  result
                  (let ((first-type (ast-get-type (car expressions))))
                    (finish (if (let same? ((rest (cdr expressions)))
                                  (or (null? rest)
                                      (and (eq? (ast-get-type (car rest)) first-type)
                                           (same? (cdr rest)))))
                                first-type
                                'RUNTIME))))))))
     ((eq? kind 'AST_BEGIN)
      (let ((result (typecheck-sequence (vector-ref expr 1) env)))
        (if (cdr result) result (finish (car result)))))
     ((eq? kind 'AST_IF)
      (let ((condition (check-child (vector-ref expr 1))))
        (if (cdr condition)
            condition
            (let ((true-result (check-child (vector-ref expr 2))))
              (if (cdr true-result)
                  true-result
                  (let ((false-result (check-child (vector-ref expr 3))))
                    (if (cdr false-result)
                        false-result
                        (let ((true-type (car true-result))
                              (false-type (car false-result)))
                          (if (and (not (eq? true-type 'RUNTIME))
                                   (not (eq? false-type 'RUNTIME))
                                   (not (eq? true-type false-type)))
                              (warning-result
                               (string-append "Warning: Type mismatch in if branches: "
                                              (format-type true-type) " and "
                                              (format-type false-type)))
                              (finish (if (eq? true-type false-type)
                                          true-type
                                          'RUNTIME)))))))))))
     ((eq? kind 'AST_COND)
      (let ((conditions (vector-ref expr 1)))
        (if (or (null? conditions)
                (not (eq? (vector-ref (vector-ref (car (reverse conditions)) 1) 0)
                          'AST_BOOL_TRUE)))
            (warning-result "Warning: cond does not have a final #t case")
            (let loop ((rest conditions) (first-type #f) (index 0))
              (if (null? rest)
                  (finish (if first-type first-type 'RUNTIME))
                  (let* ((condition-node (car rest))
                         (test-result (check-child (vector-ref condition-node 1))))
                    (if (cdr test-result)
                        test-result
                        (let ((branch-result
                               (typecheck-sequence (vector-ref condition-node 2) env)))
                          (if (cdr branch-result)
                              branch-result
                              (let ((branch-type (car branch-result)))
                                (if (and (> index 0)
                                         (not (eq? first-type 'RUNTIME))
                                         (not (eq? branch-type 'RUNTIME))
                                         (not (eq? first-type branch-type)))
                                    (warning-result
                                     (string-append "Warning: Type mismatch in cond branches: "
                                                    (format-type first-type) " and "
                                                    (format-type branch-type)))
                                    ;; The cond's type is always the first branch.
                                    ;; Later RUNTIME branches do not widen it.
                                    (loop (cdr rest)
                                          (if (= index 0) branch-type first-type)
                                          (+ index 1)))))))))))))
     ((eq? kind 'AST_DEFINE_VAR)
      (let ((result (check-child (vector-ref expr 2))))
        (if (cdr result)
            result
            (begin
              (ast-set-type! (vector-ref expr 1) (car result))
              (type-env-put! env (vector-ref (vector-ref expr 1) 1) (car result))
              (finish (car result))))))
     ((eq? kind 'AST_DEFINE_FUNC)
      (let* ((ids (vector-ref expr 1))
             (body (vector-ref expr 2)))
        (if (null? ids)
            (let ((result (typecheck-sequence body env)))
              (if (cdr result) result (finish 'LAMBDA)))
            (begin
              (ast-set-type! (car ids) 'LAMBDA)
              (type-env-put! env (vector-ref (car ids) 1) 'LAMBDA)
              (let ((result (typecheck-lambda-body (cdr ids) body '() env)))
                (if (cdr result) result (finish 'LAMBDA)))))))
     ((eq? kind 'AST_LAMBDA_DEF)
      (let ((result (typecheck-lambda-body (vector-ref expr 1) (vector-ref expr 2) '() env)))
        (if (cdr result) result (finish 'LAMBDA))))
     ((eq? kind 'AST_SET)
      (let ((result (check-child (vector-ref expr 2))))
        (if (cdr result)
            result
            (begin
              (ast-set-type! (vector-ref expr 1) 'RUNTIME)
              (finish (car result))))))
     ((eq? kind 'AST_LET)
      (let ((bindings (vector-ref expr 1)))
        (let check-bindings ((rest bindings) (types '()))
          (if (null? rest)
              (let ((local (type-env-push env)))
                (let bind ((vars bindings) (value-types (reverse types)))
                  (if (not (null? vars))
                      (begin
                        (ast-set-type! (vector-ref (car vars) 1) (car value-types))
                        (type-env-put! local
                                       (vector-ref (vector-ref (car vars) 1) 1)
                                       (car value-types))
                        (bind (cdr vars) (cdr value-types)))))
                (let ((body-result (typecheck-sequence (vector-ref expr 2) local)))
                  (if (cdr body-result) body-result (finish (car body-result)))))
              (let ((value-result (check-child (vector-ref (car rest) 2))))
                (if (cdr value-result)
                    value-result
                    (begin
                      (ast-set-type! (vector-ref (car rest) 1) (car value-result))
                      (check-bindings (cdr rest) (cons (car value-result) types)))))))))
     ((eq? kind 'AST_APPLY)
      (let ((function-result (check-child (vector-ref expr 1))))
        (if (cdr function-result)
            function-result
            (let ((args-result (check-child (vector-ref expr 2))))
              (if (cdr args-result)
                  args-result
                  (if (not (memq (car args-result) '(CONS RUNTIME)))
                      (warning-result
                       (string-append "Warning: Apply expects a list of arguments, found "
                                      (format-type (car args-result))))
                      (finish 'RUNTIME)))))))
     ((eq? kind 'AST_CALL)
      (if (null? (vector-ref expr 1))
          (finish 'RUNTIME)
          (let ((result (typecheck-call expr env)))
            (if (cdr result) result (finish (car result))))))
     (else (finish 'RUNTIME)))))