.include "t-macros.i"

	start

        load $8 0x00ff0000ffff0000 0x0000ffffffff0000
        load $9 0x00ff0000ffff0000 0x0000ffffffff0000
test_psrlvw:	
        psrlvw  $10,$8,$9       #Expectation:
        check10 0xffffffffffff0000 0xffffffffffff0000

        load $8 0x00ff00007fff0000 0x0000ffff7beef000
        load $9 0x00ff0000ffff0000 0x0000ffffffff0004
test_psrlvw2:	
        psrlvw  $10,$8,$9       #Expectation:
        check10 0x000000007fff0000 0x0000000007beef00

	exit0
