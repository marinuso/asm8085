;; Test whether lines are split correctly

	dcr a! dcr b! dcr c! dcr d! dcr e
	dw 1234h! db "A! B! C! D!"! dw 5678h
	! ! ! ! ! ; Not an error
	db	'ABCDEFG'
	