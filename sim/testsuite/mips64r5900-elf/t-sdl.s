.include "t-macros.i"

	start

	.align 3
	.data
byteaddr:	.word bytes
	.align 7
bytes:	
	.byte 0xb0
	.byte 0xb1
	.byte 0xb2
	.byte 0xb3
	.byte 0xb4
	.byte 0xb5
	.byte 0xb6
	.byte 0xb7
	.byte 0xb8
	.byte 0xb9
	.byte 0xba
	.byte 0xbb
	.byte 0xbc
	.byte 0xbd
	.byte 0xbe
	.byte 0xbf

	.text
	ld $8, byteaddr

	
test_sdl_0:
	load $10  0xdeadbeefdeadbeef 0xb0cccccccccccccc
	sdl $10, 0($8)
	ld $10, 0($8)
	check10 0xdeadbeefdeadbeef 0xb7b6b5b4b3b2b1b0
	ld $10, 8($8)
	check10 0xdeadbeefdeadbeef 0xbfbebdbcbbbab9b8

test_sdl_1:
	load $10  0xdeadbeefdeadbeef 0xb1b0cccccccccccc
	sdl $10, 1($8)
	ld $10, 0($8)
	check10 0xdeadbeefdeadbeef 0xb7b6b5b4b3b2b1b0
	ld $10, 8($8)
	check10 0xdeadbeefdeadbeef 0xbfbebdbcbbbab9b8

test_sdl_2:
	load $10  0xdeadbeefdeadbeef 0xb2b1b0cccccccccc
	sdl $10, 2($8)
	ld $10, 0($8)
	check10 0xdeadbeefdeadbeef 0xb7b6b5b4b3b2b1b0
	ld $10, 8($8)
	check10 0xdeadbeefdeadbeef 0xbfbebdbcbbbab9b8

test_sdl_3:
	load $10  0xdeadbeefdeadbeef 0xb3b2b1b0cccccccc
	sdl $10, 3($8)
	ld $10, 0($8)
	check10 0xdeadbeefdeadbeef 0xb7b6b5b4b3b2b1b0
	ld $10, 8($8)
	check10 0xdeadbeefdeadbeef 0xbfbebdbcbbbab9b8

test_sdl_4:
	load $10  0xdeadbeefdeadbeef 0xb4b3b2b1b0cccccc
	sdl $10, 4($8)
	ld $10, 0($8)
	check10 0xdeadbeefdeadbeef 0xb7b6b5b4b3b2b1b0
	ld $10, 8($8)
	check10 0xdeadbeefdeadbeef 0xbfbebdbcbbbab9b8

test_sdl_5:
	load $10  0xdeadbeefdeadbeef 0xb5b4b3b2b1b0cccc
	sdl $10, 5($8)
	ld $10, 0($8)
	check10 0xdeadbeefdeadbeef 0xb7b6b5b4b3b2b1b0
	ld $10, 8($8)
	check10 0xdeadbeefdeadbeef 0xbfbebdbcbbbab9b8

test_sdl_6:
	load $10  0xdeadbeefdeadbeef 0xb6b5b4b3b2b1b0cc
	sdl $10, 6($8)
	ld $10, 0($8)
	check10 0xdeadbeefdeadbeef 0xb7b6b5b4b3b2b1b0
	ld $10, 8($8)
	check10 0xdeadbeefdeadbeef 0xbfbebdbcbbbab9b8

test_sdl_7:
	load $10  0xdeadbeefdeadbeef 0xb7b6b5b4b3b2b1b0
	sdl $10, 7($8)
	ld $10, 0($8)
	check10 0xdeadbeefdeadbeef 0xb7b6b5b4b3b2b1b0
	ld $10, 8($8)
	check10 0xdeadbeefdeadbeef 0xbfbebdbcbbbab9b8

	
test_sdl_8:
	load $10  0xdeadbeefdeadbeef 0xb8cccccccccccccc
	sdl $10, 8($8)
	ld $10, 0($8)
	check10 0xdeadbeefdeadbeef 0xb7b6b5b4b3b2b1b0
	ld $10, 8($8)
	check10 0xdeadbeefdeadbeef 0xbfbebdbcbbbab9b8

test_sdl_9:
	load $10  0xdeadbeefdeadbeef 0xb9b8cccccccccccc
	sdl $10, 9($8)
	ld $10, 0($8)
	check10 0xdeadbeefdeadbeef 0xb7b6b5b4b3b2b1b0
	ld $10, 8($8)
	check10 0xdeadbeefdeadbeef 0xbfbebdbcbbbab9b8

test_sdl_10:
	load $10  0xdeadbeefdeadbeef 0xbab9b8cccccccccc
	sdl $10, 10($8)
	ld $10, 0($8)
	check10 0xdeadbeefdeadbeef 0xb7b6b5b4b3b2b1b0
	ld $10, 8($8)
	check10 0xdeadbeefdeadbeef 0xbfbebdbcbbbab9b8

test_sdl_11:
	load $10  0xdeadbeefdeadbeef 0xbbbab9b8cccccccc
	sdl $10, 11($8)
	ld $10, 0($8)
	check10 0xdeadbeefdeadbeef 0xb7b6b5b4b3b2b1b0
	ld $10, 8($8)
	check10 0xdeadbeefdeadbeef 0xbfbebdbcbbbab9b8

test_sdl_12:
	load $10  0xdeadbeefdeadbeef 0xbcbbbab9b8cccccc
	sdl $10, 12($8)
	ld $10, 0($8)
	check10 0xdeadbeefdeadbeef 0xb7b6b5b4b3b2b1b0
	ld $10, 8($8)
	check10 0xdeadbeefdeadbeef 0xbfbebdbcbbbab9b8

test_sdl_13:
	load $10  0xdeadbeefdeadbeef 0xbdbcbbbab9b8cccc
	sdl $10, 13($8)
	ld $10, 0($8)
	check10 0xdeadbeefdeadbeef 0xb7b6b5b4b3b2b1b0
	ld $10, 8($8)
	check10 0xdeadbeefdeadbeef 0xbfbebdbcbbbab9b8

test_sdl_14:
	load $10  0xdeadbeefdeadbeef 0xbebdbcbbbab9b8cc
	sdl $10, 14($8)
	ld $10, 0($8)
	check10 0xdeadbeefdeadbeef 0xb7b6b5b4b3b2b1b0
	ld $10, 8($8)
	check10 0xdeadbeefdeadbeef 0xbfbebdbcbbbab9b8

test_sdl_15:
	load $10  0xdeadbeefdeadbeef 0xbfbebdbcbbbab9b8
	sdl $10, 15($8)
	ld $10, 0($8)
	check10 0xdeadbeefdeadbeef 0xb7b6b5b4b3b2b1b0
	ld $10, 8($8)
	check10 0xdeadbeefdeadbeef 0xbfbebdbcbbbab9b8

	exit0
