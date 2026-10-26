;; my-floor, my-ceil, my-round: round a double to a whole-valued double.
;;
;;  (my-floor 2.7)  => 2.      (my-floor -2.7) => -3.
;;  (my-ceil 2.1)   => 3.      (my-ceil -2.1)  => -2.
;;  (my-round 2.5)  => 3.      (my-round -2.5) => -2.     (halves round up)
;;
;; You may not use the built-in `floor`, `ceiling`, `round`, or `truncate`.
;; The tool you do have is slang's `double->integer`, which chops off the
;; fractional part (truncating toward zero), and `integer->double`.

(load "slang_compat.scm")

(define (my-floor x)
  #f ;; [CSE 262] Implement Me!
)

(define (my-ceil x)
  #f ;; [CSE 262] Implement Me!
)

(define (my-round x)
  #f ;; [CSE 262] Implement Me!
)
