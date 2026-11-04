;; my-infinite? and my-finite?: classify a number.
;;
;;  (my-infinite? inf+) => #t    (my-infinite? 3.5) => #f    (my-infinite? nan) => #f
;;  (my-finite? 3.5)    => #t    (my-finite? inf-)  => #f    (my-finite? nan)   => #f
;;
;; You may not use the built-in `finite?` or `infinite?`.  The constants `inf+`,
;; `inf-`, and `nan`, and the function `nan?`, may help.

(load "slang_compat.scm")

(define (my-infinite? x)
;;infinite is either positive or negative
  (or (= x inf+) (= x inf-))
)

(define (my-finite? x)
;;finite only if regular number, so cant be nan(non number) or infinite.
;;
 (and (not (my-infinite? x)) (not (nan? x)))

)
