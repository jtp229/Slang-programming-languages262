;; charvec2string takes a vector of characters and returns a string
;; containing the same characters, in the same order.
;;
;; You should implement this without using `vector->list` or `list->string`.
;;
;; Example:
;; (charvec2string (vector #\h #\i)) ; returns "hi"
;; TODO: implement this function
(define (charvec2string cv)
  ;; allocate string up front instead of appending as we go, already know the length from the given vector
  (let* ((n (vector-length cv))
         (result (make-string n)))


    ;;copy vector indexes into result
    (define (fill! i)
      (if (< i n)
          (begin
          ;;set the string to be the i index of the vector and then recurse
            (string-set! result i (vector-ref cv i))
            ;; tail call
            (fill! (+ i 1)))))

    (fill! 0)
    ;;return finished string specifically
    result))

