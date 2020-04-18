	;; This tests the 'incbin' directive
	;; Since the test routine matches `X.asm' to `X.bin', 
	;; including `incbin_test.bin' should result in a file
	;; that is exactly equal to `incbin_test.bin'

	incbin	"incbin_test.bin"

