.include "t-macros.i"

	start

test_cvtws1:	
	clearfcsr
	loadfp $f1 4.0
	cvt.w.s $f2, $f1
	mfc1 $10, $2
	check10 0 4

test_cvtws2:	
	clearfcsr
	loadfp $f1 4.0
	neg.s $f1, $f1
	cvt.w.s $f2, $f1
	mfc1 $10, $2
	check10 0 , -4

test_cvtws3:
	clearfcsr
	loadfpx $f1 0xcf0c2c5c
	cvt.w.s $f2, $f1
	mfc1 $10, $2
	check10 0x0000000000000000, 0xffffffff80000000

test_cvtws0x4f800000:
	clearfcsr
	loadfpx $f1 0x4f800000
	cvt.w.s $f2, $f1
	mfc1 $10, $2
	check10 0x0000000000000000, 0x000000007fffffff

test_cvtws0x3f800000:
	clearfcsr
	loadfpx $f1 0x3f800000
	cvt.w.s $f2, $f1
	mfc1 $10, $2
	check10 0x0000000000000000, 0x0000000000000001

test_cvtws0xbf800001:
	clearfcsr
	loadfpx $f1 0xbf800001
	cvt.w.s $f2, $f1
	mfc1 $10, $2
	check10 0x0000000000000000, 0xffffffffffffffff

test_cvtws0x4f8c2c5c:
	clearfcsr
	loadfpx $f1 0x4f8c2c5c
	cvt.w.s $f2, $f1
	mfc1 $10, $2
	check10 0x0000000000000000, 0x000000007fffffff

test_cvtws0x4f700000:
	clearfcsr
	loadfpx $f1 0x4f700000
	cvt.w.s $f2, $f1
	mfc1 $10, $2
	check10 0x0000000000000000, 0x000000007fffffff

test_cvtws0x4f7c2c5c:
	clearfcsr
	loadfpx $f1 0x4f7c2c5c
	cvt.w.s $f2, $f1
	mfc1 $10, $2
	check10 0x0000000000000000, 0x000000007fffffff

test_cvtws0x4f000001:
	clearfcsr
	loadfpx $f1 0x4f000001
	cvt.w.s $f2, $f1
	mfc1 $10, $2
	check10 0x0000000000000000, 0x000000007fffffff

test_cvtws0xcf800000:
	clearfcsr
	loadfpx $f1 0xcf800000
	cvt.w.s $f2, $f1
	mfc1 $10, $2
	check10 0x0000000000000000, 0xffffffff80000000

test_cvtws0xcf8c2c5c:
	clearfcsr
	loadfpx $f1 0xcf8c2c5c
	cvt.w.s $f2, $f1
	mfc1 $10, $2
	check10 0x0000000000000000, 0xffffffff80000000

test_cvtws0xcf000000:
	clearfcsr
	loadfpx $f1 0xcf000000
	cvt.w.s $f2, $f1
	mfc1 $10, $2
	check10 0x0000000000000000, 0xffffffff80000000

test_cvtws0xcf700000:
	clearfcsr
	loadfpx $f1 0xcf700000
	cvt.w.s $f2, $f1
	mfc1 $10, $2
	check10 0x0000000000000000, 0xffffffff80000000

test_cvtws0xcf7c2c5c:
	clearfcsr
	loadfpx $f1 0xcf7c2c5c
	cvt.w.s $f2, $f1
	mfc1 $10, $2
	check10 0x0000000000000000, 0xffffffff80000000

test_cvtws0xcf000001:
	clearfcsr
	loadfpx $f1 0xcf000001
	cvt.w.s $f2, $f1
	mfc1 $10, $2
	check10 0x0000000000000000, 0xffffffff80000000

test_cvtsw1:
	clearfcsr
	li	$4, 4
	mtc1	$4, $4
	cvt.s.w $f3, $f4
	checkfp 0 $f3 4.0

test_cvtsw2:
	clearfcsr
	li	$4, -4
	mtc1	$4, $4
	cvt.s.w $f3, $f4
	checkfp 0 $f3 , -4.0

	exit0
