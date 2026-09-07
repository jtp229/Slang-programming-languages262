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
  #f ;; [CSE 262] Implement Me!
)
