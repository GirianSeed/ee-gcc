.include "t-macros.i"

	start

test_prot3w:	
        load $8 0x1111111133333333 0x4444444422222222
        prot3w  $10,$8       #Expectation:

        check10 0x1111111122222222 0x3333333344444444

	exit0
