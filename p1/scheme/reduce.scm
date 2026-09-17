;; reduce takes a binary function, a list, and an identity value, and computes
;; the result of repeatedly applying that function
;;
;; Example: (reduce + '(1 2 3) 0) ==> 6
;;
;; Example: (reduce * '() 1) ==> 1
(define (reduce op l identity)
  #f ;; [CSE 262] Implement Me!
  (if (null? l)
  ;; if the list is empty return the sum for base case
  identity
  (reduce op (cdr l) (op identity (car l)))))
  ;;apply whatever operation to the head of the list with identity, then recurse on the list without that entry
