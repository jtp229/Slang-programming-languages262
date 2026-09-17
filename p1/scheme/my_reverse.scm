

;; my_reverse: reverse a list without using the scheme `reverse` function
;;
;; Your implementation of this function can use special forms and standard
;; functions, such as `car`, `cdr`, `list`, `append`, and `if`, but it cannot
;; use the built-in `reverse` function.
;;
;; Your implementation should be tail recursive.


  #f ;; [CSE 262] Implement Me!
  (define (my-reverse l)
  (define (reverse-helper lst ltwo)
  ;;define recursive helper function to track whats left of input and our output
  
  ;;for base case, check if our original list is empty and then return list 2.
    (if (null? lst) 
        ltwo
        ;;recurse by taking the tail of the og list using cdr and then adding it to the beginning of the second list 
        ;; using cons
        (reverse-helper (cdr lst) (cons (car lst) ltwo))))
    ;;begin the method with the input list and an empty list 2.
  (reverse-helper l '())

)

