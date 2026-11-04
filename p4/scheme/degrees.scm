;; degrees and radians: convert an angle from radians to degrees, and back.
;; Use the constant `pi`.  A half-turn is pi radians, or 180 degrees.

(load "slang_compat.scm")

(define (degrees r)

  ;;multiply r by (180 / pi)
  (* r (/ 180 pi))
)

(define (radians d)
  ;;multiply d by (pi / 180)
  (* d(/ pi 180))
)