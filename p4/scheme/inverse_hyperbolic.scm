;; my-asinh, my-acosh, my-atanh: the inverse hyperbolic functions.
;;
;;  asinh(x) = ln(x + sqrt(x^2 + 1))
;;  acosh(x) = ln(x + sqrt(x^2 - 1))        (for x >= 1)
;;  atanh(x) = ln((1 + x) / (1 - x)) / 2    (for -1 < x < 1)
;;
;; Use `loge` (natural logarithm) and `sqrt`.  You do not need to check that
;; the argument is in range.

(load "slang_compat.scm")

(define (my-asinh x)
  #f ;; [CSE 262] Implement Me!
)

(define (my-acosh x)
  #f ;; [CSE 262] Implement Me!
)

(define (my-atanh x)
  #f ;; [CSE 262] Implement Me!
)
