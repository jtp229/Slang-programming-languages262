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

;;; Apply the four-pass sequence repeatedly until a fixed point (or the
;;; iteration cap is hit)
;;;
;;; @param forest   The list of top-level AST nodes (already type checked)
;;; @param unstable The program's unstable names (see find-unstable-names)
;;;
;;; @return the optimized list of top-level AST nodes
(define (optimize-forest forest unstable)
  ;; TODO: implement this
  (error "optimize-forest is not implemented"))

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
