.include "t-macros.i"

	start

test_ppacw:	
	load $8 0x0010000F000E000D 0x000C000B000A0009
	load $9 0x0008000700060005 0x0004000300020001
        ppacw   $10, $8, $9
	check10 0x000E000D000A0009 0x0006000500020001


	exit0
