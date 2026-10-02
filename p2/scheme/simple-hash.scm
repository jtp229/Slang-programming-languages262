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
  ;; The bucket vector is the hash set's only state. Each slot holds a list
  ;; of the strings that hashed to that slot. It lives in this let, so only
  ;; the dispatch function we return can touch it, and make-hash stays the
  ;; only global symbol.
  (let ((buckets (make-vector size '())))
    (define (hash-string str)
      (let ((len (string-length str)))
        (define (iter i h)
          (if (= i len)
              h
              (iter (+ i 1)
                    (modulo (+ (* h 33) (char->integer (string-ref str i)))
                            size))))
        (iter 0 (modulo 5381 size))))

    ;; check if str is in bucket list
    (define (member? str lst)
      (cond ((null? lst) #f)
            ((string=? str (car lst)) #t)
            (else (member? str (cdr lst)))))

    ;; returns a copy of lst without str
    ;;only call after member? so dropping first match is enough
    (define (remove-from str lst)
      (cond ((null? lst) '())
            ((string=? str (car lst)) (cdr lst))
            (else (cons (car lst) (remove-from str (cdr lst))))))

    ;; the method receiver, hash once per call and reuse index
    (define (dispatch op str)
      (let* ((idx (hash-string str))
             (bucket (vector-ref buckets idx)))
        (cond
          ((eq? op 'contains)
           (member? str bucket))

          ;; insert only if absent so set doesnt duplicate
          ((eq? op 'insert)
           (if (member? str bucket)
               #f
               (begin
                 (vector-set! buckets idx (cons str bucket))
                 #t)))

          ;; remove only if present, return #f if nothing to remove
          ((eq? op 'remove)
           (if (member? str bucket)
               (begin
                 (vector-set! buckets idx (remove-from str bucket))
                 #t)
               #f))

          ;; fail message on typo instead of silently returning
          (else (error "unknown operation:" op)))))
    dispatch))
