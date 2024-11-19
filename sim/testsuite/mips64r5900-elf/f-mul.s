.include "t-macros.i"

	start

test_mul1:	
	clearfcsr
	loadfp $f1 2.0
	loadfp $f2 4.0
	mul.s $f3, $f2, $f1
	checkfp 0 $f3 8.0


###########


test_mul_overflow_1:
	clearfcsr
	loadfpx $f1 0x7fffffff
	loadfpx $f2 0x40000000
	mul.s $f3, $f2, $f1
	checkfpx 1 0x00008011 $f3 0x7fffffff

test_mul_overflow_2:
	clearfcsr
	loadfpx $f1 0x7f0fffff
	loadfpx $f2 0x7f0fffff
	mul.s $f3, $f2, $f1
	checkfpx 2 0x00008011 $f3 0x7fffffff

test_mul_overflow_3:
	clearfcsr
	loadfpx $f1 0xffffffff
	loadfpx $f2 0x40000000
	mul.s $f3, $f2, $f1
	checkfpx 3 0x00008011 $f3 0xffffffff

test_mul_overflow_4:
	clearfcsr
	loadfpx $f1 0xff0fffff
	loadfpx $f2 0xff0fffff
	mul.s $f3, $f2, $f1
	checkfpx 4 0x00008011 $f3 0x7fffffff


###########

test_mul_underflow_1:
	clearfcsr
	loadfpx $f1 0x00800001
	loadfpx $f2 0x00800001
	mul.s $f3, $f2, $f1
	checkfpx 1 0x00004009 $f3 0x00000000

test_mul_underflow_2:
	clearfcsr
	loadfpx $f1 0x80800001
	loadfpx $f2 0x80800001
	mul.s $f3, $f2, $f1
	checkfpx 2 0x00004009 $f3 0x00000000

test_mul_underflow_3:
	clearfcsr
	loadfpx $f1 0x00800001
	loadfpx $f2 0x80800001
	mul.s $f3, $f2, $f1
	checkfpx 3 0x00004009 $f3 0x00000000

test_mul_underflow_4:
	clearfcsr
	loadfpx $f1 0x80800001
	loadfpx $f2 0x00800001
	mul.s $f3, $f2, $f1
	checkfpx 4 0x00004009 $f3 0x00000000

	exit0
