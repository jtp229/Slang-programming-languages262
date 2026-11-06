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
  (let ((truncated (integer->double (double->integer x))))
    ;; double->integer truncates toward zero
    ;; this is floor for positive numbers but ceiling for negative
    ;; If x is negative and had a fractional part (x != truncated), have to subtract 1 to move down correctly
    (if (and (< x 0) (not (= x truncated)))
        (- truncated 1.0)
        truncated)))

(define (my-ceil x)
  (let ((truncated (integer->double (double->integer x))))
    ;; truncation toward zero is already the ceiling for negative
    ;; For positive numbers truncate moves down.
    ;; If x is positive and had a fractional part add 1.0
    (if (and (> x 0) (not (= x truncated)))
        (+ truncated 1.0)
        truncated)))

(define (my-round x)
  ;; To implement halves round up add 0.5 then take floor0
  (my-floor (+ x 0.5)))