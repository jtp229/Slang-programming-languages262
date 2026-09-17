;; read_list: Use the `read` function to read from the keyboard and put the
;; results into a list.  The code should keep reading until EOF (control-d) is
;; input by the user.  It should use recursion, not iterative constructs.
;;
;; The order of elements in the list returned by (read-list) should the reverse
;; of the order in which they were entered.
;;
;; You should *not* define any other functions in the global namespace.  You may
;; need a helper function, but if you do, you should define it so that it is
;; local to `read-list`.

(define (read-list)
  (define (helper l)
  ;; local helper function that takes a list
    (let ((val (read)))
    ;;read one value from the input
      (if (eof-object? val)
      ;;check if we hit eof, and if we did then return the list
          l
          (helper (cons val l)))))
          ;;if not at eof, add the value to the front of the list and then recurse
  (helper '()))
  ;;start with an empty list