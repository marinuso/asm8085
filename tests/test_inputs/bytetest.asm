	;; Test byte and word output 

	org	0

	db	1,2,3,4,5,6,7
	db	"Waar is Berend Botje gebleven?"
	db	-128, 127, 0, 255

	dw	0, -128, 127, 255
	dw	-129, 256, -32768, 32767, 65535

