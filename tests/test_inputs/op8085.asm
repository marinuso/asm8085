	;; This file tests the 8085-specific opcodes.
	
	org	1000h
	
foo	dsub		;08h
	arhl		;10h
	rdel		;18h
	rim		;20h
	ldhi	11h	;28h (+ 11h)
	sim		;30h
	ldsi	22h	;38h (+ 22h)
	
	rstv		;CBh
	shlx		;D9h
	jnk	foo	;DDh (+ 00h 10h)
	lhlx		;EDh
	jk	foo	;FDh (+ 00h 10h)
	