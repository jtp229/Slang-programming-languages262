;; prime_divisors: compute the prime factorization of a number
;;
;; To test this function, open a new `gsi` instance and then type:
;;  (load "prime_divisors.scm")
;; Then you can issue commands such as:
;;  (prime-divisors 60)
;; And you should see results of the form:
;;  (2 2 3 5)

;; This is a skeleton for the prime-divisors function.  For now, it just
;; returns #f (false)
;;
;; Note that you will almost certainly want to write some helper functions,
;; and also that this will probably need to be a recursive function.  You are
;; not required to use good information hiding.  That is, you may `define`
;; other functions in the global namespace and use them from
;; `prime-divisors`.
(define (prime-divisors n)
  (define (helper n divisor)
    (cond
    ((= n 1) '())
  ;; if n = 1 the original number has been factored all the way down, return empty list
    ((> (* divisor divisor) n)(list n))
  ;;if the divisor squared is bigger then n, n cant have any more divisors, so n is prime and can be returned
    ((= (remainder n divisor) 0)
    (cons divisor (helper (/ n divisor) divisor)))
  ;;if the divisor devides evenly add the divisor to the result.  Recursively factor the qoutient w/ the same divisor
    (else (helper n (+ divisor 1)))))
  ;;if the divisor doesnt divide n, move to thenext one
  (helper n 2));;begin by calling the input divided by 2 and recurse from there

