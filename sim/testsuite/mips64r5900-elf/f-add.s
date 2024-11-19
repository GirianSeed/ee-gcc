.include "t-macros.i"

	start

test_add1:	
	clearfcsr
	loadfp $f1 4.0
	loadfp $f2 0.1
	add.s $f3, $f2, $f1
	checkfp 0 $f3 4.1

test_add2:
	clearfcsr
	loadfp $f1 , -4.0
	loadfp $f2 8.0
	add.s $f3, $f2, $f1
	checkfp 0 $f3 4.0

test_add3:
	clearfcsr
	loadfpmax $f1
	loadfpmax $f2
	add.s $f3, $f2, $f1
	checkfpmax FCSR_O $f3

test_add4:
	clearfcsr
	loadfpmax $f1
	neg.s $f1, $f1
	loadfpmax $f2
	neg.s $f2, $f2
	add.s $f3, $f2, $f1
	checkfcsr FCSR_O
	clearfcsr
	neg.s $f3, $f3
	checkfpmax 0 $f3

	clearfcsr
	loadfpx $f1 0x7fffffff
	loadfpx $f2 0x7fffffff
	add.s $f3, $f2, $f1
	checkfpx 11 0x00008011 $f3 0x7fffffff

	clearfcsr
	loadfpx $f1 0x7f7fffff
	loadfpx $f2 0x7f8fffff
	add.s $f3, $f2, $f1
	checkfpx 12 0x00008011 $f3 0x7fffffff

	clearfcsr
	loadfpx $f1 0x80000000
	loadfpx $f2 0x00000000
	add.s $f3, $f2, $f1
	checkfpx 13 0x00000001 $f3 0x00000000

	clearfcsr
	loadfpx $f1 0x00000000
	loadfpx $f2 0x00000000
	add.s $f3, $f2, $f1
	checkfpx 14 0x00000001 $f3 0x00000000


	clearfcsr
	loadfpx $f1 0xffffffff
	loadfpx $f2 0xffffffff
	add.s $f3, $f2, $f1
	checkfpx 15 0x00008011 $f3 0xffffffff

	clearfcsr
	loadfpx $f1 0xff7fffff
	loadfpx $f2 0xff8fffff
	add.s $f3, $f2, $f1
	checkfpx 16 0x00008011 $f3 0xffffffff

	clearfcsr
	loadfpx $f1 0x00000000
	loadfpx $f2 0x80000000
	add.s $f3, $f2, $f1
	checkfpx 17 0x00000001 $f3 0x00000000

	clearfcsr
	loadfpx $f1 0x00000000
	loadfpx $f2 0x00000000
	add.s $f3, $f2, $f1
	checkfpx 18 0x00000001 $f3 0x00000000


	clearfcsr
	loadfpx $f1 0x00800000
	loadfpx $f2 0x80080001
	add.s $f3, $f2, $f1
	checkfpx 21 0x00000001 $f3 0x00800000

	clearfcsr
	loadfpx $f1 0x80000000
	loadfpx $f2 0x00000000
	add.s $f3, $f2, $f1
	checkfpx 22 0x00000001 $f3 0x00000000

	clearfcsr
	loadfpx $f1 0x00000000
	loadfpx $f2 0x00000000
	add.s $f3, $f2, $f1
	checkfpx 23 0x00000001 $f3 0x00000000


	clearfcsr
	loadfpx $f1 0x80800000
	loadfpx $f2 0x00080001
	add.s $f3, $f2, $f1
	checkfpx 24 0x00000001 $f3 0x80800000

	clearfcsr
	loadfpx $f1 0x80000000
	loadfpx $f2 0x00000000
	add.s $f3, $f2, $f1
	checkfpx 25 0x00000001 $f3 0x00000000

	clearfcsr
	loadfpx $f1 0x00000000
	loadfpx $f2 0x00000000
	add.s $f3, $f2, $f1
	checkfpx 26 0x00000001 $f3 0x00000000

	exit0
