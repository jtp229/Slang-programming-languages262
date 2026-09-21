;; substring-wildcard is like contains-substring, but it understands
;; single-character wildcards in the pattern string.  Wildcards are
;; represented by the ? character.  Note that this is a slightly broken way
;; of doing wildcards: the '?' character can never be matched literally.
;;
;; Example:
;; (substring-wildcard "hello" "e?lo") ; returns #t
;; (substring-wildcard "hello" "yell") ; returns #f
;; (substring-wildcard "The quick brown fox jumps over lazy dogs" "q?ick") ; returns #t
;;
;; You should implement this by comparing one character at a time, and
;; should not use any string-searching or string-comparison functions
;; provided by gsi.
;; TODO: implement this function
(define (substring-wildcard source pattern)
  #f ;; [CSE 262] Implement Me!
)
