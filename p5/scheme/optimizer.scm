;;; The Optimize pass for Slang.
;;;
;;; Implements the four passes described in OPTIMIZE_SPEC.md:
;;;   1. Constant Folding
;;;   2. Constant Propagation
;;;   3. Strength Reduction
;;;   4. Dead-Code Elimination
;;;
;;; The four passes are applied in order, over the whole tree, and the whole
;;; sequence is repeated until a full pass makes zero changes (a fixed
;;; point), capped at 50 iterations.
;;;
;;; The program has already passed the type checker, so every node is tagged
;;; with its type, and some of the rules use those tags.  The optimizer never
;;; re-runs the checker.  It doesn't need to, because every rule replaces an
;;; expression with one that has the same value, and so the same type.  Two
;;; kinds of nodes need care, because the constructors in astnodes.scm tag a
;;; new node RUNTIME:
;;;   - When a pass rebuilds a node (e.g., a Call with folded arguments), the
;;;     new node gets the old node's type, via same-type.
;;;   - When a rule makes a brand new node (e.g., a folded literal), it tags
;;;     the node with its type, via with-type.

;;; Get the tag symbol for an AST node
;;;
;;; @param node The AST node
;;;
;;; @return the tag symbol (e.g. 'AST_CALL)
(define (node-type node) (vector-ref node 0))

;;; Node types that already hold a computed numeric value
;;;
;;; @param node The AST node
;;;
;;; @return #t if node is an Int or Dbl literal
(define (is-numeric-literal? node)
  (memq (node-type node) '(AST_INT AST_DBL)))

;;; Tag a node with a type
;;;
;;; @param node The AST node
;;; @param type The type symbol
;;;
;;; @return node
(define (with-type node type)
  (ast-set-type! node type)
  node)

;;; Give a rebuilt node the type of the node that it replaces
;;;
;;; @param old The original AST node
;;; @param new The rebuilt AST node
;;;
;;; @return new
(define (same-type old new)
  (with-type new (ast-get-type old)))

;;; Make a new Bool literal, tagged BOOL
;;;
;;; @param b A Scheme boolean
;;;
;;; @return a Bool literal node
(define (make-bool-lit b)
  (with-type (if b (make-bool-true-node) (make-bool-false-node)) 'BOOL))

;;; Check whether every node in a list is a numeric literal
;;;
;;; @param args The list of AST nodes
;;;
;;; @return #t if all are Int/Dbl literals
(define (all-numeric-literals? args)
  (if (null? args)
      #t
      (and (is-numeric-literal? (car args)) (all-numeric-literals? (cdr args)))))

;;; Compute the remainder of two Dbls, like C's `fmod`: the result has the
;;; sign of `a`.  The arithmetic is done with exact rationals, so the result is
;;; exact.
;;;
;;; @param a The dividend
;;; @param b The divisor
;;;
;;; @return the remainder, or NaN if `b` is zero
(define (dbl-remainder a b)
  (if (= b 0)
      +nan.0
      (let ((ea (inexact->exact a)) (eb (inexact->exact b)))
        (exact->inexact (- ea (* eb (truncate (/ ea eb))))))))

;;; Apply one arithmetic operator to two numbers
;;;
;;; @param op   The operator name (e.g. "+")
;;; @param a    The left operand
;;; @param b    The right operand
;;; @param int? #t if both operands are Ints, so `/` and `%` are Int operations
;;;
;;; @return the result
(define (arith-step op a b int?)
  (cond
   ((string=? op "+") (+ a b))
   ((string=? op "-") (- a b))
   ((string=? op "*") (* a b))
   ((string=? op "/") (if int? (quotient a b) (/ (exact->inexact a) (exact->inexact b))))
   (#t (if int? (remainder a b) (dbl-remainder (exact->inexact a) (exact->inexact b))))))

;;; Fold an arithmetic or comparison call whose arguments are all numeric
;;; literals, computing the result the same way the run-time standard library
;;; does.  For arithmetic, with one argument, `-` negates it, and the other
;;; operators return it unchanged.  Otherwise, the arguments are combined left
;;; to right.  Until the first Dbl, this uses Int arithmetic (so `/` truncates
;;; toward zero), and after that it uses Dbl arithmetic.  So `(/ 7 2 1.0)` is
;;; 3.0, not 3.5.  The result is a Dbl if any argument is a Dbl.
;;;
;;; @param op   The operator name (e.g. "+")
;;; @param args The non-empty list of numeric literal argument nodes
;;;
;;; @return a literal AST node with the result, or #f if the call would divide
;;;         an Int by an Int zero or the result would be an infinite or NaN Dbl
(define (fold-arith op args)
  (let ((vals (map (lambda (a) (vector-ref a 1)) args))
        (any-dbl (not (all-int-literals? args))))
    (cond
     ((assoc op compare-ops)
      (make-bool-lit (apply (cdr (assoc op compare-ops)) vals)))
     ((null? (cdr args))
      (if (string=? op "-")
          (if any-dbl
              (with-type (make-dbl-node (- (car vals))) 'DBL)
              (with-type (make-int-node (- (car vals))) 'INT))
          (car args)))
     (#t
      ;; `int?` is #t while every argument so far is an Int
      (let loop ((res (car vals))
                 (int? (eq? (node-type (car args)) 'AST_INT))
                 (rest (cdr args)))
        (if (null? rest)
            (cond
             ((not any-dbl) (with-type (make-int-node res) 'INT))
             ;; An infinite or NaN result (e.g. from dividing a Dbl by zero)
             ;; is not folded, since there is no literal that can represent it
             ((let ((d (exact->inexact res))) (or (not (= d d)) (= d +inf.0) (= d -inf.0))) #f)
             (#t (with-type (make-dbl-node (exact->inexact res)) 'DBL)))
            (let* ((next-int? (and int? (eq? (node-type (car rest)) 'AST_INT)))
                   (b (vector-ref (car rest) 1)))
              (if (and next-int? (member op '("/" "%")) (= b 0))
                  #f
                  (loop (arith-step op res b next-int?) next-int? (cdr rest))))))))))

;;; The comparison operators, and the Scheme function that computes each one
(define compare-ops (list (cons "<" <) (cons "<=" <=) (cons "=" =) (cons ">" >) (cons ">=" >=)))

;;; Check whether every node in a list is an Int literal
;;;
;;; @param args The list of AST nodes
;;;
;;; @return #t if all are Int literals
(define (all-int-literals? args)
  (if (null? args)
      #t
      (and (eq? (node-type (car args)) 'AST_INT) (all-int-literals? (cdr args)))))

;;; Maximum number of full pass-sequence iterations before giving up
(define optimize-max-iterations 50)

;;; A literal is an Int, Dbl, Bool, Char, or Str node.
(define (literal? node)
  (memq (node-type node) '(AST_INT AST_DBL AST_BOOL_TRUE AST_BOOL_FALSE AST_CHAR AST_STR)))

;;; An expression is pure if it is a literal or an identifier.
(define (pure? node)
  (or (literal? node) (eq? (node-type node) 'AST_IDENTIFIER)))

(define (identifier? node)
  (eq? (node-type node) 'AST_IDENTIFIER))

(define (identifier-name node)
  (vector-ref node 1))

;;; Quoted data and Cons/Vec literals are opaque to every pass.
(define (opaque-node? node)
  (memq (node-type node) '(AST_QUOTE AST_TICK AST_CONS AST_EMPTY_CONS AST_VEC)))

(define (unstable-name? name unstable)
  (if (member name unstable) #t #f))

;;; Copy a literal (or identifier) so propagation/strength reduction never
;;; alias the original node.
(define (clone-literal node)
  (let ((kind (node-type node)))
    (cond
     ((eq? kind 'AST_INT)
      (with-type (make-int-node (vector-ref node 1)) 'INT))
     ((eq? kind 'AST_DBL)
      (with-type (make-dbl-node (vector-ref node 1)) 'DBL))
     ((eq? kind 'AST_BOOL_TRUE) (make-bool-lit #t))
     ((eq? kind 'AST_BOOL_FALSE) (make-bool-lit #f))
     ((eq? kind 'AST_CHAR)
      (with-type (make-char-node (vector-ref node 1)) 'CHAR))
     ((eq? kind 'AST_STR)
      (with-type (make-str-node (vector-ref node 1)) 'STR))
     ((eq? kind 'AST_IDENTIFIER)
      (with-type (make-identifier-node (vector-ref node 1)) (ast-get-type node)))
     (else node))))

;;; Rewrite each node in a list.  Returns (new-list . changed?).
(define (rewrite-nodes nodes rule)
  (if (null? nodes)
      (cons '() #f)
      (let ((first (rewrite (car nodes) rule))
            (rest (rewrite-nodes (cdr nodes) rule)))
        (cons (cons (car first) (car rest))
              (or (cdr first) (cdr rest))))))

;;; Visit executable children of `node`, writing updated children back.
;;; Returns #t if any child changed.
(define (rewrite-children! node rule)
  (let ((kind (node-type node)))
    (define (set-list! slot)
      (let ((result (rewrite-nodes (vector-ref node slot) rule)))
        (vector-set! node slot (car result))
        (cdr result)))
    (define (set-child! slot)
      (let ((result (rewrite (vector-ref node slot) rule)))
        (vector-set! node slot (car result))
        (cdr result)))
    (cond
     ((memq kind '(AST_AND AST_OR AST_CALL AST_BEGIN))
      (set-list! 1))
     ((eq? kind 'AST_IF)
      (or (set-child! 1) (set-child! 2) (set-child! 3)))
     ((eq? kind 'AST_COND)
      (let loop ((cs (vector-ref node 1)) (changed #f))
        (if (null? cs)
            changed
            (let* ((c (car cs))
                   (test (rewrite (vector-ref c 1) rule))
                   (exprs (rewrite-nodes (vector-ref c 2) rule)))
              (vector-set! c 1 (car test))
              (vector-set! c 2 (car exprs))
              (loop (cdr cs) (or changed (cdr test) (cdr exprs)))))))
     ((eq? kind 'AST_DEFINE_VAR) (set-child! 2))
     ((eq? kind 'AST_DEFINE_FUNC) (set-list! 2))
     ((eq? kind 'AST_LAMBDA_DEF) (set-list! 2))
     ((eq? kind 'AST_SET) (set-child! 2))
     ((eq? kind 'AST_LET)
      (let loop ((bs (vector-ref node 1)) (changed #f))
        (if (null? bs)
            (or changed (set-list! 2))
            (let ((val (rewrite (vector-ref (car bs) 2) rule)))
              (vector-set! (car bs) 2 (car val))
              (loop (cdr bs) (or changed (cdr val)))))))
     ((eq? kind 'AST_APPLY)
      (or (set-child! 1) (set-child! 2)))
     (else #f))))

;;; Bottom-up rewrite.  Does not look inside quote/' / Cons / Vec.
(define (rewrite node rule)
  (if (opaque-node? node)
      (rule node)
      (let ((child-changed (rewrite-children! node rule)))
        (let ((result (rule node)))
          (cons (car result) (or child-changed (cdr result)))))))

(define (unchanged node) (cons node #f))

(define (type-pred-name? name)
  (member name '("integer?" "double?" "number?" "symbol?" "string?" "char?"
                 "vector?" "list?" "null?")))

(define (type-pred-true? name typ)
  (cond
   ((string=? name "integer?") (eq? typ 'INT))
   ((string=? name "double?") (eq? typ 'DBL))
   ((string=? name "number?") (or (eq? typ 'INT) (eq? typ 'DBL)))
   ((string=? name "symbol?") (eq? typ 'SYMBOL))
   ((string=? name "string?") (eq? typ 'STR))
   ((string=? name "char?") (eq? typ 'CHAR))
   ((string=? name "vector?") (eq? typ 'VEC))
   (else #f)))

;;; Fold a built-in call whose operator is an identifier.
(define (fold-call node)
  (let ((exprs (vector-ref node 1)))
    (if (or (null? exprs) (not (identifier? (car exprs))))
        (unchanged node)
        (let ((op (identifier-name (car exprs)))
              (args (cdr exprs)))
          (cond
           ((null? args)
            (if (string=? op "string-append")
                (cons (with-type (make-str-node "") 'STR) #t)
                (unchanged node)))
           ((member op '("+" "-" "*" "/" "%"))
            (if (all-numeric-literals? args)
                (let ((folded (fold-arith op args)))
                  (if folded (cons folded #t) (unchanged node)))
                (unchanged node)))
           ((assoc op compare-ops)
            (if (all-numeric-literals? args)
                (let ((folded (fold-arith op args)))
                  (if folded (cons folded #t) (unchanged node)))
                (unchanged node)))
           ((string=? op "not")
            (if (= (length args) 1)
                (let ((kind (node-type (car args))))
                  (cond
                   ((eq? kind 'AST_BOOL_TRUE) (cons (make-bool-lit #f) #t))
                   ((eq? kind 'AST_BOOL_FALSE) (cons (make-bool-lit #t) #t))
                   (else (unchanged node))))
                (unchanged node)))
           ((string=? op "string-append")
            (let loop ((rest args) (parts '()))
              (cond
               ((null? rest)
                (cons (with-type (make-str-node (apply string-append (reverse parts))) 'STR) #t))
               ((eq? (node-type (car rest)) 'AST_STR)
                (loop (cdr rest) (cons (vector-ref (car rest) 1) parts)))
               (else (unchanged node)))))
           ((string=? op "string-length")
            (if (and (= (length args) 1) (eq? (node-type (car args)) 'AST_STR))
                (cons (with-type (make-int-node (string-length (vector-ref (car args) 1))) 'INT) #t)
                (unchanged node)))
           ((type-pred-name? op)
            (if (or (not (= (length args) 1))
                    (not (pure? (car args)))
                    (eq? (ast-get-type (car args)) 'RUNTIME))
                (unchanged node)
                (let ((typ (ast-get-type (car args))))
                  (if (and (or (string=? op "list?") (string=? op "null?"))
                           (eq? typ 'CONS))
                      (unchanged node)
                      (cons (make-bool-lit (type-pred-true? op typ)) #t)))))
           (else (unchanged node)))))))

;;; Constant folding (bottom-up via rewrite).
(define (fold-rule node)
  (let ((kind (node-type node)))
    (cond
     ((eq? kind 'AST_CALL) (fold-call node))
     ((eq? kind 'AST_AND)
      (let ((exprs (vector-ref node 1)))
        (if (and (not (null? exprs)) (eq? (node-type (car exprs)) 'AST_BOOL_FALSE))
            (cons (make-bool-lit #f) #t)
            (unchanged node))))
     ((eq? kind 'AST_OR)
      (let ((exprs (vector-ref node 1)))
        (if (and (not (null? exprs)) (eq? (node-type (car exprs)) 'AST_BOOL_TRUE))
            (cons (make-bool-lit #t) #t)
            (unchanged node))))
     ((eq? kind 'AST_IF)
      (let ((cond-kind (node-type (vector-ref node 1))))
        (cond
         ((eq? cond-kind 'AST_BOOL_TRUE) (cons (vector-ref node 2) #t))
         ((eq? cond-kind 'AST_BOOL_FALSE) (cons (vector-ref node 3) #t))
         (else (unchanged node)))))
     (else (unchanged node)))))

;;; Association-list environment boxed in a vector so siblings can see updates.
(define (make-env-box alist) (vector alist))
(define (env-alist box) (vector-ref box 0))
(define (env-copy-box box) (vector (vector-ref box 0)))

(define (alist-delete alist name)
  (cond
   ((null? alist) '())
   ((string=? (caar alist) name) (alist-delete (cdr alist) name))
   (else (cons (car alist) (alist-delete (cdr alist) name)))))

(define (env-get box name)
  (let loop ((alist (env-alist box)))
    (cond
     ((null? alist) #f)
     ((string=? (caar alist) name) (cdar alist))
     (else (loop (cdr alist))))))

(define (env-put! box name value)
  (vector-set! box 0 (cons (cons name value) (alist-delete (env-alist box) name))))

(define (env-delete! box name)
  (vector-set! box 0 (alist-delete (env-alist box) name)))

(define (env-delete-names! box names)
  (for-each (lambda (id) (env-delete! box (identifier-name id))) names))

;;; Propagate through a sequence.  Top-level `define` of a literal becomes a
;;; constant for later expressions; nested defines only hide an outer constant.
(define (propagate-list nodes env-box unstable top-level changed)
  (if (null? nodes)
      '()
      (let ((updated (propagate-node (car nodes) env-box unstable changed)))
        (if (and top-level (eq? (node-type updated) 'AST_DEFINE_VAR))
            (let ((name (identifier-name (vector-ref updated 1)))
                  (expr (vector-ref updated 2)))
              (if (and (not (unstable-name? name unstable)) (literal? expr))
                  (env-put! env-box name (clone-literal expr))
                  (env-delete! env-box name))))
        (cons updated
              (propagate-list (cdr nodes) env-box unstable top-level changed)))))

;;; Constant propagation.  A constant is an un-unstable name bound by a
;;; top-level (define x <literal>) or a (let ((x <literal>) ...) ...) binding.
(define (propagate-node node env-box unstable changed)
  (let ((kind (node-type node)))
    (cond
     ((eq? kind 'AST_IDENTIFIER)
      (let ((value (env-get env-box (identifier-name node))))
        (if (and value (not (unstable-name? (identifier-name node) unstable)))
            (begin (vector-set! changed 0 #t)
                   (clone-literal value))
            node)))
     ((opaque-node? node) node)
     ((eq? kind 'AST_DEFINE_VAR)
      (vector-set! node 2 (propagate-node (vector-ref node 2) env-box unstable changed))
      (env-delete! env-box (identifier-name (vector-ref node 1)))
      node)
     ((eq? kind 'AST_SET)
      (vector-set! node 2 (propagate-node (vector-ref node 2) env-box unstable changed))
      (env-delete! env-box (identifier-name (vector-ref node 1)))
      node)
     ((eq? kind 'AST_LET)
      (let ((local (env-copy-box env-box))
            (outer (env-copy-box env-box))
            (bindings (vector-ref node 1)))
        (for-each (lambda (b) (env-delete! local (identifier-name (vector-ref b 1))))
                  bindings)
        (for-each
         (lambda (b)
           (let ((val (propagate-node (vector-ref b 2) outer unstable changed))
                 (name (identifier-name (vector-ref b 1))))
             (vector-set! b 2 val)
             (if (and (not (unstable-name? name unstable)) (literal? val))
                 (env-put! local name (clone-literal val)))))
         bindings)
        (vector-set! node 2 (propagate-list (vector-ref node 2) local unstable #f changed))
        node))
     ((eq? kind 'AST_BEGIN)
      (vector-set! node 1
                   (propagate-list (vector-ref node 1)
                                   (env-copy-box env-box) unstable #f changed))
      node)
     ((eq? kind 'AST_LAMBDA_DEF)
      (let ((local (env-copy-box env-box)))
        (env-delete-names! local (vector-ref node 1))
        (vector-set! node 2 (propagate-list (vector-ref node 2) local unstable #f changed))
        node))
     ((eq? kind 'AST_DEFINE_FUNC)
      (let ((local (env-copy-box env-box))
            (ids (vector-ref node 1)))
        (if (not (null? ids))
            (env-delete-names! local (cdr ids)))
        (vector-set! node 2 (propagate-list (vector-ref node 2) local unstable #f changed))
        node))
     ((eq? kind 'AST_IF)
      (vector-set! node 1 (propagate-node (vector-ref node 1) env-box unstable changed))
      (vector-set! node 2 (propagate-node (vector-ref node 2) (env-copy-box env-box) unstable changed))
      (vector-set! node 3 (propagate-node (vector-ref node 3) (env-copy-box env-box) unstable changed))
      node)
     ((eq? kind 'AST_COND)
      (for-each
       (lambda (c)
         (vector-set! c 1 (propagate-node (vector-ref c 1) (env-copy-box env-box) unstable changed))
         (vector-set! c 2 (propagate-list (vector-ref c 2)
                                          (env-copy-box env-box) unstable #f changed)))
       (vector-ref node 1))
      node)
     (else
      (rewrite-children! node
                         (lambda (child)
                           (cons (propagate-node child env-box unstable changed) #f)))
      node))))

;;; True if node is the Int 0/1/2, or the Dbl 1.0 when dbl? is true.
(define (numeric-literal? node value dbl?)
  (if dbl?
      (and (eq? (node-type node) 'AST_DBL)
           (= (vector-ref node 1) value))
      (and (eq? (node-type node) 'AST_INT)
           (= (vector-ref node 1) value))))

(define (numeric-type? node)
  (or (eq? (ast-get-type node) 'INT) (eq? (ast-get-type node) 'DBL)))

;;; (* e 2) / (* 2 e) becomes (+ e e), tagged with the type of e.
(define (make-plus-self e)
  (same-type e (make-call-node (list (make-identifier-node "+")
                                     (clone-literal e)
                                     (clone-literal e)))))

;;; Strength reduction: two-argument calls only.
(define (strength-rule node)
  (if (or (not (eq? (node-type node) 'AST_CALL))
          (not (= (length (vector-ref node 1)) 3))
          (not (identifier? (car (vector-ref node 1)))))
      (unchanged node)
      (let* ((exprs (vector-ref node 1))
             (op (identifier-name (car exprs)))
             (left (cadr exprs))
             (right (caddr exprs)))
        (cond
         ((string=? op "+")
          (cond
           ((and (numeric-type? left) (numeric-literal? right 0 #f)) (cons left #t))
           ((and (numeric-type? right) (numeric-literal? left 0 #f)) (cons right #t))
           (else (unchanged node))))
         ((string=? op "-")
          (if (and (numeric-type? left) (numeric-literal? right 0 #f))
              (cons left #t)
              (unchanged node)))
         ((string=? op "/")
          (cond
           ((and (numeric-type? left) (numeric-literal? right 1 #f)) (cons left #t))
           ((and (eq? (ast-get-type left) 'DBL) (numeric-literal? right 1 #t))
            (cons left #t))
           (else (unchanged node))))
         ((string=? op "*")
          (cond
           ((and (numeric-type? left) (numeric-literal? right 1 #f)) (cons left #t))
           ((and (numeric-type? right) (numeric-literal? left 1 #f)) (cons right #t))
           ((and (eq? (ast-get-type left) 'DBL) (numeric-literal? right 1 #t))
            (cons left #t))
           ((and (eq? (ast-get-type right) 'DBL) (numeric-literal? left 1 #t))
            (cons right #t))
           ((or (and (eq? (ast-get-type left) 'INT) (pure? left)
                     (numeric-literal? right 0 #f))
                (and (eq? (ast-get-type right) 'INT) (pure? right)
                     (numeric-literal? left 0 #f)))
            (cons (with-type (make-int-node 0) 'INT) #t))
           ((and (pure? left) (numeric-type? left) (numeric-literal? right 2 #f))
            (cons (make-plus-self left) #t))
           ((and (pure? right) (numeric-type? right) (numeric-literal? left 2 #f))
            (cons (make-plus-self right) #t))
           (else (unchanged node))))
         (else (unchanged node))))))

;;; Drop every pure expression except the last in a body.
(define (clean-body body)
  (if (or (null? body) (null? (cdr body)))
      (cons body #f)
      (let loop ((rest body) (kept '()) (changed #f))
        (if (null? (cdr rest))
            (cons (reverse (cons (car rest) kept)) changed)
            (if (pure? (car rest))
                (loop (cdr rest) kept #t)
                (loop (cdr rest) (cons (car rest) kept) changed))))))

;;; Dead-code elimination.  Top-level forest entries are never dropped; a
;;; one-expression `begin` is replaced by that expression.
(define (dead-rule node)
  (let ((kind (node-type node)))
    (cond
     ((eq? kind 'AST_BEGIN)
      (let ((cleaned (clean-body (vector-ref node 1))))
        (vector-set! node 1 (car cleaned))
        (if (= (length (car cleaned)) 1)
            (cons (car (car cleaned)) #t)
            (cons node (cdr cleaned)))))
     ((eq? kind 'AST_LET)
      (let ((cleaned (clean-body (vector-ref node 2))))
        (vector-set! node 2 (car cleaned))
        (cons node (cdr cleaned))))
     ((eq? kind 'AST_LAMBDA_DEF)
      (let ((cleaned (clean-body (vector-ref node 2))))
        (vector-set! node 2 (car cleaned))
        (cons node (cdr cleaned))))
     ((eq? kind 'AST_DEFINE_FUNC)
      (let ((cleaned (clean-body (vector-ref node 2))))
        (vector-set! node 2 (car cleaned))
        (cons node (cdr cleaned))))
     ((eq? kind 'AST_COND)
      (let loop ((cs (vector-ref node 1)) (changed #f))
        (if (null? cs)
            (cons node changed)
            (let ((cleaned (clean-body (vector-ref (car cs) 2))))
              (vector-set! (car cs) 2 (car cleaned))
              (loop (cdr cs) (or changed (cdr cleaned)))))))
     (else (unchanged node)))))

(define (rewrite-forest forest rule)
  (rewrite-nodes forest rule))

;;; Apply the four-pass sequence repeatedly until a fixed point (or the
;;; iteration cap is hit)
;;;
;;; @param forest   The list of top-level AST nodes (already type checked)
;;; @param unstable The program's unstable names (see find-unstable-names)
;;;
;;; @return the optimized list of top-level AST nodes
(define (optimize-forest forest unstable)
  (let loop ((i 0) (forest forest))
    (if (>= i optimize-max-iterations)
        forest
        (let* ((folded (rewrite-forest forest fold-rule))
               (changed (vector #f))
               (propagated (propagate-list (car folded) (make-env-box '())
                                           unstable #t changed))
               (reduced (rewrite-forest propagated strength-rule))
               (cleaned (rewrite-forest (car reduced) dead-rule)))
          (if (or (cdr folded) (vector-ref changed 0) (cdr reduced) (cdr cleaned))
              (loop (+ i 1) (car cleaned))
              (car cleaned))))))

;;; Typecheck a list of top-level AST nodes, printing (and stopping at) the
;;; first warning.
;;;
;;; @param nodes The list of AST nodes
;;; @param env   The type environment
;;;
;;; @return #t if every node typechecked cleanly, #f if any warning was found
;;;         (and already printed)
(define (typecheck-report nodes env)
  (if (null? nodes)
      #t
      (let* ((res (typecheck-ast (car nodes) env))
             (err (cdr res)))
        (if (string? err)
            (begin (display err) (newline) #f)
            (typecheck-report (cdr nodes) env)))))

;;; Typecheck, then (only on success) optimize and print the resulting
;;; AST-XML, exactly matching the "typecheck first, conditionally emit XML"
;;; contract in OPTIMIZE_SPEC.md.
;;;
;;; @param nodes The list of top-level AST nodes to optimize
(define (optimize-and-print nodes)
  (let ((unstable (find-unstable-names nodes)))
    (if (typecheck-report nodes (make-type-env unstable))
        (begin (display (ast-to-xml (optimize-forest nodes unstable))) (newline)))))
