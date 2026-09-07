;; three-zip takes three lists, and returns a single list, where each entry
;; in the returned list is a list with three elements.
;;
;; The nth element of the returned list is a list containing the nth element
;; of the first list, the nth element of the second list, and the nth element
;; of the third list.
;;
;; Your implementation should be tail recursive.
;;
;; If the three lists do not have the same length, then your code should
;; behave as if all of the lists were as long as the longest list, by
;; replicating the last element of each of the short lists.  You may assume
;; that the lists passed into the function are not empty.
;;
;; Example: (three-zip '(1 2 3) '("hi" "bye" "hello") '(a b c))
;;          -> ('(1 "hi" a) '(2 "bye" b) '(3 "hello" c))
;;
;; Example: (three-zip '(1 2 3 4) '("hi" "bye" "hello") '(a b c))
;;          -> ('(1 "hi" a) '(2 "bye" b) '(3 "hello" c) '(4 "hello" c))
(define (three-zip l1 l2 l3)
  #f ;; [CSE 262] Implement Me!
)
