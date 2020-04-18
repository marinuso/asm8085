	;; This tests the 'align' directive

	db	$aa
	align	4
	dw	$bbaa
	align	4,1
	dw	$bbaa
	db	$cc
	align	4,2
	dw	$bbaa,$ddcc
	align	4,56
	dw	$bbaa,$ddcc
	align	8,3
	dw	$ffee
