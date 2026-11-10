;;; Convert a Dbl (flonum) value to its canonical string representation.
;;;
;;; Gambit's default `number->string` on a flonum always includes a decimal
;;; point, e.g. 1.0 -> "1." and 3.0 -> "3.". To match the tests, whole-number
;;; flonums must print with NO decimal point at all (1.0 -> "1"), while
;;; non-whole flonums keep their natural/minimal decimal form (2.5 -> "2.5").
;;;
;;; @param val The flonum value
;;;
;;; @return a string representation of val, matching Rust's `format!("{}", d)`
(define (dbl->string val)
  (if (and (real? val) (finite? val) (integer? val))
      (number->string (inexact->exact (round val)))
      (number->string val)))

;;; Escape a string so that it can be an attribute of an XML tag
;;;
;;; @param s The string to escape
;;;
;;; @return The escaped string
(define (xml-escape s)
  ;; Internal recursion helper: `i` is the current index, `res` is the result
  ;; string, len is the length of `s`, which is captured
  ;;
  ;; @param i   The current index
  ;; @param res The accumulated string
  ;; @param len The string length
  ;;
  ;; @return the escaped string
  (define (helper i res len)
    (if (= i len)
        ;; Base case: we've reached the end of the string
        res
        ;; Recurse with the next index and the appended result
        (helper 
         (+ i 1) 
         (string-append res 
                        (cond
                         ((char=? (string-ref s i) #\\)        "\\\\")
                         ((char=? (string-ref s i) #\tab)      "\\t")
                         ((char=? (string-ref s i) #\newline)  "\\n")
                         ((char=? (string-ref s i) #\')        "\\'")
                         ((char=? (string-ref s i) #\&)        "&amp;")
                         ((char=? (string-ref s i) #\<)        "&lt;")
                         ((char=? (string-ref s i) #\>)        "&gt;")
                         ((char=? (string-ref s i) #\")        "&quot;")
                         (#t                                   (string (string-ref s i))))) len)))
  ;; Launch the recursion starting at index 0
  (helper 0 "" (string-length s)))

;;; Unescape a string that was escaped for XML
;;;
;;; @param s The string to unescape
;;;
;;; @return The unescaped string
(define (xml-unescape s)
  ;; Internal recursion helper: `i` is the current index, `res` is the
  ;; accumulated list of characters, `len` is the length of `s`, which is
  ;; captured
  ;;
  ;; @param i   The current index
  ;; @param res The accumulated characters
  ;; @param len The string length
  ;;
  ;; @return the unescaped string
  (define (helper i res len)
    (if (= i len)
        ;; Base case: reached the end, so reverse the accumulated list and convert to string
        (list->string (reverse res))
        (cond
         ;; Handle potential XML entities
         ((char=? (string-ref s i) #\&)
          (cond
           ((and (<= (+ i 4) len) (string=? (substring s i (+ i 4)) "&lt;")) 
            (helper (+ i 4) (cons #\< res) len))
           ((and (<= (+ i 4) len) (string=? (substring s i (+ i 4)) "&gt;"))
            (helper (+ i 4) (cons #\> res) len))
           ((and (<= (+ i 5) len) (string=? (substring s i (+ i 5)) "&amp;"))
            (helper (+ i 5) (cons #\& res) len))
           ((and (<= (+ i 6) len) (string=? (substring s i (+ i 6)) "&quot;"))
            (helper (+ i 6) (cons #\" res) len))
           ;; False alarm, just a regular &
           (#t (helper (+ i 1) (cons c res) len))))
         ;; Handle potential string escapes
         ((char=? (string-ref s i) #\\)
          (if (< (+ i 1) len)
              (let ((next (string-ref s (+ i 1))))
                (cond
                 ((char=? next #\n)  (helper (+ i 2) (cons #\newline res) len))
                 ((char=? next #\t)  (helper (+ i 2) (cons #\tab res) len))
                 ((char=? next #\')  (helper (+ i 2) (cons #\' res) len))
                 ((char=? next #\\)  (helper (+ i 2) (cons #\\ res) len))
                 ;; Unrecognized escape, keep the backslash
                 (#t                 (helper (+ i 1) (cons c res) len))))
              ;; Trailing backslash at the end of the string
              (helper (+ i 1) (cons (string-ref s i) res) len)))
         ;; Standard character
         (#t (helper (+ i 1) (cons (string-ref s i) res) len)))))
  ;; Launch the recursion starting at index 0 with an empty result list
  (helper 0 '() (string-length s)))

;;; Find the first occurrence of a pattern in a string starting at an index
;;;
;;; @param str     The string to search
;;; @param pattern The pattern to find
;;; @param start   The starting index
;;;
;;; @return The index of the first occurrence, or #f if not found
(define (string-index-of str pattern start)
  (let ((len (string-length str))
        (plen (string-length pattern)))
    ;; Helper to recursively check each index
    ;;
    ;; @param i The current index
    ;;
    ;; @return the index if found, or #f
    (define (helper i)
      (cond
       ((> (+ i plen) len) #f)
       ((string=? (substring str i (+ i plen)) pattern) i)
       (#t (helper (+ i 1)))))
    (helper start)))

;;; Check if a string contains a pattern
;;;
;;; @param str     The string to search
;;; @param pattern The pattern to find
;;;
;;; @return #t if found, #f otherwise
(define (string-contains? str pattern)
  (if (string-index-of str pattern 0) #t #f))

;;; Helper to skip whitespace
;;;
;;; @param s The XML string to process
;;; @param i The index to start from (initially zero)
;;;
;;; @return The index of the first non-whitespace character (>= i)
(define (xml-skip-whitespace s i)
  (if (and (< i (string-length s)) (char-whitespace? (string-ref s i)))
      (xml-skip-whitespace s (+ i 1))
      i))

;;; Helper to consume a literal string if it matches
;;;
;;; @param s        The XML string to process
;;; @param i        The index to start from (may be whitespace)
;;; @param pattern  The pattern to search for
;;;
;;; @return The index in `s` where the pattern starts, or #f
(define (xml-consume-on-match s i pattern)
  (let* ((idx (xml-skip-whitespace s i))
         (pi (string-index-of s pattern idx)))
    (if (and pi (= pi idx))
        (+ idx (string-length pattern))
        #f)))

;;; Helper to extract an attribute from a tag
;;;
;;; @param tag    The XML tag in which to search
;;; @param attr   The attribute to search for
;;; @param abort  A closure to call upon any error
;;;
;;; @return The attribute that was found, as a string
(define (xml-get-attr tag attr abort)
  (let* ((search (string-append attr "=\""))
         (start-idx (string-index-of tag search 0)))
    (if (not start-idx)
        (abort (string-append "Attribute not found: " attr " in tag: " tag))
        (let* ((val-start (+ start-idx (string-length search)))
               (end-idx (string-index-of tag "\"" val-start)))
          (if (not end-idx)
              (abort (string-append "Malformed attribute: " attr " in tag: " tag))
              (substring tag val-start end-idx))))))

;;; Helper to get the next XML tag
;;;
;;; @param s      The XML string to process
;;; @param i      The index from which to start searching
;;; @param abort  A closure to call upon any error
;;;
;;; @return a pair (tag . new-index) or #f
(define (xml-get-next-tag s i abort)
  ;; NB: and-let* would be nice here...
  (let ((idx (xml-skip-whitespace s i)))
    (if (>= idx (string-length s))
        #f
        (let ((start (string-index-of s "<" idx)))
          (if (not start)
              #f
              (let ((end (string-index-of s ">" start)))
                (if (not end)
                    (abort "Malformed XML: tag not closed")
                    (cons (substring s start (+ end 1)) (+ end 1)))))))))
