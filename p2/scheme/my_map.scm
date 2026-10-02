;; my-map: apply a function to every element in a list, and return a list
;; that holds the results.
;;
;; Your implementation of this function is not allowed to use the built-in
;; `map` function.
;;
;; Example:
;; (my-map (lambda (x) (* x x)) '(1 2 3)) ; returns (1 4 9)
;; TODO: implement this function
(define (my-map func l)
  ;; Base case: no elements means nothing to apply func to
  (if (null? l)
      '()
      ;; Apply func to the head in a let so it happens before the recursive call 
      (let ((head (func (car l))))
        ;; cons is O(1), so the whole map is O(n)
        (cons head (my-map func (cdr l))))))
