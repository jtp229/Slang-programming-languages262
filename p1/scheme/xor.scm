;; Compute the exclusive or of two values, using only and, or, and not
;;
;; xor should always return a boolean value
(define (xor a b)
  #f ;; [CSE 262] Implement Me!
  (or (and a (not b)) (and (not a) b))
  ;;We want either a and not b or not a and b.  
)
