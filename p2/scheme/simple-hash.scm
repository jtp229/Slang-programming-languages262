;; simple-hash creates a basic hash *set* of strings.  It uses the "method
;; receiver" style you saw in tree.scm: `make-hash` returns a function that
;; closes over some state; that function takes two arguments (a symbol
;; naming the operation, and a string to operate on).
;;
;; The argument to make-hash is a positive number: the size of the "bucket
;; vector" for the hash table.
;;
;; Three operations can be requested:
;; - 'contains string - Returns #t if the string is in the hash set, else #f
;; - 'insert string   - Returns #t if the string was inserted, #f if it was
;;                      already present
;; - 'remove string   - Returns #t if the string was removed, #f if it was
;;                      not present to begin with
;;
;; Example:
;; (define my-hash (make-hash 32))
;; (my-hash 'insert "hello")   ; returns #t
;; (my-hash 'contains "world") ; returns #f
;; (my-hash 'contains "hello") ; returns #t
;; (my-hash 'insert "hello")   ; returns #f
;; (my-hash 'remove "world")   ; returns #f
;; (my-hash 'remove "hello")   ; returns #t
;; (my-hash 'remove "hello")   ; returns #f
;; (my-hash 'contains "hello") ; returns #f
;;
;; Strings are hashed with the (very simple) djb2 function from
;; <http://www.cse.yorku.ca/~oz/hash.html>.
;;
;; Your implementation should be clean: the only global symbol exported by
;; this file should be `make-hash`.
;; TODO: implement this function
(define (make-hash size)
  #f ;; [CSE 262] Implement Me!
)
