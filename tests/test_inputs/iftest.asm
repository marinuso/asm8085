;; This file tests the various conditions

foo	equ	42	; 'foo' is defined (and 'bar' is not)

	if foo == 42
qux		equ	10 ; this should happen
	endif

spam	equ	100	; this should be incldued

	if foo == 50
qux		equ	11 ; this should not
	endif

	ifdef foo
ham		equ	20 ; this should be included
	endif

	ifdef bar
ham		equ	21 ; this should not
	endif

	ifndef bar
eggs		equ	30 ; this should be included 
	endif

	ifndef foo
eggs		equ	31 ; this should not
	endif

bar	equ	47	; but we can define 'bar' afterwards

	
