;; Test nested labels

foo	equ	4
.bar	equ	5
.baz	equ 	.bar+1
	assert	foo==4
	assert	.bar==5
	assert	.baz==6

bar
.baz	equ	10
.qux	equ	.baz+10
	assert	.baz==10
	assert	.qux==20

	assert	foo.baz != bar.baz

	org	100

lab	db	foo,foo.bar,foo.baz	; 4 5 6
.nlab	db	bar.baz,bar.qux		; 10 20

	assert	lab==100
	assert	lab.nlab==103
