;; my-append: join two lists into one new list.
;;
;;  (my-append '(1 2) '(3 4)) => (1 2 3 4)
;;  (my-append '() '(3 4))    => (3 4)
;;
;; You may not use the built-in `append`.  Your function should not change
;; either of its arguments.

(define (my-append a b)
  ;;  if the first list is empty, the result is the second list
  (if (null? a)
      b

      ;;  take the first element of list a and cons it onto the result of appending the rest of a to b
      ;;rebuild list from the front so it is not mutataed
      (cons (car a) (my-append (cdr a) b))))
