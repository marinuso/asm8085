	org	100h

a100	dw	a100	; at 100
a102	dw	a102	; at 102

	pushorg	200h

a200	dw	a200	; at 104, finally
a202	dw	a202	; at 106, finally

	poporg

a108	dw	a108	; at 108
a10a	dw	a10a	; at 10a

	pushorg	400h

a400	dw	a400	; at 10c, finally
a402	dw	a402	; at 10e, finally

	poporg

a110	dw	a110	; at 110
a112	dw	a112	; at 112

	pushorg 200h

b200	dw	b200	; at 114, finally
b202	dw	b202	; at 116, finally

	poporg

a118	dw	a118	; at 118


	

