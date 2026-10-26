;; slang_compat.scm: definitions so that `gsi` understands the parts of the
;; slang standard library that standard Scheme calls by a different name (or
;; lacks).

;; Constants
(define pi 3.141592653589793)
(define tau (* 2 pi))
(define e (exp 1.))
(define inf+ +inf.0)
(define inf- -inf.0)
(define nan +nan.0)

;; Math: slang spells the natural logarithm `loge`
(define loge log)
(define (nan? x) (not (= x x)))

;; Conversions: truncate toward zero, as slang does
(define (integer->double i) (exact->inexact i))
(define (double->integer d) (inexact->exact (truncate d)))
