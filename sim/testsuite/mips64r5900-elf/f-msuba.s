.include "t-macros.i"

	start

test_msuba1:	
	clearfcsr
	loadacc  2.0
	loadfp $f1 1.0
	loadfp $f2 2.0
	msuba.s $f2, $f1
	checkacc 0 0.0

        #------------------------------------------------#
        # mul overflow                                   #
        #------------------------------------------------#

	loadaccx 0x3f800001
	loadfpx $f1 0x7fffffff
	loadfpx $f2 0x7fffffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  101 0x00008011 0xffffffff

	loadaccx 0x3f800001
	loadfpx $f1 0x7f3fffff
	loadfpx $f2 0x7f3fffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  102 0x00008011 0xffffffff
	
	loadaccx 0x3f800001
	loadfpx $f1 0xffffffff
	loadfpx $f2 0xffffffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  103 0x00008011 0xffffffff
	
	loadaccx 0x00000000
	loadfpx $f1 0x7fffffff
	loadfpx $f2 0x7fffffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  104 0x00008011 0xffffffff
	
	loadaccx 0x00000000
	loadfpx $f1 0x7f3fffff
	loadfpx $f2 0x7f3fffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  105 0x00008011 0xffffffff
	
	loadaccx 0x00000000
	loadfpx $f1 0xffffffff
	loadfpx $f2 0xffffffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  106 0x00008011 0xffffffff
	
	loadaccx 0xbf800001
	loadfpx $f1 0x7fffffff
	loadfpx $f2 0x7fffffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  107 0x00008011 0xffffffff
	
	loadaccx 0xbf800001
	loadfpx $f1 0x7f3fffff
	loadfpx $f2 0x7f3fffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  108 0x00008011 0xffffffff
	
	loadaccx 0xbf800001
	loadfpx $f1 0xffffffff
	loadfpx $f2 0xffffffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  109 0x00008011 0xffffffff
	

	loadaccx 0x3f800001
	loadfpx $f1 0x7fffffff
	loadfpx $f2 0xffffffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  110 0x00008011 0x7fffffff
	
	loadaccx 0x3f800001
	loadfpx $f1 0x7f3fffff
	loadfpx $f2 0xff3fffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  111 0x00008011 0x7fffffff
	
	loadaccx 0x00000000
	loadfpx $f1 0xffffffff
	loadfpx $f2 0x7fffffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  112 0x00008011 0x7fffffff
	
	loadaccx 0x00000000
	loadfpx $f1 0x7f3fffff
	loadfpx $f2 0xff3fffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  113 0x00008011 0x7fffffff
	
	loadaccx 0xbf800001
	loadfpx $f1 0x7fffffff
	loadfpx $f2 0xffffffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  114 0x00008011 0x7fffffff
	
	loadaccx 0xbf800001
	loadfpx $f1 0x7f3fffff
	loadfpx $f2 0xff3fffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  115 0x00008011 0x7fffffff
	

        #------------------------------------------------#
        # mul underflow                                  #
        #------------------------------------------------#

	loadaccx 0x3f800001
	loadfpx $f1 0x00800000
	loadfpx $f2 0x00800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  121 0x00000009 0x3f800001
	
	loadaccx 0x3f800001
	loadfpx $f1 0x008fffff
	loadfpx $f2 0x008fffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  122 0x00000009 0x3f800001
	
	loadaccx 0x3f800001
	loadfpx $f1 0x8080000f
	loadfpx $f2 0x8080000f
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  123 0x00000009 0x3f800001
	
	loadaccx 0x00000000
	loadfpx $f1 0x00800000
	loadfpx $f2 0x00800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  124 0x00000009 0x00000000
	
	loadaccx 0x00000000
	loadfpx $f1 0x808fffff
	loadfpx $f2 0x808fffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  125 0x00000009 0x00000000
	
	loadaccx 0x00000000
	loadfpx $f1 0x8080000f
	loadfpx $f2 0x8080000f
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  126 0x00000009 0x00000000
	
	loadaccx 0xbf800001
	loadfpx $f1 0x00800000
	loadfpx $f2 0x00800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  127 0x00000009 0xbf800001
	
	loadaccx 0xbf800001
	loadfpx $f1 0x008fffff
	loadfpx $f2 0x008fffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  128 0x00000009 0xbf800001
	
	loadaccx 0xbf800001
	loadfpx $f1 0x8080000f
	loadfpx $f2 0x8080000f
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  129 0x00000009 0xbf800001
	

	loadaccx 0x3f800001
	loadfpx $f1 0x808fffff
	loadfpx $f2 0x008fffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  130 0x00000009 0x3f800001
	
	loadaccx 0x3f800001
	loadfpx $f1 0x0080000f
	loadfpx $f2 0x8080000f
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  131 0x00000009 0x3f800001
	
	loadaccx 0x00000000
	loadfpx $f1 0x008fffff
	loadfpx $f2 0x808fffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  132 0x00000009 0x00000000
	
	loadaccx 0x00000000
	loadfpx $f1 0x8080000f
	loadfpx $f2 0x0080000f
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  133 0x00000009 0x00000000
	
	loadaccx 0xbf800001
	loadfpx $f1 0x808fffff
	loadfpx $f2 0x008fffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  134 0x00000009 0xbf800001
	
	loadaccx 0xbf800001
	loadfpx $f1 0x0080000f
	loadfpx $f2 0x8080000f
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  135 0x00000009 0xbf800001
	

        #------------------------------------------------#
        # add overflow                                   #
        #------------------------------------------------#
	loadaccx 0xff7fffff
	loadfpx $f1 0x7f8fffff
	loadfpx $f2 0x3f800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  141 0x00008011 0xffffffff
	
	loadaccx 0x7f7fffff
	loadfpx $f1 0xff8fffff
	loadfpx $f2 0x3f800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  142 0x00008011 0x7fffffff
	
	loadaccx 0xff7fffff
	loadfpx $f1 0x7f8fffff
	loadfpx $f2 0x3f800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  143 0x00008011 0xffffffff
	
	loadaccx 0x7f7fffff
	loadfpx $f1 0x7f8fffff
	loadfpx $f2 0xbf800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  144 0x00008011 0x7fffffff
	


        #------------------------------------------------#
        # add underflow                                  #
        #------------------------------------------------#

	loadaccx 0x80800000
	loadfpx $f1 0x00800001
	loadfpx $f2 0xbf800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  151 0x00004009 0x00000000
	
	loadaccx 0x80800000
	loadfpx $f1 0x00800001
	loadfpx $f2 0xbf800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  152 0x00004009 0x00000000
	
	loadaccx 0x00800000
	loadfpx $f1 0x80800001
	loadfpx $f2 0xbf800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  153 0x00004009 0x80000000
	
	loadaccx 0x00800000
	loadfpx $f1 0x00800001
	loadfpx $f2 0x3f800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  154 0x00004009 0x80000000
	

        #------------------------------------------------#
        # momal                                          #
        #------------------------------------------------#
	
	loadaccx 0x7fffffff
	loadfpx $f1 0x7f8fffff
	loadfpx $f2 0x3f800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  101 0x00008011 0x7fffffff
	
	loadaccx 0x7fffffff
	loadfpx $f1 0x7f8fffff
	loadfpx $f2 0xbf800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  102 0x00008011 0x7fffffff
	
	loadaccx 0x7fffffff
	loadfpx $f1 0xff8fffff
	loadfpx $f2 0x3f800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  103 0x00008011 0x7fffffff
	
	loadaccx 0x7fffffff
	loadfpx $f1 0xff8fffff
	loadfpx $f2 0xbf800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  104 0x00008011 0x7fffffff
	
	loadaccx 0x7fffffff
	loadfpx $f1 0x00080001
	loadfpx $f2 0x3f800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  105 0x00008011 0x7fffffff
	
	loadaccx 0x7fffffff
	loadfpx $f1 0x80080001
	loadfpx $f2 0x3f800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  106 0x00008011 0x7fffffff
	
	loadaccx 0x7fffffff
	loadfpx $f1 0x00080001
	loadfpx $f2 0xbf800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  107 0x00008011 0x7fffffff
	
	loadaccx 0x7fffffff
	loadfpx $f1 0x80080001
	loadfpx $f2 0xbf800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  108 0x00008011 0x7fffffff
	

	loadaccx 0xffffffff
	loadfpx $f1 0x7f8fffff
	loadfpx $f2 0x3f800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  111 0x00008011 0xffffffff
	
	loadaccx 0xffffffff
	loadfpx $f1 0x7f8fffff
	loadfpx $f2 0xbf800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  112 0x00008011 0xffffffff
	
	loadaccx 0xffffffff
	loadfpx $f1 0xff8fffff
	loadfpx $f2 0x3f800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  113 0x00008011 0xffffffff
	
	loadaccx 0xffffffff
	loadfpx $f1 0xff8fffff
	loadfpx $f2 0xbf800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  114 0x00008011 0xffffffff
	
	loadaccx 0xffffffff
	loadfpx $f1 0x00080001
	loadfpx $f2 0x3f800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  115 0x00008011 0xffffffff
	
	loadaccx 0xffffffff
	loadfpx $f1 0x80080001
	loadfpx $f2 0x3f800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  116 0x00008011 0xffffffff
	
	loadaccx 0xffffffff
	loadfpx $f1 0x00080001
	loadfpx $f2 0xbf800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  117 0x00008011 0xffffffff
	
	loadaccx 0xffffffff
	loadfpx $f1 0x80080001
	loadfpx $f2 0xbf800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  118 0x00008011 0xffffffff
	
	
        #------------------------------------------------#
        # mul underflow                                  #
        #------------------------------------------------#
	
	loadaccx 0x7fffffff
	loadfpx $f1 0x00800000
	loadfpx $f2 0x00800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  121 0x00008019 0x7fffffff
	
	loadaccx 0x7fffffff
	loadfpx $f1 0x008fffff
	loadfpx $f2 0x008fffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  122 0x00008019 0x7fffffff
	
	loadaccx 0x7fffffff
	loadfpx $f1 0x8080000f
	loadfpx $f2 0x8080000f
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  123 0x00008019 0x7fffffff
	
	loadaccx 0xffffffff
	loadfpx $f1 0x00800000
	loadfpx $f2 0x00800000
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  124 0x00008019 0xffffffff
	
	loadaccx 0xffffffff
	loadfpx $f1 0x008fffff
	loadfpx $f2 0x008fffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  125 0x00008019 0xffffffff
	
	loadaccx 0xffffffff
	loadfpx $f1 0x8080000f
	loadfpx $f2 0x8080000f
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  126 0x00008019 0xffffffff
	

        #------------------------------------------------#
        # mul overflow                                   #
        #------------------------------------------------#

	loadaccx 0x7fffffff
	loadfpx $f1 0x7fffffff
	loadfpx $f2 0x7fffffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  131 0x00008011 0xffffffff
	
	loadaccx 0x7fffffff
	loadfpx $f1 0x7f3fffff
	loadfpx $f2 0x7f3fffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  132 0x00008011 0xffffffff
	
	loadaccx 0x7fffffff
	loadfpx $f1 0xffffffff
	loadfpx $f2 0xffffffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  133 0x00008011 0xffffffff
	

	loadaccx 0x7fffffff
	loadfpx $f1 0xffffffff
	loadfpx $f2 0x7fffffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  141 0x00008011 0x7fffffff
	
	loadaccx 0x7fffffff
	loadfpx $f1 0x7fffffff
	loadfpx $f2 0xffffffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  142 0x00008011 0x7fffffff
	
	loadaccx 0x7fffffff
	loadfpx $f1 0x7f3fffff
	loadfpx $f2 0xff3fffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  143 0x00008011 0x7fffffff
	

	loadaccx 0xffffffff
	loadfpx $f1 0x7fffffff
	loadfpx $f2 0x7fffffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  151 0x00008011 0xffffffff
	
	loadaccx 0xffffffff
	loadfpx $f1 0x7f3fffff
	loadfpx $f2 0x7f3fffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  152 0x00008011 0xffffffff
	
	loadaccx 0xffffffff
	loadfpx $f1 0xffffffff
	loadfpx $f2 0xffffffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  153 0x00008011 0xffffffff
	

	loadaccx 0xffffffff
	loadfpx $f1 0xffffffff
	loadfpx $f2 0x7fffffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  161 0x00008011 0x7fffffff
	
	loadaccx 0xffffffff
	loadfpx $f1 0x7fffffff
	loadfpx $f2 0xffffffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  162 0x00008011 0x7fffffff
	
	loadaccx 0xffffffff
	loadfpx $f1 0x7f3fffff
	loadfpx $f2 0xff3fffff
	clearfcsr
	msuba.s $f1, $f2
	checkaccx  163 0x00008011 0x7fffffff
	

	exit0
