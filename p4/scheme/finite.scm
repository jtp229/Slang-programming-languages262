;; my-infinite? and my-finite?: classify a number.
;;
;;  (my-infinite? inf+) => #t    (my-infinite? 3.5) => #f    (my-infinite? nan) => #f
;;  (my-finite? 3.5)    => #t    (my-finite? inf-)  => #f    (my-finite? nan)   => #f
;;
;; You may not use the built-in `finite?` or `infinite?`.  The constants `inf+`,
;; `inf-`, and `nan`, and the function `nan?`, may help.

(load "slang_compat.scm")

(define (my-infinite? x)
  #f ;; [CSE 262] Implement Me!
)

(define (my-finite? x)
  #f ;; [CSE 262] Implement Me!
)
