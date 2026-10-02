;; list2vector takes a list and returns a vector containing the same
;; elements, in the same order.
;;
;; You should implement this without using `list->vector`.
;;
;; Example:
;; (list2vector '(1 2 3)) ; returns #(1 2 3)
;; TODO: implement this function
(define (list2vector l)
 ;;get length of vector
  (let* ((n (length l))
         (result (make-vector n)))

    ;;walk list and index, carry rest of list as argument
    (define (fill! i rest)
      (if (< i n)
          (begin
            ;; Store the current head at position i, then advance both the index and the list.
            (vector-set! result i (car rest))
            ;; Tail call
            (fill! (+ i 1) (cdr rest)))))

    (fill! 0 l)
    ;; return vector 
    result))