;; This file tests whether expressions using labels are evaluated
;; properly, and that forward references are resolved properly. 

;; This should leave 'qux' set to 3
qux	equ	bar+baz
baz	equ	foo+bar
bar	equ	foo
foo	equ	1


;; "spam", "ham", and "eggs" should remain unknown, this should not cause
;; an infinite loop. 
spam	equ	ham
ham	equ	spam
eggs	equ	eggs
