	;; This file tests every 8080 opcode
	
	org	100h
	
	;; 00-0F
	nop
foo	lxi	b,bar
	stax	b
	inx	b
	inr	b
	dcr	b
	mvi	b,11h
	rlc
	dad	b
	ldax	b
	dcx	b
	inr	c
	dcr	c
	mvi	c,22h
	rrc
	;; 10-1F
bar	lxi	d,foo
	stax	d
	inx	d
	inr	d
	dcr	d
	mvi	d,33h
	ral
	dad	d
	ldax 	d
	dcx	d
	inr	e
	dcr	e
	mvi	e,44h
	rar
	;; 20-2F
baz	lxi	h,qux
	shld	foo
	inx	h
	inr	h
	dcr	h
	mvi	h,55h
	daa
	dad	h
	lhld	bar
	dcx	h
	inr	l
	dcr	l
	mvi	l,66h
	cma
	;; 30-3F
qux	lxi	sp,baz
	sta	bar
	inx	sp
	inr	m
	dcr	m
	mvi	m,77h
	stc
	dad	sp
	lda	baz
	dcx	sp
	inr	a
	dcr	a
	mvi	a,88h
	cmc
	;; 40-4F
	mov	b,b
	mov	b,c
	mov	b,d
	mov	b,e
	mov	b,h
	mov	b,l
	mov	b,m
	mov 	b,a
	mov 	c,b
	mov 	c,c
	mov	c,d
	mov	c,e
	mov	c,h
	mov	c,l
	mov	c,m
	mov	c,a 
	;; 50-5F
	mov	d,b
	mov	d,c
	mov	d,d
	mov	d,e 
	mov	d,h
	mov	d,l
	mov	d,m
	mov	d,a
	mov	e,b
	mov	e,c
	mov	e,d
	mov	e,e
	mov	e,h
	mov	e,l
	mov	e,m
	mov	e,a
	;; 60-6F
	mov	h,b
	mov	h,c
	mov	h,d
	mov	h,e
	mov	h,h
	mov	h,l
	mov	h,m
	mov 	h,a
	mov 	l,b 
	mov	l,c
	mov	l,d
	mov	l,e
	mov	l,h
	mov 	l,l
	mov	l,m
	mov	l,a
	;; 70-7F
	mov	m,b
	mov	m,c
	mov	m,d
	mov	m,e
	mov	m,h
	mov	m,l
	hlt
	mov	m,a
	mov	a,b
	mov	a,c
	mov	a,d
	mov	a,e
	mov	a,h
	mov	a,l
	mov	a,m
	mov	a,a
	;; 80-8F
	add	b
	add	c
	add	d
	add	e
	add	h
	add 	l
	add 	m
	add	a
	adc 	b
	adc 	c
	adc 	d
	adc 	e
	adc 	h
	adc 	l
	adc	m
	adc 	a
	;; 90-9F
	sub	b
	sub	c
	sub	d
	sub	e
	sub	h
	sub	l
	sub	m
	sub	a
	sbb 	b
	sbb	c
	sbb	d
	sbb	e
	sbb	h
	sbb	l
	sbb	m
	sbb	a
	;; A0-AF
	ana	b
	ana	c
	ana	d
	ana	e
	ana	h
	ana	l
	ana	m
	ana 	a
	xra	b
	xra 	c
	xra	d
	xra	e
	xra	h
	xra 	l
	xra	m
	xra	a
	;; B0-BF
	ora	b
	ora	c
	ora	d
	ora	e
	ora	h
	ora	l
	ora	a
	cmp	b
	cmp	c
	cmp	d
	cmp 	e
	cmp	h
	cmp	l
	cmp 	m
	cmp	a
	;; C0-CF
	rnz
	pop	b
	jnz	foo
	jmp	foo
	cnz	foo
	push	b
	adi	99h
	rst	0
	rz
	ret
	jz	foo
	cz	foo
	call	foo
	aci	0aah
	rst	1
	;; D0-DF
	rnc
	pop	d
	jnc	foo
	out	0bbh
	cnc	foo
	push	d
	sui	0cch
	rst	2
	rc
	jc	foo
	in	0ddh
	cc	foo
	sbi	0eeh
	rst	3
	;; E0-EF
	rpo
	pop	h
	jpo	foo
	xthl
	cpo	foo
	push	h
	ani	0ffh
	rst	4
	rpe
	pchl
	jpe	foo
	xchg
	cpe	foo
	xri	0f0h
	rst	5
	;; F0-FF
	rp
	pop	psw
	jp	foo
	di
	cp	foo
	push	psw
	ori	0e1h
	rst	6
	rm
	sphl
	jm	foo
	ei
	cm	foo
	cpi	0d2h
	rst	7
	
	