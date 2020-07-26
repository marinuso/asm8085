	; Macro: write N bytes of N digits
rep:	macro	n
	 repeat	!n
	  db	'0' + !n
	 endr
	endm

	rep	0 ; 1 2 2 3 3 3 4 4 4 4 ...
	rep	1
	rep	2
	rep	3
	rep	4
	rep	5
	rep	6
	rep	7
	rep	8
	rep	9
	
	; Test nested repeats
	repeat	5	; XYYZ XYYZ XYYZ XYYZ XYYZ XYYZ
	 db	'X'
	 repeat	2
	  db	 'Y'
	 endr
         db     'Z'
	endr

