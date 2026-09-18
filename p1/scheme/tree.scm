;; tree: A binary tree, implemented as a "closure"
;;
;; The tree should support the following methods:
;; - 'ins x      - Insert the value x into the tree
;; - 'clear      - Reset the tree to empty
;; - 'inslist l  - Insert all the elements from list `l` into the tree
;; - 'display    - Use `display` to print the tree
;; - 'inorder f  - Traverse the tree using an in-order traversal, applying
;;                 function `f` to the value in each non-null position
;; - 'preorder f - Traverse the tree using a pre-order traversal, applying
;;                 function `f` to the value in each non-null position
;;
;; Note: every method should take two arguments (the method name and a
;; parameter).  If a method is defined as not using any parameters, you
;; should still require a parameter, but your code can ignore it.
;;
;; Note: You should implement the tree as a closure.  One of the simplest
;; examples of a closure that acts like an object is the following:
;;
;; (define (make-my-ds)
;;   (let ((x '())) (lambda (msg arg)
;;       (cond ((eq? msg 'set) (set! x arg) 'ok) ((eq? msg 'get) x) (else 'error)))))
;;
;; In that example, I have intentionally *not* commented anything.  You will
;; need to figure out what is going on there: how does calling the closure
;; with different `msg` values change what it does, and how does `set!`
;; let it "remember" things between calls?
;;
;; Once your `make-bst` is working, here's a concrete example for checking
;; that you have 'inorder and 'preorder right, since it's easy to mix them
;; up.  Inserting 5, 3, 8, 1, 4 (in that order) produces this tree:
;;
;;            5
;;          /   \
;;         3     8
;;        / \
;;       1   4
;;
;; 'inorder visits a node's left subtree, then the node itself, then its
;; right subtree.  For a BST, that always means visiting values in sorted
;; order:
;;
;; (define t (make-bst))
;; (t 'inslist '(5 3 8 1 4))
;; (t 'inorder (lambda (v) (display v) (display " ")))   ; prints: 1 3 4 5 8
;;
;; 'preorder visits a node itself first, then its left subtree, then its
;; right subtree.  The first value it visits is always the root:
;;
;; (t 'preorder (lambda (v) (display v) (display " ")))  ; prints: 5 3 1 4 8
;;
;; Your implementation should be *clean*.  That is, the only global symbol
;; exported by this file should be the `make-bst` function.

(define (make-bst)
  (define (insert tree val)
  ;;helper function for insert
    (cond
    ;;if tree is empty make a node with two empty subtrees
      ((null? tree) (list val '() '()))
      ;;if the value is less then the root, keep the root and right tree the same, add value to left tree
      ((< val (car tree)) (list (car tree) (insert (cadr tree) val) (caddr tree)))
      ;;if the value is greater then the root, insert it into the right tree, leave the left and root alone
      ((> val (car tree)) (list (car tree) (cadr tree) (insert (caddr tree) val)))
      ;;else we have the root
      (else tree)))
  
  (define (insert-all tree lst)
  ;;take a tree and list of values and insert them
    (if (null? lst)
    ;;if list of values is empty were done inserting, return the tree
        tree
        ;;insert 1st element into tree then recursively insert rest of list insto tree
        (insert-all (insert tree (car lst)) (cdr lst))))
  ;;display the tree
  (define (display-tree tree)
    (if (null? tree)
    ;;if tree is empty print nothing
        (display "()")
        (begin
        ;;recursively print 
          (display "(")
          (display (car tree))
          ;;root value
          (display " ")
          (display-tree (cadr tree))
          ;;left subtree
          (display " ")
          ;;right subtree
          (display-tree (caddr tree))
          (display ")"))))
  ;;in order traverse LEFT NODE RIGHT
  (define (inorder-traverse tree f)

    (if (not (null? tree))
    ;;if the tree isnt empty, recursively go through left subtree
        (begin
          (inorder-traverse (cadr tree) f)
          ;;apply function to node value
          (f (car tree))
          ;;go through left subtree
          (inorder-traverse (caddr tree) f))))
  ;;traverse with function NODE LEFT RIGHT
  (define (preorder-traverse tree f)
    (if (not (null? tree))
        (begin
        ;;apply function to current node
          (f (car tree))
          ;;then left tree
          (preorder-traverse (cadr tree) f)
       
          (preorder-traverse (caddr tree) f))))
      
  
  (let ((root '()))
  ;;hold tree data
  ;;message and argument as parameters
    (lambda (msg arg)
      (cond
      ;;message for inserting a single value
        ((eq? msg 'ins)
         (set! root (insert root arg))
         ;;update root and return ok when were done
         'ok)
         ;;make root empty return ok
        ((eq? msg 'clear)
         (set! root '())
         'ok)
         ;;insert all values from list
        ((eq? msg 'inslist)
         (set! root (insert-all root arg))
         'ok)
         ;;print tree using display
        ((eq? msg 'display)
         (display-tree root)
         'ok)
         ;;traverse in order and apply arg to each node
        ((eq? msg 'inorder)
         (inorder-traverse root arg)
         'ok)
         ;;traverse in preorder and apply arg
        ((eq? msg 'preorder)
         (preorder-traverse root arg)
         'ok)
         ;;if nothing matches throw an error
        (else 'error)))))
