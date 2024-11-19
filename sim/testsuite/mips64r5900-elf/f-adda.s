.include "t-macros.i"

	start

test_adda1:	
	clearfcsr
	loadfp $f1 4.0
	loadfp $f2 0.1
	adda.s $f1, $f2
	checkacc 0 4.1

test_add2:
	clearfcsr
	loadfp $f1 , -4.0
	loadfp $f2 8.0
	adda.s $f2, $f1
	checkacc 0 4.0

test_add3:
	clearfcsr
	loadfpmax $f1
	loadfpmax $f2
	adda.s $f2, $f1
	checkaccmax FCSR_O

test_add4:
	clearfcsr
	loadfpmax $f1
	neg.s $f1, $f1
	loadfpmax $f2
	neg.s $f2, $f2
	adda.s $f1, $f2
	checkfcsr FCSR_O
	clearfcsr
	msub.s $f3, $f0, $f0
	neg.s $f3, $f3
	clearfcsr
	checkfpmax 0 $f3

	clearfcsr
	loadfpx $f1 0x7fffffff 
	loadfpx $f2 0x7fffffff
	adda.s $f1, $f2
	checkaccx 10 0x00008011 0x7fffffff

	clearfcsr
	loadfpx $f1 0x7f7fffff 
	loadfpx $f2 0x7f8fffff 
	adda.s $f1, $f2
	checkaccx 11 0x00008011 0x7fffffff

	clearfcsr
	loadfpx $f1 0x80000000 
	loadfpx $f2 0x00000000 
	adda.s $f1, $f2
	checkaccx 12 0x00000001 0x00000000

	clearfcsr
	loadfpx $f1 0x00000000 
	loadfpx $f2 0x00000000 
	adda.s $f1, $f2
	checkaccx 13 0x00000001 0x00000000


	clearfcsr
	loadfpx $f1 0xffffffff 
	loadfpx $f2 0xffffffff 
	adda.s $f1, $f2
	checkaccx 14 0x00008011 0xffffffff

	clearfcsr
	loadfpx $f1 0xff7fffff 
	loadfpx $f2 0xff8fffff
	adda.s $f1, $f2
	checkaccx 15 0x00008011 0xffffffff

	clearfcsr
	loadfpx $f1 0x00000000 
	loadfpx $f2 0x80000000 
	adda.s $f1, $f2
	checkaccx 16 0x00000001 0x00000000

	clearfcsr
	loadfpx $f1 0x00000000 
	loadfpx $f2 0x00000000 
	adda.s $f1, $f2
	checkaccx 17 0x00000001 0x00000000


	clearfcsr
	loadfpx $f1 0x00800000 
	loadfpx $f2 0x80080001 
	adda.s $f1, $f2
	checkaccx 18 0x00000001 0x00800000

	clearfcsr
	loadfpx $f1 0x80000000 
	loadfpx $f2 0x00000000 
	adda.s $f1, $f2
	checkaccx 19 0x00000001 0x00000000

	clearfcsr
	loadfpx $f1 0x00000000 
	loadfpx $f2 0x00000000 
	adda.s $f1, $f2
	checkaccx 20 0x00000001 0x00000000


	clearfcsr
	loadfpx $f1 0x80800000 
	loadfpx $f2 0x00080001 
	adda.s $f1, $f2
	checkaccx 21 0x00000001 0x80800000

	clearfcsr
	loadfpx $f1 0x80000000 
	loadfpx $f2 0x00000000 
	adda.s $f1, $f2
	checkaccx 22 0x00000001 0x00000000

	clearfcsr
	loadfpx $f1 0x00000000 
	loadfpx $f2 0x00000000 
	adda.s $f1, $f2
	checkaccx 23 0x00000001 0x00000000
	
	exit0
