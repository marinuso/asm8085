;; This file tests that ORG, DB, DW, and DS move the instruction
;; location in the right way.


	org	100

at100	db	1,2,3,4
at104	dw	56,78
at108	ds	12	; 12+108=120

at120	;; label on empty line must still be defined.


