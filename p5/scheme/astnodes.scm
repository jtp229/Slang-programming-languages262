;;; Create a node to represent a Bool True datum
;;;
;;; @return a Vector representing the node
(define (make-bool-true-node) (vector 'AST_BOOL_TRUE 'RUNTIME))

;;; Create a node to represent a Bool False datum
;;;
;;; @return a Vector representing the node
(define (make-bool-false-node) (vector 'AST_BOOL_FALSE 'RUNTIME))

;;; Create a node to represent a Char datum
;;;
;;; @param val The character value
;;;
;;; @return a Vector representing the node
(define (make-char-node val) (vector 'AST_CHAR val 'RUNTIME))

;;; Create a node to represent a Cons datum
;;;
;;; @param car-val The car of the cons
;;; @param cdr-val The cdr of the cons
;;;
;;; @return a Vector representing the node
(define (make-cons-node car-val cdr-val) (vector 'AST_CONS car-val cdr-val 'RUNTIME))

;;; Create a node to represent an Empty Cons datum
;;;
;;; @return a Vector representing the node
(define (make-empty-cons-node) (vector 'AST_EMPTY_CONS 'RUNTIME))

;;; Create a node to represent a Dbl datum
;;;
;;; @param val The double value
;;;
;;; @return a Vector representing the node
(define (make-dbl-node val) (vector 'AST_DBL val 'RUNTIME))

;;; Create a node to represent an Int datum
;;;
;;; @param val The integer value
;;;
;;; @return a Vector representing the node
(define (make-int-node val) (vector 'AST_INT val 'RUNTIME))

;;; Create a node to represent a Str datum
;;;
;;; @param val The string value
;;;
;;; @return a Vector representing the node
(define (make-str-node val) (vector 'AST_STR val 'RUNTIME))

;;; Create a node to represent a Symbol datum
;;;
;;; @param val The symbol value
;;;
;;; @return a Vector representing the node
(define (make-symbol-node val) (vector 'AST_SYMBOL val 'RUNTIME))

;;; Create a node to represent a Vec datum
;;;
;;; @param items A list of items to put in the vector
;;;
;;; @return a Vector representing the node
(define (make-vec-node items) (vector 'AST_VEC items 'RUNTIME))

;;; Create a node to represent an And node
;;;
;;; @param exprs A list of expressions in the and
;;;
;;; @return a Vector representing the node
(define (make-and-node exprs) (vector 'AST_AND exprs 'RUNTIME))

;;; Create a node to represent an Apply node
;;;
;;; @param func The function to apply
;;; @param args The arguments to the function
;;;
;;; @return a Vector representing the node
(define (make-apply-node func args) (vector 'AST_APPLY func args 'RUNTIME))

;;; Create a node to represent a Call node
;;;
;;; @param exprs A list of expressions in the call
;;;
;;; @return a Vector representing the node

(define (make-call-node exprs) (vector 'AST_CALL exprs 'RUNTIME))

;;; Create a node to represent a Begin node
;;;
;;; @param exprs A list of expressions in the begin
;;;
;;; @return a Vector representing the node
(define (make-begin-node exprs) (vector 'AST_BEGIN exprs 'RUNTIME))

;;; Create an object to represent a condition (not an AST node!)
;;;
;;; @param test  The test expression
;;; @param exprs A list of body expressions
;;;
;;; @return a Vector representing the condition
(define (make-condition test exprs) (vector 'CONDITION test exprs))

;;; Create a node to represent a Cond node
;;;
;;; @param conditions The list of conditions
;;;
;;; @return a Vector representing the node
(define (make-cond-node conditions) (vector 'AST_COND conditions 'RUNTIME))

;;; Create a node to represent a DefineVar node
;;;
;;; @param id   The variable identifier
;;; @param expr The value expression
;;;
;;; @return a Vector representing the node
(define (make-define-var-node id expr) (vector 'AST_DEFINE_VAR id expr 'RUNTIME))

;;; Create a node to represent a DefineFunc node
;;;
;;; @param ids  A list of parameter identifiers
;;; @param body A list of expressions in the function body
;;;
;;; @return a Vector representing the node
(define (make-define-func-node ids body) (vector 'AST_DEFINE_FUNC ids body 'RUNTIME))

;;; Create a node to represent an Identifier node
;;;
;;; @param id The identifier name
;;;
;;; @return a Vector representing the node
(define (make-identifier-node id) (vector 'AST_IDENTIFIER id 'RUNTIME))

;;; Create a node to represent an If node
;;;
;;; @param test-expr  The test expression
;;; @param if-true    The true expression
;;; @param if-false   The false expression
;;;
;;; @return a Vector representing the node
(define (make-if-node test-expr if-true if-false)
  (vector 'AST_IF test-expr if-true if-false 'RUNTIME))

;;; Create a node to represent a LambdaDef node
;;;
;;; @param formals A list of parameter identifiers
;;; @param exprs   A list of expressions in the body
;;;
;;; @return a Vector representing the node
(define (make-lambda-def-node formals exprs)
  (vector 'AST_LAMBDA_DEF formals exprs 'RUNTIME))

;;; Create an object to represent a let binding (not an AST node!)
;;;
;;; @param id  The identifier
;;; @param val The value expression
;;;
;;; @return a Vector representing the binding
(define (make-let-def id val) (vector 'LET_DEF id val))

;;; Create a node to represent a Let node
;;;
;;; @param vars A list of bindings
;;; @param body A list of expressions in the let body
;;;
;;; @return a Vector representing the node
(define (make-let-node vars body) (vector 'AST_LET vars body 'RUNTIME))

;;; Create a node to represent an Or node
;;;
;;; @param exprs A list of expressions in the or
;;;
;;; @return a Vector representing the node
(define (make-or-node exprs) (vector 'AST_OR exprs 'RUNTIME))

;;; Create a node to represent a Quote node
;;;
;;; @param datum The quoted datum
;;;
;;; @return a Vector representing the node
(define (make-quote-node datum) (vector 'AST_QUOTE datum 'RUNTIME))

;;; Create a node to represent a Set node
;;;
;;; @param id   The variable identifier
;;; @param expr The value expression
;;;
;;; @return a Vector representing the node
(define (make-set-node id expr) (vector 'AST_SET id expr 'RUNTIME))

;;; Create a node to represent a Tick node
;;;
;;; @param datum The ticked datum
;;;
;;; @return a Vector representing the node
(define (make-tick-node datum) (vector 'AST_TICK datum 'RUNTIME))

;;; Create a node to represent an error message
;;;
;;; @param msg The error message
;;;
;;; @return a Vector representing the error node
(define (make-err-node msg) (vector 'AST_ERR msg 'RUNTIME))

;;; Get the type decoration for an AST node
;;;
;;; @param node The AST node
;;;
;;; @return the type symbol
(define (ast-get-type node)
  (vector-ref node (- (vector-length node) 1)))

;;; Set the type decoration for an AST node
;;;
;;; @param node The AST node
;;; @param type The type symbol
(define (ast-set-type! node type)
  (vector-set! node (- (vector-length node) 1) type))
