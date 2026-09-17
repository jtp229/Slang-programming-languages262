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
   (define (reverse-list lst)
    (define (rev-helper lst acc)
    ;;need to reverse the sum of the three lists because we use tail recursion
      (if (null? lst)
      ;;base case if the list is null return the accumulated list
          acc
          (rev-helper (cdr lst) (cons (car lst) acc))))
    (rev-helper lst '()))
  
  (define (helper l1 l2 l3 last1 last2 last3 acc)
  ;;l1 l2 and l3 are the remaing elements in each list
  ;;last1 2 and 3 are for if the entries in one of the lists run out we can use it for the rest of the other lists
    (if (and (null? l1) (null? l2) (null? l3))
    ;;if the originals are empty move to reverse the list
        (reverse-list acc)
        ;;e1 e2 e3 are the indexes from each list were taking off
        (let ((e1 (if (null? l1) last1 (car l1)))
              (e2 (if (null? l2) last2 (car l2)))
              (e3 (if (null? l3) last3 (car l3))))
              ;;use the next element in the list if there is one, if the og list is blank use whatever the last entry is
          (helper (if (null? l1) '() (cdr l1))
                  (if (null? l2) '() (cdr l2))
                  (if (null? l3) '() (cdr l3))
                  e1 e2 e3
                  ;;make a list of lists with each entry being e1 e2 and e3
                  (cons (list e1 e2 e3) acc)))))
  
  (helper l1 l2 l3 (car l1) (car l2) (car l3) '()))
  ;;recurse 