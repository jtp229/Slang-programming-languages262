;; contains-substring checks if a string contains the given substring.  It
;; does not count how many times: it merely returns true or false.
;;
;; The first argument to contains-substring is the string to search.
;; The second argument to contains-substring is the substring to try and
;; find.
;;
;; You should implement this by comparing one character at a time, and
;; should not use any string-searching or string-comparison functions
;; provided by gsi.
;;
;; Example:
;; (contains-substring "hello" "ello") ; returns #t
;; (contains-substring "hello" "yell") ; returns #f
;; (contains-substring "The quick brown fox jumps over lazy dogs" "ox") ; returns #t
;; TODO: implement this function
(define (contains-substring source pattern)
  ;; get lengths of source string and substring we are lookin for
  (let ((slen (string-length source))
        (plen (string-length pattern)))

    ;; Checks whether pattern lines up with source starting at start
    (define (matches-at? start i)
      (cond ((= i plen) #t)  ; every pattern char matched, so it's a hit
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