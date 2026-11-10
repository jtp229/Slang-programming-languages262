(include "xml_helpers.scm")

;;; Create a string of spaces for indentation
;;;
;;; @param n The indentation level
;;;
;;; @return a string of spaces
(define (indent-str n) (make-string n #\space))

;;; Serialize a list of nodes into a single XML string
;;;
;;; @param nodes  The list of AST nodes
;;; @param indent The indentation level
;;;
;;; @return an XML string
(define (nodes-to-xml nodes indent)
  (if (null? nodes)
      ""
      (string-append (node-to-xml (car nodes) indent)
                     (nodes-to-xml (cdr nodes) indent))))

;;; Serialize an AST node into an XML string
;;;
;;; @param node   The AST node vector
;;; @param indent The indentation level
;;;
;;; @return an XML string
(define (node-to-xml node indent)
  (let* ((type (vector-ref node 0))
         (ind (indent-str indent)))
    (cond
     ((eq? type 'AST_BOOL_TRUE)  (string-append ind "<Bool val=\"true\"/>\n"))
     ((eq? type 'AST_BOOL_FALSE) (string-append ind "<Bool val=\"false\"/>\n"))
     ((eq? type 'AST_CHAR)
      (string-append ind "<Char val=\"" (xml-escape (vector-ref node 1)) "\"/>\n"))
     ((eq? type 'AST_CONS)
      (string-append ind "<Cons>\n"
                     (node-to-xml (vector-ref node 1) (+ indent 1))
                     (node-to-xml (vector-ref node 2) (+ indent 1))
                     ind "</Cons>\n"))
     ((eq? type 'AST_EMPTY_CONS) (string-append ind "<Empty/>\n"))
     ((eq? type 'AST_DBL)
      (string-append ind "<Dbl val=\"" (dbl->string (vector-ref node 1)) "\"/>\n"))
     ((eq? type 'AST_INT)
      (string-append ind "<Int val=\"" (number->string (vector-ref node 1)) "\"/>\n"))
     ((eq? type 'AST_STR)
      (string-append ind "<Str val=\"" (xml-escape (vector-ref node 1)) "\"/>\n"))
     ((eq? type 'AST_SYMBOL)
      (string-append ind "<Symbol val=\"" (xml-escape (vector-ref node 1)) "\"/>\n"))
     ((eq? type 'AST_VEC)
      (let ((items (vector-ref node 1)))
        (if (null? items)
            (string-append ind "<Vector/>\n")
            (string-append ind "<Vector>\n"
                           (nodes-to-xml items (+ indent 1))
                           ind "</Vector>\n"))))
     ((eq? type 'AST_AND)
      (string-append ind "<And>\n"
                     (nodes-to-xml (vector-ref node 1) (+ indent 1))
                     ind "</And>\n"))
     ((eq? type 'AST_APPLY)
      (string-append ind "<Apply>\n"
                     (node-to-xml (vector-ref node 1) (+ indent 1))
                     (node-to-xml (vector-ref node 2) (+ indent 1))
                     ind "</Apply>\n"))
     ((eq? type 'AST_CALL)
      (string-append ind "<Call>\n"
                     (nodes-to-xml (vector-ref node 1) (+ indent 1))
                     ind "</Call>\n"))
     ((eq? type 'AST_BEGIN)
      (string-append ind "<Begin>\n"
                     (nodes-to-xml (vector-ref node 1) (+ indent 1))
                     ind "</Begin>\n"))
     ((eq? type 'AST_COND)
      (string-append ind "<Cond>\n"
                     (nodes-to-xml (vector-ref node 1) (+ indent 1))
                     ind "</Cond>\n"))
     ((eq? type 'CONDITION)
      (let ((exprs (vector-ref node 2)))
        (string-append ind "<Condition>\n"
                       (indent-str (+ indent 1)) "<Test>\n"
                       (node-to-xml (vector-ref node 1) (+ indent 2))
                       (indent-str (+ indent 1)) "</Test>\n"
                       (if (null? exprs)
                           (string-append (indent-str (+ indent 1)) "<Actions/>\n")
                           (string-append (indent-str (+ indent 1)) "<Actions>\n"
                                          (nodes-to-xml exprs (+ indent 2))
                                          (indent-str (+ indent 1)) "</Actions>\n"))
                       ind "</Condition>\n")))
     ((eq? type 'AST_DEFINE_VAR)
      (string-append ind "<DefineVar>\n"
                     (node-to-xml (vector-ref node 1) (+ indent 1))
                     (node-to-xml (vector-ref node 2) (+ indent 1))
                     ind "</DefineVar>\n"))
     ((eq? type 'AST_DEFINE_FUNC)
      (string-append ind "<DefineFunc>\n"
                     (indent-str (+ indent 1)) "<Identifiers>\n"
                     (nodes-to-xml (vector-ref node 1) (+ indent 2))
                     (indent-str (+ indent 1)) "</Identifiers>\n"
                     (indent-str (+ indent 1)) "<Expressions>\n"
                     (nodes-to-xml (vector-ref node 2) (+ indent 2))
                     (indent-str (+ indent 1)) "</Expressions>\n"
                     ind "</DefineFunc>\n"))
     ((eq? type 'AST_IDENTIFIER)
      (string-append ind "<Identifier val=\"" (xml-escape (vector-ref node 1)) "\"/>\n"))
     ((eq? type 'AST_IF)
      (string-append ind "<If>\n"
                     (node-to-xml (vector-ref node 1) (+ indent 1))
                     (node-to-xml (vector-ref node 2) (+ indent 1))
                     (node-to-xml (vector-ref node 3) (+ indent 1))
                     ind "</If>\n"))
     ((eq? type 'AST_LAMBDA_DEF)
      (let ((formals (vector-ref node 1))
            (body (vector-ref node 2)))
        (string-append ind "<Lambda>\n"
                       (if (list? formals)
                           (if (null? formals)
                               (string-append (indent-str (+ indent 1)) "<Formals/>\n")
                               (string-append (indent-str (+ indent 1)) "<Formals>\n"
                                              (nodes-to-xml formals (+ indent 2))
                                              (indent-str (+ indent 1)) "</Formals>\n"))
                           (node-to-xml formals (+ indent 1)))
                       (indent-str (+ indent 1)) "<Expressions>\n"
                       (nodes-to-xml body (+ indent 2))
                       (indent-str (+ indent 1)) "</Expressions>\n"
                       ind "</Lambda>\n")))
     ((eq? type 'LET_DEF)
      (string-append ind "<Binding>\n"
                     (node-to-xml (vector-ref node 1) (+ indent 1))
                     (node-to-xml (vector-ref node 2) (+ indent 1))
                     ind "</Binding>\n"))
     ((eq? type 'AST_LET)
      (string-append ind "<Let>\n"
                     (indent-str (+ indent 1)) "<Bindings>\n"
                     (nodes-to-xml (vector-ref node 1) (+ indent 2))
                     (indent-str (+ indent 1)) "</Bindings>\n"
                     (indent-str (+ indent 1)) "<Actions>\n"
                     (nodes-to-xml (vector-ref node 2) (+ indent 2))
                     (indent-str (+ indent 1)) "</Actions>\n"
                     ind "</Let>\n"))
     ((eq? type 'AST_OR)
      (string-append ind "<Or>\n"
                     (nodes-to-xml (vector-ref node 1) (+ indent 1))
                     ind "</Or>\n"))
     ((eq? type 'AST_QUOTE)
      (string-append ind "<Quote>\n"
                     (node-to-xml (vector-ref node 1) (+ indent 1))
                     ind "</Quote>\n"))
     ((eq? type 'AST_SET)
      (string-append ind "<Set>\n"
                     (node-to-xml (vector-ref node 1) (+ indent 1))
                     (node-to-xml (vector-ref node 2) (+ indent 1))
                     ind "</Set>\n"))
     ((eq? type 'AST_TICK)
      (string-append ind "<Tick>\n"
                     (node-to-xml (vector-ref node 1) (+ indent 1))
                     ind "</Tick>\n"))
     ((eq? type 'AST_ERR)
      (string-append ind "<Error val=\"" (xml-escape (vector-ref node 1)) "\"/>\n"))
     (#t   (string-append ind "<Unknown type=\"" (symbol->string type) "\"/>\n")))))

;;; Serialize a forest of AST nodes into an XML string
;;;
;;; @param nodes The list of AST nodes
;;;
;;; @return an XML string
(define (ast-to-xml nodes)
  (string-append "<Ast xmlns=\"ast\">\n" (nodes-to-xml nodes 1) "</Ast>"))

;;; Convert an XML string into a forest of AST nodes
;;;
;;; @param xml-str The XML string to parse
;;;
;;; @return A list of AST nodes
(define (xml-to-ast xml-str)
  (call/cc
   (lambda (abort)
     ;; NB: Can't start a lambda with define, but can start let with
     ;;     define, so here it is...
     (let ()
       
       ;; Collect children until a closing tag is found
       ;;
       ;; @param xml-str         The XML string
       ;; @param closing-tag      The expected closing tag
       ;; @param constructor      The AST node constructor
       ;; @param xml-to-node-rec  Recursive call for individual nodes
       ;; @param cur              Current index in the string
       ;; @param nodes            Accumulated nodes
       ;;
       ;; @return a Pair (node . new-index)
       (define (parse-children-node-helper xml-str closing-tag constructor xml-to-node-rec cur nodes)
         (let ((end-idx (xml-consume-on-match xml-str (xml-skip-whitespace xml-str cur) closing-tag)))
           (if end-idx
               (cons (constructor (reverse nodes)) end-idx)
               (let ((child-res (xml-to-node-rec (xml-skip-whitespace xml-str cur))))
                 (parse-children-node-helper xml-str closing-tag constructor xml-to-node-rec (cdr child-res) (cons (car child-res) nodes))))))
       
       ;; Parse a node with homogeneous children, using parse-children-node-helper
       ;;
       ;; @param tag         The opening tag
       ;; @param next-idx    Current index in the string
       ;; @param closing-tag The expected closing tag
       ;; @param constructor The AST node constructor
       ;;
       ;; @return a Pair (node . new-index)
       (define (parse-children-node tag next-idx closing-tag constructor)
         (if (string-contains? tag "/>")
             (cons (constructor '()) next-idx)
             (parse-children-node-helper xml-str closing-tag constructor xml-to-node-rec next-idx '())))
       
       ;; Collect children until a closing tag is found, but don't run a
       ;; constructor on the result
       ;;
       ;; @param cur         Current index in the string
       ;; @param nodes       Accumulated nodes
       ;; @param closing-tag The expected closing tag
       ;;
       ;; @return a Pair (list-of-nodes . new-index)
       (define (wrap-children-helper cur nodes closing-tag)
         (let ((end-idx (xml-consume-on-match xml-str (xml-skip-whitespace xml-str cur) closing-tag)))
           (if end-idx
               (cons (reverse nodes) end-idx)
               (let ((child-res (xml-to-node-rec (xml-skip-whitespace xml-str cur))))
                 (wrap-children-helper (cdr child-res) (cons (car child-res) nodes) closing-tag)))))
       
       ;; Parse a named wrapper element
       ;;
       ;; @param i           Current index in the string
       ;; @param open-tag    The expected opening tag
       ;; @param closing-tag The expected closing tag
       ;;
       ;; @return a Pair (list-of-nodes . new-index)
       (define (parse-wrapped-children i open-tag closing-tag)
         (let ((tag-res (xml-get-next-tag xml-str (xml-skip-whitespace xml-str i) abort)))
           (if (not tag-res)
               (abort (string-append "Expected " open-tag))
               (let ((tag (car tag-res)))
                 (if (not (string-contains? tag open-tag))
                     (abort (string-append "Expected " open-tag " but got " tag))
                     (if (string-contains? tag "/>")
                         (cons '() (cdr tag-res))
                         (wrap-children-helper (cdr tag-res) '() closing-tag)))))))

       ;; Recursive-descent parsing of an XML node
       ;;
       ;; @param i Current index in the string
       ;;
       ;; @return a Pair (node . new-index)
       (define (xml-to-node-rec i)
         (let ((tag-res (xml-get-next-tag xml-str i abort)))
           (if (not tag-res)
               (abort "Unexpected end of input while looking for tag")
               (let ((tag (car tag-res))
                     (next-idx (cdr tag-res)))
                 (cond
                  ((string-contains? tag "<Identifier")
                   (cons (make-identifier-node (xml-unescape (xml-get-attr tag "val" abort))) next-idx))
                  ((string-contains? tag "<Bool")
                   (cons (if (string=? (xml-get-attr tag "val" abort) "true") (make-bool-true-node) (make-bool-false-node)) next-idx))
                  ((string-contains? tag "<Int")
                   (cons (make-int-node (string->number (xml-get-attr tag "val" abort))) next-idx))
                  ((string-contains? tag "<Dbl")
                   (cons (make-dbl-node (string->number (xml-get-attr tag "val" abort))) next-idx))
                  ((string-contains? tag "<Str")
                   (cons (make-str-node (xml-unescape (xml-get-attr tag "val" abort))) next-idx))
                  ((string-contains? tag "<Symbol")
                   (cons (make-symbol-node (xml-unescape (xml-get-attr tag "val" abort))) next-idx))
                  ((string-contains? tag "<Char")
                   (cons (make-char-node (xml-unescape (xml-get-attr tag "val" abort))) next-idx))
                  ((string-contains? tag "<Empty")
                   (cons (make-empty-cons-node) next-idx))
                  ((string-contains? tag "<Vector")
                   (parse-children-node tag next-idx "</Vector>" make-vec-node))
                  ((string-contains? tag "<Call")
                   (parse-children-node tag next-idx "</Call>" make-call-node))
                  ((string-contains? tag "<And")
                   (parse-children-node tag next-idx "</And>" make-and-node))
                  ((string-contains? tag "<Or")
                   (parse-children-node tag next-idx "</Or>" make-or-node))
                  ((string-contains? tag "<Begin")
                   (parse-children-node tag next-idx "</Begin>" make-begin-node))
                  ((string-contains? tag "<Condition")
                   (let* ((test-res (parse-wrapped-children next-idx "<Test" "</Test>"))
                          (exprs-res (parse-wrapped-children (cdr test-res) "<Actions" "</Actions>"))
                          (final-idx (xml-consume-on-match xml-str (cdr exprs-res) "</Condition>")))
                     (if (not final-idx) (abort "Missing </Condition>")
                         (cons (make-condition (if (null? (car test-res)) (abort "Empty <Test>") (caar test-res)) (car exprs-res)) final-idx))))
                  ((string-contains? tag "<Cond")
                   (parse-children-node tag next-idx "</Cond>" make-cond-node))
                  ((string-contains? tag "<If")
                   (let* ((cond-res (xml-to-node-rec next-idx))
                          (true-res (xml-to-node-rec (cdr cond-res)))
                          (false-res (xml-to-node-rec (cdr true-res)))
                          (final-idx (xml-consume-on-match xml-str (cdr false-res) "</If>")))
                     (if (not final-idx) (abort "Missing </If>")
                         (cons (make-if-node (car cond-res) (car true-res) (car false-res)) final-idx))))
                  
                  ((string-contains? tag "<DefineVar")
                   (let* ((id-res (xml-to-node-rec next-idx))
                          (expr-res (xml-to-node-rec (cdr id-res)))
                          (final-idx (xml-consume-on-match xml-str (cdr expr-res) "</DefineVar>")))
                     (if (not final-idx) (abort "Missing </DefineVar>")
                         (cons (make-define-var-node (car id-res) (car expr-res)) final-idx))))
                  
                  ((string-contains? tag "<DefineFunc")
                   (let* ((ids-res (parse-wrapped-children next-idx "<Identifiers" "</Identifiers>"))
                          (body-res (parse-wrapped-children (cdr ids-res) "<Expressions" "</Expressions>"))
                          (final-idx (xml-consume-on-match xml-str (cdr body-res) "</DefineFunc>")))
                     (if (not final-idx) (abort "Missing </DefineFunc>")
                         (cons (make-define-func-node (car ids-res) (car body-res)) final-idx))))
                  
                  ((string-contains? tag "<Lambda")
                   (let* ((formals-res (parse-wrapped-children next-idx "<Formals" "</Formals>"))
                          (body-res (parse-wrapped-children (cdr formals-res) "<Expressions" "</Expressions>"))
                          (final-idx (xml-consume-on-match xml-str (cdr body-res) "</Lambda>")))
                     (if (not final-idx) (abort "Missing </Lambda>")
                         (cons (make-lambda-def-node (car formals-res) (car body-res)) final-idx))))
                  
                  ((string-contains? tag "<Quote")
                   (let* ((datum-res (xml-to-node-rec next-idx))
                          (final-idx (xml-consume-on-match xml-str (cdr datum-res) "</Quote>")))
                     (if (not final-idx) (abort "Missing </Quote>")
                         (cons (make-quote-node (car datum-res)) final-idx))))
                  
                  ((string-contains? tag "<Tick")
                   (let* ((datum-res (xml-to-node-rec next-idx))
                          (final-idx (xml-consume-on-match xml-str (cdr datum-res) "</Tick>")))
                     (if (not final-idx) (abort "Missing </Tick>")
                         (cons (make-tick-node (car datum-res)) final-idx))))
                  
                  ((string-contains? tag "<Cons")
                   (let* ((car-res (xml-to-node-rec next-idx))
                          (cdr-res (xml-to-node-rec (cdr car-res)))
                          (final-idx (xml-consume-on-match xml-str (cdr cdr-res) "</Cons>")))
                     (if (not final-idx) (abort "Missing </Cons>")
                         (cons (make-cons-node (car car-res) (car cdr-res)) final-idx))))
                  
                  ((string-contains? tag "<Set")
                   (let* ((id-res (xml-to-node-rec next-idx))
                          (expr-res (xml-to-node-rec (cdr id-res)))
                          (final-idx (xml-consume-on-match xml-str (cdr expr-res) "</Set>")))
                     (if (not final-idx) (abort "Missing </Set>")
                         (cons (make-set-node (car id-res) (car expr-res)) final-idx))))
                  
                  ((string-contains? tag "<Let")
                   (let* ((vars-res (parse-wrapped-children next-idx "<Bindings" "</Bindings>"))
                          (body-res (parse-wrapped-children (cdr vars-res) "<Actions" "</Actions>"))
                          (final-idx (xml-consume-on-match xml-str (cdr body-res) "</Let>")))
                     (if (not final-idx) (abort "Missing </Let>")
                         (cons (make-let-node (car vars-res) (car body-res)) final-idx))))
                  
                  ((string-contains? tag "<Apply")
                   (let* ((func-res (xml-to-node-rec next-idx))
                          (args-res (xml-to-node-rec (cdr func-res)))
                          (final-idx (xml-consume-on-match xml-str (cdr args-res) "</Apply>")))
                     (if (not final-idx) (abort "Missing </Apply>")
                         (cons (make-apply-node (car func-res) (car args-res)) final-idx))))
                  
                  ((string-contains? tag "<Binding")
                   (let* ((id-res (xml-to-node-rec next-idx))
                          (val-res (xml-to-node-rec (cdr id-res)))
                          (final-idx (xml-consume-on-match xml-str (cdr val-res) "</Binding>")))
                     (if (not final-idx) (abort "Missing </Binding>")
                         (cons (make-let-def (car id-res) (car val-res)) final-idx))))
                  
                  (#t   (abort (string-append "Unknown tag: " tag))))))))
       
       ;; Parse a list of top-level XML nodes
       ;;
       ;; @param cur    Current index in the string
       ;; @param forest Accumulated list of nodes
       ;;
       ;; @return a list of AST nodes
       (define (xml-to-forest cur forest)
         (let ((end-idx (xml-consume-on-match xml-str (xml-skip-whitespace xml-str cur) "</Ast>")))
           (if end-idx
               (reverse forest)
               (let ((node-res (xml-to-node-rec (xml-skip-whitespace xml-str cur))))
                 (xml-to-forest (cdr node-res) (cons (car node-res) forest))))))
       
       ;; Entry point: consume <Ast> open tag and parse forest
       (let ((start-idx (xml-consume-on-match xml-str 0 "<Ast xmlns=\"ast\">")))
         (if (not start-idx)
             '()
             (xml-to-forest start-idx '())))))))
