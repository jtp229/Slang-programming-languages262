;; list-max and list-min: the largest and smallest number in a non-empty list.
;;
;;  (list-max '(3))         => 3
;;  (list-max '(3 9 4))     => 9
;;  (list-min '(3 9 4 1.5)) => 1.5
;;
;; You may not use the built-in `max` or `min`.  You do not need to handle an
;; empty list.  Your functions should be tail recursive.





(define (list-max l)
  ;; remaining is the part of the list not yet checked
  ;; current-max is the highest value encountered so far
  (define (iter remaining current-max)
    (if (null? remaining)
        current-max ;; Base case
        ;;  compare the head of the list to current best

        (iter (cdr remaining)
              (if (> (car remaining) current-max)
                  (car remaining)
                  current-max))))
  ;; Initialize with first element of the lsit
  (iter (cdr l) (car l)))

(define (list-min l)
  ;; Internal helper for tail recursion
  (define (iter remaining current-min)
    (if (null? remaining)
        current-min
        (iter (cdr remaining)
              (if (< (car remaining) current-min)
                  (car remaining)
                  current-min))))
  (iter (cdr l) (car l)))