.include "t-macros.i"

	start

test_madd1:	
	clearfcsr
	loadacc 1.0
	loadfp $f1 2.0
	loadfp $f2 4.0
	madd.s $f3, $f2, $f1
	checkfp 0 $f3 9.0

test_madd2:	
	clearfcsr
	loadacc 4.0
	loadfp $f1 2.0
	loadfp $f2 2.0
	madd.s $f3, $f2, $f1
	checkfp 0 $f3 8.0

	
        #------------------------------------------------#
        # mul overflow                                   #
        #------------------------------------------------#
 
	loadaccx 0x3f800001
	loadfpx $f1 0x7fffffff
	loadfpx $f2 0x7fffffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 1 0x00008011 $f3 0x7fffffff

	loadaccx 0x3f800001
	loadfpx $f1 0x7f3fffff
	loadfpx $f2 0x7f3fffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 2 0x00008011 $f3 0x7fffffff

	loadaccx 0x3f800001
	loadfpx $f1 0xffffffff
	loadfpx $f2 0xffffffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 3 0x00008011 $f3 0x7fffffff

	loadaccx 0x00000000
	loadfpx $f1 0x7fffffff
	loadfpx $f2 0x7fffffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 4 0x00008011 $f3 0x7fffffff

	loadaccx 0x00000000
	loadfpx $f1 0x7f3fffff
	loadfpx $f2 0x7f3fffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 5 0x00008011 $f3 0x7fffffff

	loadaccx 0x00000000
	loadfpx $f1 0xffffffff
	loadfpx $f2 0xffffffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 6 0x00008011 $f3 0x7fffffff

	loadaccx 0xbf800001
	loadfpx $f1 0x7fffffff
	loadfpx $f2 0x7fffffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 7 0x00008011 $f3 0x7fffffff

	loadaccx 0xbf800001
	loadfpx $f1 0x7f3fffff
	loadfpx $f2 0x7f3fffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 8 0x00008011 $f3 0x7fffffff

	loadaccx 0xbf800001
	loadfpx $f1 0xffffffff
	loadfpx $f2 0xffffffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 9 0x00008011 $f3 0x7fffffff


	loadaccx 0x3f800001
	loadfpx $f1 0x7fffffff
	loadfpx $f2 0xffffffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 10 0x00008011 $f3 0xffffffff

	loadaccx 0x3f800001
	loadfpx $f1 0x7f3fffff
	loadfpx $f2 0xff3fffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 11 0x00008011 $f3 0xffffffff

	loadaccx 0x00000000
	loadfpx $f1 0xffffffff
	loadfpx $f2 0x7fffffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 12 0x00008011 $f3 0xffffffff

	loadaccx 0x00000000
	loadfpx $f1 0x7f3fffff
	loadfpx $f2 0xff3fffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 13 0x00008011 $f3 0xffffffff

	loadaccx 0xbf800001
	loadfpx $f1 0x7fffffff
	loadfpx $f2 0xffffffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 14 0x00008011 $f3 0xffffffff

	loadaccx 0xbf800001
	loadfpx $f1 0x7f3fffff
	loadfpx $f2 0xff3fffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 15 0x00008011 $f3 0xffffffff


        #------------------------------------------------#
        # mul underflow                                  #
        #------------------------------------------------#

	loadaccx 0x3f800001
	loadfpx $f1 0x00800000
	loadfpx $f2 0x00800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 21 0x00000009 $f3 0x3f800001

	loadaccx 0x3f800001
	loadfpx $f1 0x008fffff
	loadfpx $f2 0x008fffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 22 0x00000009 $f3 0x3f800001

	loadaccx 0x3f800001
	loadfpx $f1 0x8080000f
	loadfpx $f2 0x8080000f
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 23 0x00000009 $f3 0x3f800001

	loadaccx 0x00000000
	loadfpx $f1 0x00800000
	loadfpx $f2 0x00800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 24 0x00000009 $f3 0x00000000

	loadaccx 0x00000000
	loadfpx $f1 0x808fffff
	loadfpx $f2 0x808fffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 25 0x00000009 $f3 0x00000000

	loadaccx 0x00000000
	loadfpx $f1 0x8080000f
	loadfpx $f2 0x8080000f
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 26 0x00000009 $f3 0x00000000

	loadaccx 0xbf800001
	loadfpx $f1 0x00800000
	loadfpx $f2 0x00800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 27 0x00000009 $f3 0xbf800001

	loadaccx 0xbf800001
	loadfpx $f1 0x008fffff
	loadfpx $f2 0x008fffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 28 0x00000009 $f3 0xbf800001

	loadaccx 0xbf800001
	loadfpx $f1 0x8080000f
	loadfpx $f2 0x8080000f
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 29 0x00000009 $f3 0xbf800001


	loadaccx 0x3f800001
	loadfpx $f1 0x808fffff
	loadfpx $f2 0x008fffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 30 0x00000009 $f3 0x3f800001

	loadaccx 0x3f800001
	loadfpx $f1 0x0080000f
	loadfpx $f2 0x8080000f
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 31 0x00000009 $f3 0x3f800001

	loadaccx 0x00000000
	loadfpx $f1 0x008fffff
	loadfpx $f2 0x808fffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 32 0x00000009 $f3 0x00000000

	loadaccx 0x00000000
	loadfpx $f1 0x8080000f
	loadfpx $f2 0x0080000f
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 33 0x00000009 $f3 0x00000000

	loadaccx 0xbf800001
	loadfpx $f1 0x808fffff
	loadfpx $f2 0x008fffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 34 0x00000009 $f3 0xbf800001

	loadaccx 0xbf800001
	loadfpx $f1 0x0080000f
	loadfpx $f2 0x8080000f
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 35 0x00000009 $f3 0xbf800001


        #------------------------------------------------#
        # add overflow                                   #
        #------------------------------------------------#

	loadaccx 0x7f7fffff
	loadfpx $f1 0x7f8fffff
	loadfpx $f2 0x3f800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 41 0x00008011 $f3 0x7fffffff

	loadaccx 0x7f7fffff
	loadfpx $f1 0xff8fffff
	loadfpx $f2 0xbf800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 42 0x00008011 $f3 0x7fffffff

	loadaccx 0xff7fffff
	loadfpx $f1 0x7f8fffff
	loadfpx $f2 0xbf800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 43 0x00008011 $f3 0xffffffff

	loadaccx 0xff7fffff
	loadfpx $f1 0xff8fffff
	loadfpx $f2 0x3f800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 44 0x00008011 $f3 0xffffffff



        #------------------------------------------------#
        # add underflow                                  #
        #------------------------------------------------#

	loadaccx 0x80800000
	loadfpx $f1 0x00800001
	loadfpx $f2 0x3f800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 51 0x00004009 $f3 0x00000000

	loadaccx 0x80800000
	loadfpx $f1 0x00800001
	loadfpx $f2 0x3f800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 52 0x00004009 $f3 0x00000000

	loadaccx 0x00800000
	loadfpx $f1 0x80800001
	loadfpx $f2 0x3f800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 53 0x00004009 $f3 0x80000000

	loadaccx 0x00800000
	loadfpx $f1 0x00800001
	loadfpx $f2 0xbf800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 54 0x00004009 $f3 0x80000000

	

        #------------------------------------------------#
        # momal                                          #
        #------------------------------------------------#
	
	loadaccx 0x7fffffff
	loadfpx $f1 0x7f8fffff
	loadfpx $f2 0x3f800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 1 0x00008011 $f3 0x7fffffff

	loadaccx 0x7fffffff
	loadfpx $f1 0x7f8fffff
	loadfpx $f2 0xbf800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 2 0x00008011 $f3 0x7fffffff

	loadaccx 0x7fffffff
	loadfpx $f1 0xff8fffff
	loadfpx $f2 0x3f800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 3 0x00008011 $f3 0x7fffffff

	loadaccx 0x7fffffff
	loadfpx $f1 0xff8fffff
	loadfpx $f2 0xbf800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 4 0x00008011 $f3 0x7fffffff

	loadaccx 0x7fffffff
	loadfpx $f1 0x00080001
	loadfpx $f2 0x3f800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 5 0x00008011 $f3 0x7fffffff

	loadaccx 0x7fffffff
	loadfpx $f1 0x80080001
	loadfpx $f2 0x3f800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 6 0x00008011 $f3 0x7fffffff

	loadaccx 0x7fffffff
	loadfpx $f1 0x00080001
	loadfpx $f2 0xbf800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 7 0x00008011 $f3 0x7fffffff

	loadaccx 0x7fffffff
	loadfpx $f1 0x80080001
	loadfpx $f2 0xbf800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 8 0x00008011 $f3 0x7fffffff


	loadaccx 0xffffffff
	loadfpx $f1 0x7f8fffff
	loadfpx $f2 0x3f800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 11 0x00008011 $f3 0xffffffff

	loadaccx 0xffffffff
	loadfpx $f1 0x7f8fffff
	loadfpx $f2 0xbf800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 12 0x00008011 $f3 0xffffffff

	loadaccx 0xffffffff
	loadfpx $f1 0xff8fffff
	loadfpx $f2 0x3f800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 13 0x00008011 $f3 0xffffffff

	loadaccx 0xffffffff
	loadfpx $f1 0xff8fffff
	loadfpx $f2 0xbf800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 14 0x00008011 $f3 0xffffffff

	loadaccx 0xffffffff
	loadfpx $f1 0x00080001
	loadfpx $f2 0x3f800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 15 0x00008011 $f3 0xffffffff

	loadaccx 0xffffffff
	loadfpx $f1 0x80080001
	loadfpx $f2 0x3f800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 16 0x00008011 $f3 0xffffffff

	loadaccx 0xffffffff
	loadfpx $f1 0x00080001
	loadfpx $f2 0xbf800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 17 0x00008011 $f3 0xffffffff

	loadaccx 0xffffffff
	loadfpx $f1 0x80080001
	loadfpx $f2 0xbf800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 18 0x00008011 $f3 0xffffffff

	
        #------------------------------------------------#
        # mul underflow                                  #
        #------------------------------------------------#
	
	loadaccx 0x7fffffff
	loadfpx $f1 0x00800000
	loadfpx $f2 0x00800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 21 0x00008019 $f3 0x7fffffff

	loadaccx 0x7fffffff
	loadfpx $f1 0x008fffff
	loadfpx $f2 0x008fffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 22 0x00008019 $f3 0x7fffffff

	loadaccx 0x7fffffff
	loadfpx $f1 0x8080000f
	loadfpx $f2 0x8080000f
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 23 0x00008019 $f3 0x7fffffff

	loadaccx 0xffffffff
	loadfpx $f1 0x00800000
	loadfpx $f2 0x00800000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 24 0x00008019 $f3 0xffffffff

	loadaccx 0xffffffff
	loadfpx $f1 0x008fffff
	loadfpx $f2 0x008fffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 25 0x00008019 $f3 0xffffffff

	loadaccx 0xffffffff
	loadfpx $f1 0x8080000f
	loadfpx $f2 0x8080000f
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 26 0x00008019 $f3 0xffffffff


        #------------------------------------------------#
        # mul overflow                                   #
        #------------------------------------------------#

	loadaccx 0x7fffffff
	loadfpx $f1 0x7fffffff
	loadfpx $f2 0x7fffffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 31 0x00008011 $f3 0x7fffffff

	loadaccx 0x7fffffff
	loadfpx $f1 0x7f3fffff
	loadfpx $f2 0x7f3fffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 32 0x00008011 $f3 0x7fffffff

	loadaccx 0x7fffffff
	loadfpx $f1 0xffffffff
	loadfpx $f2 0xffffffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 33 0x00008011 $f3 0x7fffffff


	loadaccx 0x7fffffff
	loadfpx $f1 0xffffffff
	loadfpx $f2 0x7fffffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 41 0x00008011 $f3 0xffffffff

	loadaccx 0x7fffffff
	loadfpx $f1 0x7fffffff
	loadfpx $f2 0xffffffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 42 0x00008011 $f3 0xffffffff

	loadaccx 0x7fffffff
	loadfpx $f1 0x7f3fffff
	loadfpx $f2 0xff3fffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 43 0x00008011 $f3 0xffffffff


	loadaccx 0xffffffff
	loadfpx $f1 0x7fffffff
	loadfpx $f2 0x7fffffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 51 0x00008011 $f3 0x7fffffff

	loadaccx 0xffffffff
	loadfpx $f1 0x7f3fffff
	loadfpx $f2 0x7f3fffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 52 0x00008011 $f3 0x7fffffff

	loadaccx 0xffffffff
	loadfpx $f1 0xffffffff
	loadfpx $f2 0xffffffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 53 0x00008011 $f3 0x7fffffff


	loadaccx 0xffffffff
	loadfpx $f1 0xffffffff
	loadfpx $f2 0x7fffffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 61 0x00008011 $f3 0xffffffff

	loadaccx 0xffffffff
	loadfpx $f1 0x7fffffff
	loadfpx $f2 0xffffffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 62 0x00008011 $f3 0xffffffff

	loadaccx 0xffffffff
	loadfpx $f1 0x7f3fffff
	loadfpx $f2 0xff3fffff
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 63 0x00008011 $f3 0xffffffff


        #------------------------------------------------#
        # zero                                           #
        #------------------------------------------------#
 
	loadaccx 0x00000000
	loadfpx $f1 0x00000000
	loadfpx $f2 0x00000000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 1 0x00000001 $f3 0x00000000

	loadaccx 0x00000000
	loadfpx $f1 0x00000000
	loadfpx $f2 0x80000000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 2 0x00000001 $f3 0x00000000

	loadaccx 0x00000000
	loadfpx $f1 0x80000000
	loadfpx $f2 0x00000000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 3 0x00000001 $f3 0x00000000

	loadaccx 0x00000000
	loadfpx $f1 0x80000000
	loadfpx $f2 0x80000000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 4 0x00000001 $f3 0x00000000

	loadaccx 0x80000000
	loadfpx $f1 0x00000000
	loadfpx $f2 0x00000000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 5 0x00000001 $f3 0x00000000

	loadaccx 0x80000000
	loadfpx $f1 0x00000000
	loadfpx $f2 0x80000000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 6 0x00000001 $f3 0x80000000

	loadaccx 0x80000000
	loadfpx $f1 0x80000000
	loadfpx $f2 0x00000000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 7 0x00000001 $f3 0x80000000

	loadaccx 0x80000000
	loadfpx $f1 0x80000000
	loadfpx $f2 0x80000000
	clearfcsr
	madd.s $f3, $f1, $f2
	checkfpx 8 0x00000001 $f3 0x00000000

	exit0
