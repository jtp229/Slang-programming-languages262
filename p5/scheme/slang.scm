(include "astnodes.scm")
(include "parser_xml.scm")
(include "typechecker.scm")
(include "optimizer.scm")

;;; Print a help message
(define (print-help)
  (display "slang -- An interpreter for a Scheme-like language (Scheme version)") (newline)
  (display "  Usage: slang [mode] [filename]") (newline)
  (display "  Modes:") (newline)
  (display "    -help              Display this message and exit") (newline)
  (display "    -typecheck         Typecheck XML AST") (newline)
  (display "    -optimize          Typecheck, then optimize XML AST") (newline))

;;; Process the command-line arguments
;;;
;;; @param args       a list of strings representing the command-line arguments
;;; @param mode       the current guess for the mode (initially "")
;;; @param filename   the filename to process (initially "")
;;; @param num-modes  number of modes processed so far (initially 0)
;;; @param num-files  number of files processed so far (initially 0)
;;;
;;; @return a Vector with the mode and filename
(define (process-args args filename mode num-modes num-files)
  (if (null? args)
      ;; null args means either no args or end of recursion, so check args here
      (if (or (not (= num-modes 1)) (not (= num-files 1)) (string=? mode "-help"))
          (vector "-help" "")
          (vector mode filename))
      ;; non-null args means we handle a mode or filename and recurse
      (cond
       ((member (car args) '("-help" "-typecheck" "-optimize"))
        (process-args (cdr args) filename (car args) (+ num-modes 1) num-files))
       (#t
        (process-args (cdr args) (car args) mode num-modes (+ num-files 1))))))

;;; Load a file as a string.  If the file is EOF, return an empty string
;;;
;;; @param filename The name of the file to load
;;;
;;; @return the contents of the file as a string
(define (load-file filename)
  (let ((contents (call-with-input-file filename (lambda (p) (read-line p #f)))))
    (if (eof-object? contents) "" contents)))

;;; Process a single file and then exit
;;;
;;; @param filename The filename to process
;;; @param mode     The mode of operation
(define (process-file filename mode)
  (if (file-exists? filename)
      (let ((content (load-file filename)))
        (cond
         ((string=? mode "-typecheck")
          (let ((nodes (xml-to-ast content)))
            (typecheck-report nodes (make-type-env (find-unstable-names nodes)))))

         ((string=? mode "-optimize")
          (optimize-and-print (xml-to-ast content)))
         ;; NB: No fallthrough because we pre-validated modes
         ))
      ;; Otherwise print an error and return
      (begin (display "Error: file not found" (current-error-port)) (newline))))

;;; Run the Scheme interpreter
;;;
;;; @param args The command-line arguments to the program (excluding `gsi` and
;;;             `slang.scm`)
(define (main args)
  ;; Process the command-line args
  (let ((args (process-args args "" "" 0 0)))
    (if (string=? (vector-ref args 0) "-help")
        (print-help)
        (process-file (vector-ref args 1) (vector-ref args 0)))))

;; Start the program by calling main with the command line arguments
;; (excluding the program name itself)
(main (cdr (cdr (command-line))))
(exit)
