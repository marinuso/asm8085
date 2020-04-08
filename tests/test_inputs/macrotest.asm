
	;; Define three macros

add_one	macro	n,v
!n	equ	(!v) + 1
	endm

add_two	macro	n,v
!n	equ	(!v) + 2
	endm

mul_two	macro	n,v
!n	equ	(!v) * 2
	endm

	;; Expand the macros

	add_one	three,2
	add_two	four,2
	mul_two	six,three

	add_one	ten,9
	add_two	twelve,ten
	mul_two	twentyfour,twelve


