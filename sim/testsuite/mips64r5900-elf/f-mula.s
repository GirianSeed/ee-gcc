.include "t-macros.i"

	start

test_mula1:	
	clearfcsr
	loadfp $f1 2.0
	loadfp $f2 4.0
	mula.s $f2, $f1
	checkacc 0 8.0



###########


test_mula_overflow_1:
	clearfcsr
	loadfpx $f1 0x7fffffff
	loadfpx $f2 0x40000000
	mula.s $f2, $f1
	checkaccx 1 0x00008011 0x7fffffff

test_mula_overflow_2:
	clearfcsr
	loadfpx $f1 0x7f0fffff
	loadfpx $f2 0x7f0fffff
	mula.s $f2, $f1
	checkaccx 2 0x00008011 0x7fffffff

test_mula_overflow_3:
	clearfcsr
	loadfpx $f1 0xffffffff
	loadfpx $f2 0x40000000
	mula.s $f2, $f1
	checkaccx 3 0x00008011 0xffffffff

test_mula_overflow_4:
	clearfcsr
	loadfpx $f1 0xff0fffff
	loadfpx $f2 0xff0fffff
	mula.s $f2, $f1
	checkaccx 4 0x00008011 0x7fffffff


###########

test_mula_underflow_1:
	clearfcsr
	loadfpx $f1 0x00800001
	loadfpx $f2 0x00800001
	mula.s $f2, $f1
	checkaccx 1 0x00004009 0x00000000

test_mula_underflow_2:
	clearfcsr
	loadfpx $f1 0x80800001
	loadfpx $f2 0x80800001
	mula.s $f2, $f1
	checkaccx 2 0x00004009 0x00000000

test_mula_underflow_3:
	clearfcsr
	loadfpx $f1 0x00800001
	loadfpx $f2 0x80800001
	mula.s $f2, $f1
	checkaccx 3 0x00004009 0x00000000

test_mula_underflow_4:
	clearfcsr
	loadfpx $f1 0x80800001
	loadfpx $f2 0x00800001
	mula.s $f2, $f1
	checkaccx 4 0x00004009 0x00000000


	exit0
