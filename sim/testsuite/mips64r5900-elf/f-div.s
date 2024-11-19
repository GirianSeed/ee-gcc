.include "t-macros.i"

	start

test_div1:	
	clearfcsr
	loadfp $f1 2.0
	loadfp $f2 4.0
	div.s $f3, $f2, $f1
	checkfp 0 $f3 2.0

	
test_div2:
	clearfcsr
	loadfp $f1 0.0
	loadfp $f2 0.0
	div.s $f3, $f2, $f1
	checkfpmax FCSR_I $f3

test_div3:
	clearfcsr
	loadfp $f1 0.0
	loadfp $f2 1.0
	div.s $f3, $f2, $f1
	checkfpmax FCSR_D $f3
	
test_divx:	

	loadfpx $f1 0x00000000
	loadfpx $f2 0x00000000
	clearfcsr
	div.s $f3, $f1, $f2
	checkfpx 11 0x00020041 $f3 0x7fffffff

	loadfpx $f1 0x80000000
	loadfpx $f2 0x80000000
	clearfcsr
	div.s $f3, $f1, $f2
	checkfpx 12 0x00020041 $f3 0x7fffffff

	loadfpx $f1 0x00000000
	loadfpx $f2 0x80000000
	clearfcsr
	div.s $f3, $f1, $f2
	checkfpx 13 0x00020041 $f3 0xffffffff

	loadfpx $f1 0x80000000
	loadfpx $f2 0x00000000
	clearfcsr
	div.s $f3, $f1, $f2
	checkfpx 14 0x00020041 $f3 0xffffffff


	loadfpx $f1 0x00800000
	loadfpx $f2 0x00000000
	clearfcsr
	div.s $f3, $f1, $f2
	checkfpx 21 0x00010021 $f3 0x7fffffff

	loadfpx $f1 0x80800000
	loadfpx $f2 0x00000000
	clearfcsr
	div.s $f3, $f1, $f2
	checkfpx 22 0x00010021 $f3 0xffffffff

	loadfpx $f1 0x00800000
	loadfpx $f2 0x80000000
	clearfcsr
	div.s $f3, $f1, $f2
	checkfpx 23 0x00010021 $f3 0xffffffff

	loadfpx $f1 0x80800000
	loadfpx $f2 0x80000000
	clearfcsr
	div.s $f3, $f1, $f2
	checkfpx 24 0x00010021 $f3 0x7fffffff


	loadfpx $f1 0x00800000
	loadfpx $f2 0x7fffffff
	clearfcsr
	div.s $f3, $f1, $f2
	checkfpx 31 0x00000001 $f3 0x00000000

	loadfpx $f1 0x80800000
	loadfpx $f2 0xffffffff
	clearfcsr
	div.s $f3, $f1, $f2
	checkfpx 32 0x00000001 $f3 0x00000000

	loadfpx $f1 0x00800000
	loadfpx $f2 0xffffffff
	clearfcsr
	div.s $f3, $f1, $f2
	checkfpx 33 0x00000001 $f3 0x80000000

	loadfpx $f1 0x80800000
	loadfpx $f2 0x7fffffff
	clearfcsr
	div.s $f3, $f1, $f2
	checkfpx 34 0x00000001 $f3 0x80000000


	loadfpx $f1 0x7fffffff
	loadfpx $f2 0x00800001
	clearfcsr
	div.s $f3, $f1, $f2
	checkfpx 41 0x00000001 $f3 0x7fffffff

	loadfpx $f1 0xffffffff
	loadfpx $f2 0x80800001
	clearfcsr
	div.s $f3, $f1, $f2
	checkfpx 42 0x00000001 $f3 0x7fffffff

	loadfpx $f1 0xffffffff
	loadfpx $f2 0x00800001
	clearfcsr
	div.s $f3, $f1, $f2
	checkfpx 43 0x00000001 $f3 0xffffffff

	loadfpx $f1 0x7fffffff
	loadfpx $f2 0x80800001
	clearfcsr
	div.s $f3, $f1, $f2
	checkfpx 44 0x00000001 $f3 0xffffffff

	exit0
