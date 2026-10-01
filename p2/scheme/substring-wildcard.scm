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
    (let ((slen (string-length source))
        (plen (string-length pattern)))

    ;; Checks whether pattern lines up with source starting at start
    (define (matches-at? start i)
      (cond ((= i plen) #t)  ; every pattern char matched, so it's a hit
            ;;i literally just copy and pasted my contains substring code and added a case for ?
             ((char=? (string-ref pattern i) #\?)
             (matches-at? start (+ i 1)))


            ;; Compare single characters with char=? 
            ((char=? (string-ref source (+ start i))
                     (string-ref pattern i))
             (matches-at? start (+ i 1)))
            ;; First mismatch bail out early
            (else #f)))

    ;; Slides the pattern across source one position at a time
    (define (try start)
      ;; Stop once fewer than plen characters remain as a match cant fit
      (cond ((> (+ start plen) slen) #f)
            ((matches-at? start 0) #t)
            (else (try (+ start 1)))))
    (try 0)))

