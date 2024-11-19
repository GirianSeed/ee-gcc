	.text

	.global stuff
	.ent stuff
stuff:
	.set noreorder
	.set nomacro
	.set noat
	
# enable COP2
cop2:		
	mfc0    $1,$12
	dli	$2,0x40000000
	or	$1,$2,$2
	mtc0	$1,$12

# put some END/NOPs into VU0 uMEM
mpg:		
	dli	$1,0x400002ff
	dli	$4,0x000002ff
	dli	$2,0x8000033c
	dli	$3,0xb1000000
	sw	$2,0($3)
	sw	$1,4($3)
	sw	$2,8($3)
	sw	$4,12($3)
	sw	$2,16($3)
	sw	$4,20($3)
	sw	$2,24($3)
	sw	$4,28($3)
	sw	$2,32($3)
	sw	$1,36($3)
			
# start whacking away
	lqc2    vf01,128($6)
	qmfc2   $5,vf2
	qmfc2.NI  $6,vf3
	qmfc2.I   $7,vf4
	qmtc2   $5,vf2
	qmtc2.NI  $6,vf3
	qmtc2.I   $7,vf4
	sqc2    vf01,128($6)
	vabs.w  vf1,vf2
	vabs.x  vf1,vf2
	vabs.y  vf1,vf2
	vabs.z  vf1,vf2
	vabs.wx  vf1,vf2
	vabs.wy  vf1,vf2
	vabs.wz  vf1,vf2
	vabs.xy  vf1,vf2
	vabs.xz  vf1,vf2
	vabs.yz  vf1,vf2
	vabs.wxy  vf1,vf2
	vabs.wxz  vf1,vf2
	vabs.wyz  vf1,vf2
	vabs.xyz  vf1,vf2
	vabs.wxyz  vf1,vf2
	vadd.w  vf1,vf2,vf3
	vadd.x  vf1,vf2,vf3
	vadd.y  vf1,vf2,vf3
	vadd.z  vf1,vf2,vf3
	vadd.wx  vf1,vf2,vf3
	vadd.wy  vf1,vf2,vf3
	vadd.wz  vf1,vf2,vf3
	vadd.xy  vf1,vf2,vf3
	vadd.xz  vf1,vf2,vf3
	vadd.yz  vf1,vf2,vf3
	vadd.wxy  vf1,vf2,vf3
	vadd.wxz  vf1,vf2,vf3
	vadd.wyz  vf1,vf2,vf3
	vadd.xyz  vf1,vf2,vf3
	vadd.wxyz  vf1,vf2,vf3
	vaddi.w  vf1,vf2,I
	vaddi.x  vf1,vf2,I
	vaddi.y  vf1,vf2,I
	vaddi.z  vf1,vf2,I
	vaddi.wx  vf1,vf2,I
	vaddi.wy  vf1,vf2,I
	vaddi.wz  vf1,vf2,I
	vaddi.xy  vf1,vf2,I
	vaddi.xz  vf1,vf2,I
	vaddi.yz  vf1,vf2,I
	vaddi.wxy  vf1,vf2,I
	vaddi.wxz  vf1,vf2,I
	vaddi.wyz  vf1,vf2,I
	vaddi.xyz  vf1,vf2,I
	vaddi.wxyz  vf1,vf2,I
	vaddq.w  vf1,vf2,Q
	vaddq.x  vf1,vf2,Q
	vaddq.y  vf1,vf2,Q
	vaddq.z  vf1,vf2,Q
	vaddq.wx  vf1,vf2,Q
	vaddq.wy  vf1,vf2,Q
	vaddq.wz  vf1,vf2,Q
	vaddq.xy  vf1,vf2,Q
	vaddq.xz  vf1,vf2,Q
	vaddq.yz  vf1,vf2,Q
	vaddq.wxy  vf1,vf2,Q
	vaddq.wxz  vf1,vf2,Q
	vaddq.wyz  vf1,vf2,Q
	vaddq.xyz  vf1,vf2,Q
	vaddq.wxyz  vf1,vf2,Q
	vaddw.w  vf1,vf2,vf3
	vaddw.x  vf1,vf2,vf3
	vaddw.y  vf1,vf2,vf3
	vaddw.z  vf1,vf2,vf3
	vaddw.wx  vf1,vf2,vf3
	vaddw.wy  vf1,vf2,vf3
	vaddw.wz  vf1,vf2,vf3
	vaddw.xy  vf1,vf2,vf3
	vaddw.xz  vf1,vf2,vf3
	vaddw.yz  vf1,vf2,vf3
	vaddw.wxy  vf1,vf2,vf3
	vaddw.wxz  vf1,vf2,vf3
	vaddw.wyz  vf1,vf2,vf3
	vaddw.xyz  vf1,vf2,vf3
	vaddw.wxyz  vf1,vf2,vf3
	vaddx.w  vf1,vf2,vf3
	vaddx.x  vf1,vf2,vf3
	vaddx.y  vf1,vf2,vf3
	vaddx.z  vf1,vf2,vf3
	vaddx.wx  vf1,vf2,vf3
	vaddx.wy  vf1,vf2,vf3
	vaddx.wz  vf1,vf2,vf3
	vaddx.xy  vf1,vf2,vf3
	vaddx.xz  vf1,vf2,vf3
	vaddx.yz  vf1,vf2,vf3
	vaddx.wxy  vf1,vf2,vf3
	vaddx.wxz  vf1,vf2,vf3
	vaddx.wyz  vf1,vf2,vf3
	vaddx.xyz  vf1,vf2,vf3
	vaddx.wxyz  vf1,vf2,vf3
	vaddy.w  vf1,vf2,vf3
	vaddy.x  vf1,vf2,vf3
	vaddy.y  vf1,vf2,vf3
	vaddy.z  vf1,vf2,vf3
	vaddy.wx  vf1,vf2,vf3
	vaddy.wy  vf1,vf2,vf3
	vaddy.wz  vf1,vf2,vf3
	vaddy.xy  vf1,vf2,vf3
	vaddy.xz  vf1,vf2,vf3
	vaddy.yz  vf1,vf2,vf3
	vaddy.wxy  vf1,vf2,vf3
	vaddy.wxz  vf1,vf2,vf3
	vaddy.wyz  vf1,vf2,vf3
	vaddy.xyz  vf1,vf2,vf3
	vaddy.wxyz  vf1,vf2,vf3
	vaddz.w  vf1,vf2,vf3
	vaddz.x  vf1,vf2,vf3
	vaddz.y  vf1,vf2,vf3
	vaddz.z  vf1,vf2,vf3
	vaddz.wx  vf1,vf2,vf3
	vaddz.wy  vf1,vf2,vf3
	vaddz.wz  vf1,vf2,vf3
	vaddz.xy  vf1,vf2,vf3
	vaddz.xz  vf1,vf2,vf3
	vaddz.yz  vf1,vf2,vf3
	vaddz.wxy  vf1,vf2,vf3
	vaddz.wxz  vf1,vf2,vf3
	vaddz.wyz  vf1,vf2,vf3
	vaddz.xyz  vf1,vf2,vf3
	vaddz.wxyz  vf1,vf2,vf3
	vadda.w  ACC,vf2,vf3
	vadda.x  ACC,vf2,vf3
	vadda.y  ACC,vf2,vf3
	vadda.z  ACC,vf2,vf3
	vadda.wx  ACC,vf2,vf3
	vadda.wy  ACC,vf2,vf3
	vadda.wz  ACC,vf2,vf3
	vadda.xy  ACC,vf2,vf3
	vadda.xz  ACC,vf2,vf3
	vadda.yz  ACC,vf2,vf3
	vadda.wxy  ACC,vf2,vf3
	vadda.wxz  ACC,vf2,vf3
	vadda.wyz  ACC,vf2,vf3
	vadda.xyz  ACC,vf2,vf3
	vadda.wxyz  ACC,vf2,vf3
	vaddai.w  ACC,vf2,I
	vaddai.x  ACC,vf2,I
	vaddai.y  ACC,vf2,I
	vaddai.z  ACC,vf2,I
	vaddai.wx  ACC,vf2,I
	vaddai.wy  ACC,vf2,I
	vaddai.wz  ACC,vf2,I
	vaddai.xy  ACC,vf2,I
	vaddai.xz  ACC,vf2,I
	vaddai.yz  ACC,vf2,I
	vaddai.wxy  ACC,vf2,I
	vaddai.wxz  ACC,vf2,I
	vaddai.wyz  ACC,vf2,I
	vaddai.xyz  ACC,vf2,I
	vaddai.wxyz  ACC,vf2,I
	vaddaq.w  ACC,vf2,Q
	vaddaq.x  ACC,vf2,Q
	vaddaq.y  ACC,vf2,Q
	vaddaq.z  ACC,vf2,Q
	vaddaq.wx  ACC,vf2,Q
	vaddaq.wy  ACC,vf2,Q
	vaddaq.wz  ACC,vf2,Q
	vaddaq.xy  ACC,vf2,Q
	vaddaq.xz  ACC,vf2,Q
	vaddaq.yz  ACC,vf2,Q
	vaddaq.wxy  ACC,vf2,Q
	vaddaq.wxz  ACC,vf2,Q
	vaddaq.wyz  ACC,vf2,Q
	vaddaq.xyz  ACC,vf2,Q
	vaddaq.wxyz  ACC,vf2,Q
	vaddaw.w  ACC,vf2,vf3
	vaddaw.x  ACC,vf2,vf3
	vaddaw.y  ACC,vf2,vf3
	vaddaw.z  ACC,vf2,vf3
	vaddaw.wx  ACC,vf2,vf3
	vaddaw.wy  ACC,vf2,vf3
	vaddaw.wz  ACC,vf2,vf3
	vaddaw.xy  ACC,vf2,vf3
	vaddaw.xz  ACC,vf2,vf3
	vaddaw.yz  ACC,vf2,vf3
	vaddaw.wxy  ACC,vf2,vf3
	vaddaw.wxz  ACC,vf2,vf3
	vaddaw.wyz  ACC,vf2,vf3
	vaddaw.xyz  ACC,vf2,vf3
	vaddaw.wxyz  ACC,vf2,vf3
	vaddax.w  ACC,vf2,vf3
	vaddax.x  ACC,vf2,vf3
	vaddax.y  ACC,vf2,vf3
	vaddax.z  ACC,vf2,vf3
	vaddax.wx  ACC,vf2,vf3
	vaddax.wy  ACC,vf2,vf3
	vaddax.wz  ACC,vf2,vf3
	vaddax.xy  ACC,vf2,vf3
	vaddax.xz  ACC,vf2,vf3
	vaddax.yz  ACC,vf2,vf3
	vaddax.wxy  ACC,vf2,vf3
	vaddax.wxz  ACC,vf2,vf3
	vaddax.wyz  ACC,vf2,vf3
	vaddax.xyz  ACC,vf2,vf3
	vaddax.wxyz  ACC,vf2,vf3
	vadday.w  ACC,vf2,vf3
	vadday.x  ACC,vf2,vf3
	vadday.y  ACC,vf2,vf3
	vadday.z  ACC,vf2,vf3
	vadday.wx  ACC,vf2,vf3
	vadday.wy  ACC,vf2,vf3
	vadday.wz  ACC,vf2,vf3
	vadday.xy  ACC,vf2,vf3
	vadday.xz  ACC,vf2,vf3
	vadday.yz  ACC,vf2,vf3
	vadday.wxy  ACC,vf2,vf3
	vadday.wxz  ACC,vf2,vf3
	vadday.wyz  ACC,vf2,vf3
	vadday.xyz  ACC,vf2,vf3
	vadday.wxyz  ACC,vf2,vf3
	vaddaz.w  ACC,vf2,vf3
	vaddaz.x  ACC,vf2,vf3
	vaddaz.y  ACC,vf2,vf3
	vaddaz.z  ACC,vf2,vf3
	vaddaz.wx  ACC,vf2,vf3
	vaddaz.wy  ACC,vf2,vf3
	vaddaz.wz  ACC,vf2,vf3
	vaddaz.xy  ACC,vf2,vf3
	vaddaz.xz  ACC,vf2,vf3
	vaddaz.yz  ACC,vf2,vf3
	vaddaz.wxy  ACC,vf2,vf3
	vaddaz.wxz  ACC,vf2,vf3
	vaddaz.wyz  ACC,vf2,vf3
	vaddaz.xyz  ACC,vf2,vf3
	vaddaz.wxyz  ACC,vf2,vf3
	vcallms  0
	vcallmsr vi27
	vclipw.xyz vf4,vf3w
	vdiv Q,vf4z,vf8w
	vdiv Q,vf4y,vf8x
	vdiv Q,vf4x,vf8y
	vdiv Q,vf4w,vf8z
# placeholders
	vnop
	vnop
	vnop
	vnop
	vftoi0.w  vf1,vf2
	vftoi0.x  vf1,vf2
	vftoi0.y  vf1,vf2
	vftoi0.z  vf1,vf2
	vftoi0.wx  vf1,vf2
	vftoi0.wy  vf1,vf2
	vftoi0.wz  vf1,vf2
	vftoi0.xy  vf1,vf2
	vftoi0.xz  vf1,vf2
	vftoi0.yz  vf1,vf2
	vftoi0.wxy  vf1,vf2
	vftoi0.wxz  vf1,vf2
	vftoi0.wyz  vf1,vf2
	vftoi0.xyz  vf1,vf2
	vftoi0.wxyz  vf1,vf2
	vftoi4.w  vf1,vf2
	vftoi4.x  vf1,vf2
	vftoi4.y  vf1,vf2
	vftoi4.z  vf1,vf2
	vftoi4.wx  vf1,vf2
	vftoi4.wy  vf1,vf2
	vftoi4.wz  vf1,vf2
	vftoi4.xy  vf1,vf2
	vftoi4.xz  vf1,vf2
	vftoi4.yz  vf1,vf2
	vftoi4.wxy  vf1,vf2
	vftoi4.wxz  vf1,vf2
	vftoi4.wyz  vf1,vf2
	vftoi4.xyz  vf1,vf2
	vftoi4.wxyz  vf1,vf2
	vftoi12.w  vf1,vf2
	vftoi12.x  vf1,vf2
	vftoi12.y  vf1,vf2
	vftoi12.z  vf1,vf2
	vftoi12.wx  vf1,vf2
	vftoi12.wy  vf1,vf2
	vftoi12.wz  vf1,vf2
	vftoi12.xy  vf1,vf2
	vftoi12.xz  vf1,vf2
	vftoi12.yz  vf1,vf2
	vftoi12.wxy  vf1,vf2
	vftoi12.wxz  vf1,vf2
	vftoi12.wyz  vf1,vf2
	vftoi12.xyz  vf1,vf2
	vftoi12.wxyz  vf1,vf2
	vftoi15.w  vf1,vf2
	vftoi15.x  vf1,vf2
	vftoi15.y  vf1,vf2
	vftoi15.z  vf1,vf2
	vftoi15.wx  vf1,vf2
	vftoi15.wy  vf1,vf2
	vftoi15.wz  vf1,vf2
	vftoi15.xy  vf1,vf2
	vftoi15.xz  vf1,vf2
	vftoi15.yz  vf1,vf2
	vftoi15.wxy  vf1,vf2
	vftoi15.wxz  vf1,vf2
	vftoi15.wyz  vf1,vf2
	vftoi15.xyz  vf1,vf2
	vftoi15.wxyz  vf1,vf2
	viadd  vi1,vi2,vi3
	viaddi  vi1,vi2,10
	viand  vi1,vi2,vi3
	vilwr.w	vi1,(vi2)
	vilwr.x	vi1,(vi2)
	vilwr.y	vi1,(vi2)
	vilwr.z	vi1,(vi2)
	vior  vi1,vi2,vi3
	viswr.w	vi1,(vi2)
	viswr.x	vi1,(vi2)
	viswr.y	vi1,(vi2)
	viswr.z	vi1,(vi2)
	visub  vi1,vi2,vi3
	vitof0.w  vf1,vf2
	vitof0.x  vf1,vf2
	vitof0.y  vf1,vf2
	vitof0.z  vf1,vf2
	vitof0.wx  vf1,vf2
	vitof0.wy  vf1,vf2
	vitof0.wz  vf1,vf2
	vitof0.xy  vf1,vf2
	vitof0.xz  vf1,vf2
	vitof0.yz  vf1,vf2
	vitof0.wxy  vf1,vf2
	vitof0.wxz  vf1,vf2
	vitof0.wyz  vf1,vf2
	vitof0.xyz  vf1,vf2
	vitof0.wxyz  vf1,vf2
	vitof4.w  vf1,vf2
	vitof4.x  vf1,vf2
	vitof4.y  vf1,vf2
	vitof4.z  vf1,vf2
	vitof4.wx  vf1,vf2
	vitof4.wy  vf1,vf2
	vitof4.wz  vf1,vf2
	vitof4.xy  vf1,vf2
	vitof4.xz  vf1,vf2
	vitof4.yz  vf1,vf2
	vitof4.wxy  vf1,vf2
	vitof4.wxz  vf1,vf2
	vitof4.wyz  vf1,vf2
	vitof4.xyz  vf1,vf2
	vitof4.wxyz  vf1,vf2
	vitof12.w  vf1,vf2
	vitof12.x  vf1,vf2
	vitof12.y  vf1,vf2
	vitof12.z  vf1,vf2
	vitof12.wx  vf1,vf2
	vitof12.wy  vf1,vf2
	vitof12.wz  vf1,vf2
	vitof12.xy  vf1,vf2
	vitof12.xz  vf1,vf2
	vitof12.yz  vf1,vf2
	vitof12.wxy  vf1,vf2
	vitof12.wxz  vf1,vf2
	vitof12.wyz  vf1,vf2
	vitof12.xyz  vf1,vf2
	vitof12.wxyz  vf1,vf2
	vitof15.w  vf1,vf2
	vitof15.x  vf1,vf2
	vitof15.y  vf1,vf2
	vitof15.z  vf1,vf2
	vitof15.wx  vf1,vf2
	vitof15.wy  vf1,vf2
	vitof15.wz  vf1,vf2
	vitof15.xy  vf1,vf2
	vitof15.xz  vf1,vf2
	vitof15.yz  vf1,vf2
	vitof15.wxy  vf1,vf2
	vitof15.wxz  vf1,vf2
	vitof15.wyz  vf1,vf2
	vitof15.xyz  vf1,vf2
	vitof15.wxyz  vf1,vf2
	vlqi.w  vf4,(vi2++)
	vlqi.x  vf4,(vi2++)
	vlqi.y  vf4,(vi2++)
	vlqi.z  vf4,(vi2++)
	vlqi.wx  vf4,(vi2++)
	vlqi.wy  vf4,(vi2++)
	vlqi.wz  vf4,(vi2++)
	vlqi.xy  vf4,(vi2++)
	vlqi.xz  vf4,(vi2++)
	vlqi.yz  vf4,(vi2++)
	vlqi.wxy  vf4,(vi2++)
	vlqi.wxz  vf4,(vi2++)
	vlqi.wyz  vf4,(vi2++)
	vlqi.xyz  vf4,(vi2++)
	vlqi.wxyz  vf4,(vi2++)
	vlqd.w  vf4,(--vi2)
	vlqd.x  vf4,(--vi2)
	vlqd.y  vf4,(--vi2)
	vlqd.z  vf4,(--vi2)
	vlqd.wx  vf4,(--vi2)
	vlqd.wy  vf4,(--vi2)
	vlqd.wz  vf4,(--vi2)
	vlqd.xy  vf4,(--vi2)
	vlqd.xz  vf4,(--vi2)
	vlqd.yz  vf4,(--vi2)
	vlqd.wxy  vf4,(--vi2)
	vlqd.wxz  vf4,(--vi2)
	vlqd.wyz  vf4,(--vi2)
	vlqd.xyz  vf4,(--vi2)
	vlqd.wxyz  vf4,(--vi2)
	vmadd.w  vf1,vf2,vf3
	vmadd.x  vf1,vf2,vf3
	vmadd.y  vf1,vf2,vf3
	vmadd.z  vf1,vf2,vf3
	vmadd.wx  vf1,vf2,vf3
	vmadd.wy  vf1,vf2,vf3
	vmadd.wz  vf1,vf2,vf3
	vmadd.xy  vf1,vf2,vf3
	vmadd.xz  vf1,vf2,vf3
	vmadd.yz  vf1,vf2,vf3
	vmadd.wxy  vf1,vf2,vf3
	vmadd.wxz  vf1,vf2,vf3
	vmadd.wyz  vf1,vf2,vf3
	vmadd.xyz  vf1,vf2,vf3
	vmadd.wxyz  vf1,vf2,vf3
	vmaddi.w  vf1,vf2,I
	vmaddi.x  vf1,vf2,I
	vmaddi.y  vf1,vf2,I
	vmaddi.z  vf1,vf2,I
	vmaddi.wx  vf1,vf2,I
	vmaddi.wy  vf1,vf2,I
	vmaddi.wz  vf1,vf2,I
	vmaddi.xy  vf1,vf2,I
	vmaddi.xz  vf1,vf2,I
	vmaddi.yz  vf1,vf2,I
	vmaddi.wxy  vf1,vf2,I
	vmaddi.wxz  vf1,vf2,I
	vmaddi.wyz  vf1,vf2,I
	vmaddi.xyz  vf1,vf2,I
	vmaddi.wxyz  vf1,vf2,I
	vmaddq.w  vf1,vf2,Q
	vmaddq.x  vf1,vf2,Q
	vmaddq.y  vf1,vf2,Q
	vmaddq.z  vf1,vf2,Q
	vmaddq.wx  vf1,vf2,Q
	vmaddq.wy  vf1,vf2,Q
	vmaddq.wz  vf1,vf2,Q
	vmaddq.xy  vf1,vf2,Q
	vmaddq.xz  vf1,vf2,Q
	vmaddq.yz  vf1,vf2,Q
	vmaddq.wxy  vf1,vf2,Q
	vmaddq.wxz  vf1,vf2,Q
	vmaddq.wyz  vf1,vf2,Q
	vmaddq.xyz  vf1,vf2,Q
	vmaddq.wxyz  vf1,vf2,Q
	vmaddw.w  vf1,vf2,vf3
	vmaddw.x  vf1,vf2,vf3
	vmaddw.y  vf1,vf2,vf3
	vmaddw.z  vf1,vf2,vf3
	vmaddw.wx  vf1,vf2,vf3
	vmaddw.wy  vf1,vf2,vf3
	vmaddw.wz  vf1,vf2,vf3
	vmaddw.xy  vf1,vf2,vf3
	vmaddw.xz  vf1,vf2,vf3
	vmaddw.yz  vf1,vf2,vf3
	vmaddw.wxy  vf1,vf2,vf3
	vmaddw.wxz  vf1,vf2,vf3
	vmaddw.wyz  vf1,vf2,vf3
	vmaddw.xyz  vf1,vf2,vf3
	vmaddw.wxyz  vf1,vf2,vf3
	vmaddx.w  vf1,vf2,vf3
	vmaddx.x  vf1,vf2,vf3
	vmaddx.y  vf1,vf2,vf3
	vmaddx.z  vf1,vf2,vf3
	vmaddx.wx  vf1,vf2,vf3
	vmaddx.wy  vf1,vf2,vf3
	vmaddx.wz  vf1,vf2,vf3
	vmaddx.xy  vf1,vf2,vf3
	vmaddx.xz  vf1,vf2,vf3
	vmaddx.yz  vf1,vf2,vf3
	vmaddx.wxy  vf1,vf2,vf3
	vmaddx.wxz  vf1,vf2,vf3
	vmaddx.wyz  vf1,vf2,vf3
	vmaddx.xyz  vf1,vf2,vf3
	vmaddx.wxyz  vf1,vf2,vf3
	vmaddy.w  vf1,vf2,vf3
	vmaddy.x  vf1,vf2,vf3
	vmaddy.y  vf1,vf2,vf3
	vmaddy.z  vf1,vf2,vf3
	vmaddy.wx  vf1,vf2,vf3
	vmaddy.wy  vf1,vf2,vf3
	vmaddy.wz  vf1,vf2,vf3
	vmaddy.xy  vf1,vf2,vf3
	vmaddy.xz  vf1,vf2,vf3
	vmaddy.yz  vf1,vf2,vf3
	vmaddy.wxy  vf1,vf2,vf3
	vmaddy.wxz  vf1,vf2,vf3
	vmaddy.wyz  vf1,vf2,vf3
	vmaddy.xyz  vf1,vf2,vf3
	vmaddy.wxyz  vf1,vf2,vf3
	vmaddz.w  vf1,vf2,vf3
	vmaddz.x  vf1,vf2,vf3
	vmaddz.y  vf1,vf2,vf3
	vmaddz.z  vf1,vf2,vf3
	vmaddz.wx  vf1,vf2,vf3
	vmaddz.wy  vf1,vf2,vf3
	vmaddz.wz  vf1,vf2,vf3
	vmaddz.xy  vf1,vf2,vf3
	vmaddz.xz  vf1,vf2,vf3
	vmaddz.yz  vf1,vf2,vf3
	vmaddz.wxy  vf1,vf2,vf3
	vmaddz.wxz  vf1,vf2,vf3
	vmaddz.wyz  vf1,vf2,vf3
	vmaddz.xyz  vf1,vf2,vf3
	vmaddz.wxyz  vf1,vf2,vf3
	vmadda.w  ACC,vf2,vf3
	vmadda.x  ACC,vf2,vf3
	vmadda.y  ACC,vf2,vf3
	vmadda.z  ACC,vf2,vf3
	vmadda.wx  ACC,vf2,vf3
	vmadda.wy  ACC,vf2,vf3
	vmadda.wz  ACC,vf2,vf3
	vmadda.xy  ACC,vf2,vf3
	vmadda.xz  ACC,vf2,vf3
	vmadda.yz  ACC,vf2,vf3
	vmadda.wxy  ACC,vf2,vf3
	vmadda.wxz  ACC,vf2,vf3
	vmadda.wyz  ACC,vf2,vf3
	vmadda.xyz  ACC,vf2,vf3
	vmadda.wxyz  ACC,vf2,vf3
	vmaddai.w  ACC,vf2,I
	vmaddai.x  ACC,vf2,I
	vmaddai.y  ACC,vf2,I
	vmaddai.z  ACC,vf2,I
	vmaddai.wx  ACC,vf2,I
	vmaddai.wy  ACC,vf2,I
	vmaddai.wz  ACC,vf2,I
	vmaddai.xy  ACC,vf2,I
	vmaddai.xz  ACC,vf2,I
	vmaddai.yz  ACC,vf2,I
	vmaddai.wxy  ACC,vf2,I
	vmaddai.wxz  ACC,vf2,I
	vmaddai.wyz  ACC,vf2,I
	vmaddai.xyz  ACC,vf2,I
	vmaddai.wxyz  ACC,vf2,I
	vmaddaq.w  ACC,vf2,Q
	vmaddaq.x  ACC,vf2,Q
	vmaddaq.y  ACC,vf2,Q
	vmaddaq.z  ACC,vf2,Q
	vmaddaq.wx  ACC,vf2,Q
	vmaddaq.wy  ACC,vf2,Q
	vmaddaq.wz  ACC,vf2,Q
	vmaddaq.xy  ACC,vf2,Q
	vmaddaq.xz  ACC,vf2,Q
	vmaddaq.yz  ACC,vf2,Q
	vmaddaq.wxy  ACC,vf2,Q
	vmaddaq.wxz  ACC,vf2,Q
	vmaddaq.wyz  ACC,vf2,Q
	vmaddaq.xyz  ACC,vf2,Q
	vmaddaq.wxyz  ACC,vf2,Q
	vmaddaw.w  ACC,vf2,vf3
	vmaddaw.x  ACC,vf2,vf3
	vmaddaw.y  ACC,vf2,vf3
	vmaddaw.z  ACC,vf2,vf3
	vmaddaw.wx  ACC,vf2,vf3
	vmaddaw.wy  ACC,vf2,vf3
	vmaddaw.wz  ACC,vf2,vf3
	vmaddaw.xy  ACC,vf2,vf3
	vmaddaw.xz  ACC,vf2,vf3
	vmaddaw.yz  ACC,vf2,vf3
	vmaddaw.wxy  ACC,vf2,vf3
	vmaddaw.wxz  ACC,vf2,vf3
	vmaddaw.wyz  ACC,vf2,vf3
	vmaddaw.xyz  ACC,vf2,vf3
	vmaddaw.wxyz  ACC,vf2,vf3
	vmaddax.w  ACC,vf2,vf3
	vmaddax.x  ACC,vf2,vf3
	vmaddax.y  ACC,vf2,vf3
	vmaddax.z  ACC,vf2,vf3
	vmaddax.wx  ACC,vf2,vf3
	vmaddax.wy  ACC,vf2,vf3
	vmaddax.wz  ACC,vf2,vf3
	vmaddax.xy  ACC,vf2,vf3
	vmaddax.xz  ACC,vf2,vf3
	vmaddax.yz  ACC,vf2,vf3
	vmaddax.wxy  ACC,vf2,vf3
	vmaddax.wxz  ACC,vf2,vf3
	vmaddax.wyz  ACC,vf2,vf3
	vmaddax.xyz  ACC,vf2,vf3
	vmaddax.wxyz  ACC,vf2,vf3
	vmadday.w  ACC,vf2,vf3
	vmadday.x  ACC,vf2,vf3
	vmadday.y  ACC,vf2,vf3
	vmadday.z  ACC,vf2,vf3
	vmadday.wx  ACC,vf2,vf3
	vmadday.wy  ACC,vf2,vf3
	vmadday.wz  ACC,vf2,vf3
	vmadday.xy  ACC,vf2,vf3
	vmadday.xz  ACC,vf2,vf3
	vmadday.yz  ACC,vf2,vf3
	vmadday.wxy  ACC,vf2,vf3
	vmadday.wxz  ACC,vf2,vf3
	vmadday.wyz  ACC,vf2,vf3
	vmadday.xyz  ACC,vf2,vf3
	vmadday.wxyz  ACC,vf2,vf3
	vmaddaz.w  ACC,vf2,vf3
	vmaddaz.x  ACC,vf2,vf3
	vmaddaz.y  ACC,vf2,vf3
	vmaddaz.z  ACC,vf2,vf3
	vmaddaz.wx  ACC,vf2,vf3
	vmaddaz.wy  ACC,vf2,vf3
	vmaddaz.wz  ACC,vf2,vf3
	vmaddaz.xy  ACC,vf2,vf3
	vmaddaz.xz  ACC,vf2,vf3
	vmaddaz.yz  ACC,vf2,vf3
	vmaddaz.wxy  ACC,vf2,vf3
	vmaddaz.wxz  ACC,vf2,vf3
	vmaddaz.wyz  ACC,vf2,vf3
	vmaddaz.xyz  ACC,vf2,vf3
	vmaddaz.wxyz  ACC,vf2,vf3
	vmax.w  vf1,vf2,vf3
	vmax.x  vf1,vf2,vf3
	vmax.y  vf1,vf2,vf3
	vmax.z  vf1,vf2,vf3
	vmax.wx  vf1,vf2,vf3
	vmax.wy  vf1,vf2,vf3
	vmax.wz  vf1,vf2,vf3
	vmax.xy  vf1,vf2,vf3
	vmax.xz  vf1,vf2,vf3
	vmax.yz  vf1,vf2,vf3
	vmax.wxy  vf1,vf2,vf3
	vmax.wxz  vf1,vf2,vf3
	vmax.wyz  vf1,vf2,vf3
	vmax.xyz  vf1,vf2,vf3
	vmax.wxyz  vf1,vf2,vf3
	vmaxi.w  vf1,vf2,I
	vmaxi.x  vf1,vf2,I
	vmaxi.y  vf1,vf2,I
	vmaxi.z  vf1,vf2,I
	vmaxi.wx  vf1,vf2,I
	vmaxi.wy  vf1,vf2,I
	vmaxi.wz  vf1,vf2,I
	vmaxi.xy  vf1,vf2,I
	vmaxi.xz  vf1,vf2,I
	vmaxi.yz  vf1,vf2,I
	vmaxi.wxy  vf1,vf2,I
	vmaxi.wxz  vf1,vf2,I
	vmaxi.wyz  vf1,vf2,I
	vmaxi.xyz  vf1,vf2,I
	vmaxi.wxyz  vf1,vf2,I
	vmaxw.w  vf1,vf2,vf3
	vmaxw.x  vf1,vf2,vf3
	vmaxw.y  vf1,vf2,vf3
	vmaxw.z  vf1,vf2,vf3
	vmaxw.wx  vf1,vf2,vf3
	vmaxw.wy  vf1,vf2,vf3
	vmaxw.wz  vf1,vf2,vf3
	vmaxw.xy  vf1,vf2,vf3
	vmaxw.xz  vf1,vf2,vf3
	vmaxw.yz  vf1,vf2,vf3
	vmaxw.wxy  vf1,vf2,vf3
	vmaxw.wxz  vf1,vf2,vf3
	vmaxw.wyz  vf1,vf2,vf3
	vmaxw.xyz  vf1,vf2,vf3
	vmaxw.wxyz  vf1,vf2,vf3
	vmaxx.w  vf1,vf2,vf3
	vmaxx.x  vf1,vf2,vf3
	vmaxx.y  vf1,vf2,vf3
	vmaxx.z  vf1,vf2,vf3
	vmaxx.wx  vf1,vf2,vf3
	vmaxx.wy  vf1,vf2,vf3
	vmaxx.wz  vf1,vf2,vf3
	vmaxx.xy  vf1,vf2,vf3
	vmaxx.xz  vf1,vf2,vf3
	vmaxx.yz  vf1,vf2,vf3
	vmaxx.wxy  vf1,vf2,vf3
	vmaxx.wxz  vf1,vf2,vf3
	vmaxx.wyz  vf1,vf2,vf3
	vmaxx.xyz  vf1,vf2,vf3
	vmaxx.wxyz  vf1,vf2,vf3
	vmaxy.w  vf1,vf2,vf3
	vmaxy.x  vf1,vf2,vf3
	vmaxy.y  vf1,vf2,vf3
	vmaxy.z  vf1,vf2,vf3
	vmaxy.wx  vf1,vf2,vf3
	vmaxy.wy  vf1,vf2,vf3
	vmaxy.wz  vf1,vf2,vf3
	vmaxy.xy  vf1,vf2,vf3
	vmaxy.xz  vf1,vf2,vf3
	vmaxy.yz  vf1,vf2,vf3
	vmaxy.wxy  vf1,vf2,vf3
	vmaxy.wxz  vf1,vf2,vf3
	vmaxy.wyz  vf1,vf2,vf3
	vmaxy.xyz  vf1,vf2,vf3
	vmaxy.wxyz  vf1,vf2,vf3
	vmaxz.w  vf1,vf2,vf3
	vmaxz.x  vf1,vf2,vf3
	vmaxz.y  vf1,vf2,vf3
	vmaxz.z  vf1,vf2,vf3
	vmaxz.wx  vf1,vf2,vf3
	vmaxz.wy  vf1,vf2,vf3
	vmaxz.wz  vf1,vf2,vf3
	vmaxz.xy  vf1,vf2,vf3
	vmaxz.xz  vf1,vf2,vf3
	vmaxz.yz  vf1,vf2,vf3
	vmaxz.wxy  vf1,vf2,vf3
	vmaxz.wxz  vf1,vf2,vf3
	vmaxz.wyz  vf1,vf2,vf3
	vmaxz.xyz  vf1,vf2,vf3
	vmaxz.wxyz  vf1,vf2,vf3
	vmfir.w  vf4,vi2
	vmfir.x  vf4,vi2
	vmfir.y  vf4,vi2
	vmfir.z  vf4,vi2
	vmfir.wx  vf4,vi2
	vmfir.wy  vf4,vi2
	vmfir.wz  vf4,vi2
	vmfir.xy  vf4,vi2
	vmfir.xz  vf4,vi2
	vmfir.yz  vf4,vi2
	vmfir.wxy  vf4,vi2
	vmfir.wxz  vf4,vi2
	vmfir.wyz  vf4,vi2
	vmfir.xyz  vf4,vi2
	vmfir.wxyz  vf4,vi2
	vmini.w  vf1,vf2,vf3
	vmini.x  vf1,vf2,vf3
	vmini.y  vf1,vf2,vf3
	vmini.z  vf1,vf2,vf3
	vmini.wx  vf1,vf2,vf3
	vmini.wy  vf1,vf2,vf3
	vmini.wz  vf1,vf2,vf3
	vmini.xy  vf1,vf2,vf3
	vmini.xz  vf1,vf2,vf3
	vmini.yz  vf1,vf2,vf3
	vmini.wxy  vf1,vf2,vf3
	vmini.wxz  vf1,vf2,vf3
	vmini.wyz  vf1,vf2,vf3
	vmini.xyz  vf1,vf2,vf3
	vmini.wxyz  vf1,vf2,vf3
	vminii.w  vf1,vf2,I
	vminii.x  vf1,vf2,I
	vminii.y  vf1,vf2,I
	vminii.z  vf1,vf2,I
	vminii.wx  vf1,vf2,I
	vminii.wy  vf1,vf2,I
	vminii.wz  vf1,vf2,I
	vminii.xy  vf1,vf2,I
	vminii.xz  vf1,vf2,I
	vminii.yz  vf1,vf2,I
	vminii.wxy  vf1,vf2,I
	vminii.wxz  vf1,vf2,I
	vminii.wyz  vf1,vf2,I
	vminii.xyz  vf1,vf2,I
	vminii.wxyz  vf1,vf2,I
	vminiw.w  vf1,vf2,vf3
	vminiw.x  vf1,vf2,vf3
	vminiw.y  vf1,vf2,vf3
	vminiw.z  vf1,vf2,vf3
	vminiw.wx  vf1,vf2,vf3
	vminiw.wy  vf1,vf2,vf3
	vminiw.wz  vf1,vf2,vf3
	vminiw.xy  vf1,vf2,vf3
	vminiw.xz  vf1,vf2,vf3
	vminiw.yz  vf1,vf2,vf3
	vminiw.wxy  vf1,vf2,vf3
	vminiw.wxz  vf1,vf2,vf3
	vminiw.wyz  vf1,vf2,vf3
	vminiw.xyz  vf1,vf2,vf3
	vminiw.wxyz  vf1,vf2,vf3
	vminix.w  vf1,vf2,vf3
	vminix.x  vf1,vf2,vf3
	vminix.y  vf1,vf2,vf3
	vminix.z  vf1,vf2,vf3
	vminix.wx  vf1,vf2,vf3
	vminix.wy  vf1,vf2,vf3
	vminix.wz  vf1,vf2,vf3
	vminix.xy  vf1,vf2,vf3
	vminix.xz  vf1,vf2,vf3
	vminix.yz  vf1,vf2,vf3
	vminix.wxy  vf1,vf2,vf3
	vminix.wxz  vf1,vf2,vf3
	vminix.wyz  vf1,vf2,vf3
	vminix.xyz  vf1,vf2,vf3
	vminix.wxyz  vf1,vf2,vf3
	vminiy.w  vf1,vf2,vf3
	vminiy.x  vf1,vf2,vf3
	vminiy.y  vf1,vf2,vf3
	vminiy.z  vf1,vf2,vf3
	vminiy.wx  vf1,vf2,vf3
	vminiy.wy  vf1,vf2,vf3
	vminiy.wz  vf1,vf2,vf3
	vminiy.xy  vf1,vf2,vf3
	vminiy.xz  vf1,vf2,vf3
	vminiy.yz  vf1,vf2,vf3
	vminiy.wxy  vf1,vf2,vf3
	vminiy.wxz  vf1,vf2,vf3
	vminiy.wyz  vf1,vf2,vf3
	vminiy.xyz  vf1,vf2,vf3
	vminiy.wxyz  vf1,vf2,vf3
	vminiz.w  vf1,vf2,vf3
	vminiz.x  vf1,vf2,vf3
	vminiz.y  vf1,vf2,vf3
	vminiz.z  vf1,vf2,vf3
	vminiz.wx  vf1,vf2,vf3
	vminiz.wy  vf1,vf2,vf3
	vminiz.wz  vf1,vf2,vf3
	vminiz.xy  vf1,vf2,vf3
	vminiz.xz  vf1,vf2,vf3
	vminiz.yz  vf1,vf2,vf3
	vminiz.wxy  vf1,vf2,vf3
	vminiz.wxz  vf1,vf2,vf3
	vminiz.wyz  vf1,vf2,vf3
	vminiz.xyz  vf1,vf2,vf3
	vminiz.wxyz  vf1,vf2,vf3
	vmove.w  vf1,vf2
	vmove.x  vf1,vf2
	vmove.y  vf1,vf2
	vmove.z  vf1,vf2
	vmove.wx  vf1,vf2
	vmove.wy  vf1,vf2
	vmove.wz  vf1,vf2
	vmove.xy  vf1,vf2
	vmove.xz  vf1,vf2
	vmove.yz  vf1,vf2
	vmove.wxy  vf1,vf2
	vmove.wxz  vf1,vf2
	vmove.wyz  vf1,vf2
	vmove.xyz  vf1,vf2
	vmove.wxyz  vf1,vf2
	vmr32.w  vf1,vf2
	vmr32.x  vf1,vf2
	vmr32.y  vf1,vf2
	vmr32.z  vf1,vf2
	vmr32.wx  vf1,vf2
	vmr32.wy  vf1,vf2
	vmr32.wz  vf1,vf2
	vmr32.xy  vf1,vf2
	vmr32.xz  vf1,vf2
	vmr32.yz  vf1,vf2
	vmr32.wxy  vf1,vf2
	vmr32.wxz  vf1,vf2
	vmr32.wyz  vf1,vf2
	vmr32.xyz  vf1,vf2
	vmr32.wxyz  vf1,vf2
	vmsub.w  vf1,vf2,vf3
	vmsub.x  vf1,vf2,vf3
	vmsub.y  vf1,vf2,vf3
	vmsub.z  vf1,vf2,vf3
	vmsub.wx  vf1,vf2,vf3
	vmsub.wy  vf1,vf2,vf3
	vmsub.wz  vf1,vf2,vf3
	vmsub.xy  vf1,vf2,vf3
	vmsub.xz  vf1,vf2,vf3
	vmsub.yz  vf1,vf2,vf3
	vmsub.wxy  vf1,vf2,vf3
	vmsub.wxz  vf1,vf2,vf3
	vmsub.wyz  vf1,vf2,vf3
	vmsub.xyz  vf1,vf2,vf3
	vmsub.wxyz  vf1,vf2,vf3
	vmsubi.w  vf1,vf2,I
	vmsubi.x  vf1,vf2,I
	vmsubi.y  vf1,vf2,I
	vmsubi.z  vf1,vf2,I
	vmsubi.wx  vf1,vf2,I
	vmsubi.wy  vf1,vf2,I
	vmsubi.wz  vf1,vf2,I
	vmsubi.xy  vf1,vf2,I
	vmsubi.xz  vf1,vf2,I
	vmsubi.yz  vf1,vf2,I
	vmsubi.wxy  vf1,vf2,I
	vmsubi.wxz  vf1,vf2,I
	vmsubi.wyz  vf1,vf2,I
	vmsubi.xyz  vf1,vf2,I
	vmsubi.wxyz  vf1,vf2,I
	vmsubq.w  vf1,vf2,Q
	vmsubq.x  vf1,vf2,Q
	vmsubq.y  vf1,vf2,Q
	vmsubq.z  vf1,vf2,Q
	vmsubq.wx  vf1,vf2,Q
	vmsubq.wy  vf1,vf2,Q
	vmsubq.wz  vf1,vf2,Q
	vmsubq.xy  vf1,vf2,Q
	vmsubq.xz  vf1,vf2,Q
	vmsubq.yz  vf1,vf2,Q
	vmsubq.wxy  vf1,vf2,Q
	vmsubq.wxz  vf1,vf2,Q
	vmsubq.wyz  vf1,vf2,Q
	vmsubq.xyz  vf1,vf2,Q
	vmsubq.wxyz  vf1,vf2,Q
	vmsubw.w  vf1,vf2,vf3
	vmsubw.x  vf1,vf2,vf3
	vmsubw.y  vf1,vf2,vf3
	vmsubw.z  vf1,vf2,vf3
	vmsubw.wx  vf1,vf2,vf3
	vmsubw.wy  vf1,vf2,vf3
	vmsubw.wz  vf1,vf2,vf3
	vmsubw.xy  vf1,vf2,vf3
	vmsubw.xz  vf1,vf2,vf3
	vmsubw.yz  vf1,vf2,vf3
	vmsubw.wxy  vf1,vf2,vf3
	vmsubw.wxz  vf1,vf2,vf3
	vmsubw.wyz  vf1,vf2,vf3
	vmsubw.xyz  vf1,vf2,vf3
	vmsubw.wxyz  vf1,vf2,vf3
	vmsubx.w  vf1,vf2,vf3
	vmsubx.x  vf1,vf2,vf3
	vmsubx.y  vf1,vf2,vf3
	vmsubx.z  vf1,vf2,vf3
	vmsubx.wx  vf1,vf2,vf3
	vmsubx.wy  vf1,vf2,vf3
	vmsubx.wz  vf1,vf2,vf3
	vmsubx.xy  vf1,vf2,vf3
	vmsubx.xz  vf1,vf2,vf3
	vmsubx.yz  vf1,vf2,vf3
	vmsubx.wxy  vf1,vf2,vf3
	vmsubx.wxz  vf1,vf2,vf3
	vmsubx.wyz  vf1,vf2,vf3
	vmsubx.xyz  vf1,vf2,vf3
	vmsubx.wxyz  vf1,vf2,vf3
	vmsuby.w  vf1,vf2,vf3
	vmsuby.x  vf1,vf2,vf3
	vmsuby.y  vf1,vf2,vf3
	vmsuby.z  vf1,vf2,vf3
	vmsuby.wx  vf1,vf2,vf3
	vmsuby.wy  vf1,vf2,vf3
	vmsuby.wz  vf1,vf2,vf3
	vmsuby.xy  vf1,vf2,vf3
	vmsuby.xz  vf1,vf2,vf3
	vmsuby.yz  vf1,vf2,vf3
	vmsuby.wxy  vf1,vf2,vf3
	vmsuby.wxz  vf1,vf2,vf3
	vmsuby.wyz  vf1,vf2,vf3
	vmsuby.xyz  vf1,vf2,vf3
	vmsuby.wxyz  vf1,vf2,vf3
	vmsubz.w  vf1,vf2,vf3
	vmsubz.x  vf1,vf2,vf3
	vmsubz.y  vf1,vf2,vf3
	vmsubz.z  vf1,vf2,vf3
	vmsubz.wx  vf1,vf2,vf3
	vmsubz.wy  vf1,vf2,vf3
	vmsubz.wz  vf1,vf2,vf3
	vmsubz.xy  vf1,vf2,vf3
	vmsubz.xz  vf1,vf2,vf3
	vmsubz.yz  vf1,vf2,vf3
	vmsubz.wxy  vf1,vf2,vf3
	vmsubz.wxz  vf1,vf2,vf3
	vmsubz.wyz  vf1,vf2,vf3
	vmsubz.xyz  vf1,vf2,vf3
	vmsubz.wxyz  vf1,vf2,vf3
	vmsuba.w  ACC,vf2,vf3
	vmsuba.x  ACC,vf2,vf3
	vmsuba.y  ACC,vf2,vf3
	vmsuba.z  ACC,vf2,vf3
	vmsuba.wx  ACC,vf2,vf3
	vmsuba.wy  ACC,vf2,vf3
	vmsuba.wz  ACC,vf2,vf3
	vmsuba.xy  ACC,vf2,vf3
	vmsuba.xz  ACC,vf2,vf3
	vmsuba.yz  ACC,vf2,vf3
	vmsuba.wxy  ACC,vf2,vf3
	vmsuba.wxz  ACC,vf2,vf3
	vmsuba.wyz  ACC,vf2,vf3
	vmsuba.xyz  ACC,vf2,vf3
	vmsuba.wxyz  ACC,vf2,vf3
	vmsubai.w  ACC,vf2,I
	vmsubai.x  ACC,vf2,I
	vmsubai.y  ACC,vf2,I
	vmsubai.z  ACC,vf2,I
	vmsubai.wx  ACC,vf2,I
	vmsubai.wy  ACC,vf2,I
	vmsubai.wz  ACC,vf2,I
	vmsubai.xy  ACC,vf2,I
	vmsubai.xz  ACC,vf2,I
	vmsubai.yz  ACC,vf2,I
	vmsubai.wxy  ACC,vf2,I
	vmsubai.wxz  ACC,vf2,I
	vmsubai.wyz  ACC,vf2,I
	vmsubai.xyz  ACC,vf2,I
	vmsubai.wxyz  ACC,vf2,I
	vmsubaq.w  ACC,vf2,Q
	vmsubaq.x  ACC,vf2,Q
	vmsubaq.y  ACC,vf2,Q
	vmsubaq.z  ACC,vf2,Q
	vmsubaq.wx  ACC,vf2,Q
	vmsubaq.wy  ACC,vf2,Q
	vmsubaq.wz  ACC,vf2,Q
	vmsubaq.xy  ACC,vf2,Q
	vmsubaq.xz  ACC,vf2,Q
	vmsubaq.yz  ACC,vf2,Q
	vmsubaq.wxy  ACC,vf2,Q
	vmsubaq.wxz  ACC,vf2,Q
	vmsubaq.wyz  ACC,vf2,Q
	vmsubaq.xyz  ACC,vf2,Q
	vmsubaq.wxyz  ACC,vf2,Q
	vmsubaw.w  ACC,vf2,vf3
	vmsubaw.x  ACC,vf2,vf3
	vmsubaw.y  ACC,vf2,vf3
	vmsubaw.z  ACC,vf2,vf3
	vmsubaw.wx  ACC,vf2,vf3
	vmsubaw.wy  ACC,vf2,vf3
	vmsubaw.wz  ACC,vf2,vf3
	vmsubaw.xy  ACC,vf2,vf3
	vmsubaw.xz  ACC,vf2,vf3
	vmsubaw.yz  ACC,vf2,vf3
	vmsubaw.wxy  ACC,vf2,vf3
	vmsubaw.wxz  ACC,vf2,vf3
	vmsubaw.wyz  ACC,vf2,vf3
	vmsubaw.xyz  ACC,vf2,vf3
	vmsubaw.wxyz  ACC,vf2,vf3
	vmsubax.w  ACC,vf2,vf3
	vmsubax.x  ACC,vf2,vf3
	vmsubax.y  ACC,vf2,vf3
	vmsubax.z  ACC,vf2,vf3
	vmsubax.wx  ACC,vf2,vf3
	vmsubax.wy  ACC,vf2,vf3
	vmsubax.wz  ACC,vf2,vf3
	vmsubax.xy  ACC,vf2,vf3
	vmsubax.xz  ACC,vf2,vf3
	vmsubax.yz  ACC,vf2,vf3
	vmsubax.wxy  ACC,vf2,vf3
	vmsubax.wxz  ACC,vf2,vf3
	vmsubax.wyz  ACC,vf2,vf3
	vmsubax.xyz  ACC,vf2,vf3
	vmsubax.wxyz  ACC,vf2,vf3
	vmsubay.w  ACC,vf2,vf3
	vmsubay.x  ACC,vf2,vf3
	vmsubay.y  ACC,vf2,vf3
	vmsubay.z  ACC,vf2,vf3
	vmsubay.wx  ACC,vf2,vf3
	vmsubay.wy  ACC,vf2,vf3
	vmsubay.wz  ACC,vf2,vf3
	vmsubay.xy  ACC,vf2,vf3
	vmsubay.xz  ACC,vf2,vf3
	vmsubay.yz  ACC,vf2,vf3
	vmsubay.wxy  ACC,vf2,vf3
	vmsubay.wxz  ACC,vf2,vf3
	vmsubay.wyz  ACC,vf2,vf3
	vmsubay.xyz  ACC,vf2,vf3
	vmsubay.wxyz  ACC,vf2,vf3
	vmsubaz.w  ACC,vf2,vf3
	vmsubaz.x  ACC,vf2,vf3
	vmsubaz.y  ACC,vf2,vf3
	vmsubaz.z  ACC,vf2,vf3
	vmsubaz.wx  ACC,vf2,vf3
	vmsubaz.wy  ACC,vf2,vf3
	vmsubaz.wz  ACC,vf2,vf3
	vmsubaz.xy  ACC,vf2,vf3
	vmsubaz.xz  ACC,vf2,vf3
	vmsubaz.yz  ACC,vf2,vf3
	vmsubaz.wxy  ACC,vf2,vf3
	vmsubaz.wxz  ACC,vf2,vf3
	vmsubaz.wyz  ACC,vf2,vf3
	vmsubaz.xyz  ACC,vf2,vf3
	vmsubaz.wxyz  ACC,vf2,vf3
	vmtir  vi2,vf4w
	vmtir  vi2,vf4x
	vmtir  vi2,vf4y
	vmtir  vi2,vf4z
	vmul.w  vf1,vf2,vf3
	vmul.x  vf1,vf2,vf3
	vmul.y  vf1,vf2,vf3
	vmul.z  vf1,vf2,vf3
	vmul.wx  vf1,vf2,vf3
	vmul.wy  vf1,vf2,vf3
	vmul.wz  vf1,vf2,vf3
	vmul.xy  vf1,vf2,vf3
	vmul.xz  vf1,vf2,vf3
	vmul.yz  vf1,vf2,vf3
	vmul.wxy  vf1,vf2,vf3
	vmul.wxz  vf1,vf2,vf3
	vmul.wyz  vf1,vf2,vf3
	vmul.xyz  vf1,vf2,vf3
	vmul.wxyz  vf1,vf2,vf3
	vmuli.w  vf1,vf2,I
	vmuli.x  vf1,vf2,I
	vmuli.y  vf1,vf2,I
	vmuli.z  vf1,vf2,I
	vmuli.wx  vf1,vf2,I
	vmuli.wy  vf1,vf2,I
	vmuli.wz  vf1,vf2,I
	vmuli.xy  vf1,vf2,I
	vmuli.xz  vf1,vf2,I
	vmuli.yz  vf1,vf2,I
	vmuli.wxy  vf1,vf2,I
	vmuli.wxz  vf1,vf2,I
	vmuli.wyz  vf1,vf2,I
	vmuli.xyz  vf1,vf2,I
	vmuli.wxyz  vf1,vf2,I
	vmulq.w  vf1,vf2,Q
	vmulq.x  vf1,vf2,Q
	vmulq.y  vf1,vf2,Q
	vmulq.z  vf1,vf2,Q
	vmulq.wx  vf1,vf2,Q
	vmulq.wy  vf1,vf2,Q
	vmulq.wz  vf1,vf2,Q
	vmulq.xy  vf1,vf2,Q
	vmulq.xz  vf1,vf2,Q
	vmulq.yz  vf1,vf2,Q
	vmulq.wxy  vf1,vf2,Q
	vmulq.wxz  vf1,vf2,Q
	vmulq.wyz  vf1,vf2,Q
	vmulq.xyz  vf1,vf2,Q
	vmulq.wxyz  vf1,vf2,Q
	vmulw.w  vf1,vf2,vf3
	vmulw.x  vf1,vf2,vf3
	vmulw.y  vf1,vf2,vf3
	vmulw.z  vf1,vf2,vf3
	vmulw.wx  vf1,vf2,vf3
	vmulw.wy  vf1,vf2,vf3
	vmulw.wz  vf1,vf2,vf3
	vmulw.xy  vf1,vf2,vf3
	vmulw.xz  vf1,vf2,vf3
	vmulw.yz  vf1,vf2,vf3
	vmulw.wxy  vf1,vf2,vf3
	vmulw.wxz  vf1,vf2,vf3
	vmulw.wyz  vf1,vf2,vf3
	vmulw.xyz  vf1,vf2,vf3
	vmulw.wxyz  vf1,vf2,vf3
	vmulx.w  vf1,vf2,vf3
	vmulx.x  vf1,vf2,vf3
	vmulx.y  vf1,vf2,vf3
	vmulx.z  vf1,vf2,vf3
	vmulx.wx  vf1,vf2,vf3
	vmulx.wy  vf1,vf2,vf3
	vmulx.wz  vf1,vf2,vf3
	vmulx.xy  vf1,vf2,vf3
	vmulx.xz  vf1,vf2,vf3
	vmulx.yz  vf1,vf2,vf3
	vmulx.wxy  vf1,vf2,vf3
	vmulx.wxz  vf1,vf2,vf3
	vmulx.wyz  vf1,vf2,vf3
	vmulx.xyz  vf1,vf2,vf3
	vmulx.wxyz  vf1,vf2,vf3
	vmuly.w  vf1,vf2,vf3
	vmuly.x  vf1,vf2,vf3
	vmuly.y  vf1,vf2,vf3
	vmuly.z  vf1,vf2,vf3
	vmuly.wx  vf1,vf2,vf3
	vmuly.wy  vf1,vf2,vf3
	vmuly.wz  vf1,vf2,vf3
	vmuly.xy  vf1,vf2,vf3
	vmuly.xz  vf1,vf2,vf3
	vmuly.yz  vf1,vf2,vf3
	vmuly.wxy  vf1,vf2,vf3
	vmuly.wxz  vf1,vf2,vf3
	vmuly.wyz  vf1,vf2,vf3
	vmuly.xyz  vf1,vf2,vf3
	vmuly.wxyz  vf1,vf2,vf3
	vmulz.w  vf1,vf2,vf3
	vmulz.x  vf1,vf2,vf3
	vmulz.y  vf1,vf2,vf3
	vmulz.z  vf1,vf2,vf3
	vmulz.wx  vf1,vf2,vf3
	vmulz.wy  vf1,vf2,vf3
	vmulz.wz  vf1,vf2,vf3
	vmulz.xy  vf1,vf2,vf3
	vmulz.xz  vf1,vf2,vf3
	vmulz.yz  vf1,vf2,vf3
	vmulz.wxy  vf1,vf2,vf3
	vmulz.wxz  vf1,vf2,vf3
	vmulz.wyz  vf1,vf2,vf3
	vmulz.xyz  vf1,vf2,vf3
	vmulz.wxyz  vf1,vf2,vf3
	vmula.w  ACC,vf2,vf3
	vmula.x  ACC,vf2,vf3
	vmula.y  ACC,vf2,vf3
	vmula.z  ACC,vf2,vf3
	vmula.wx  ACC,vf2,vf3
	vmula.wy  ACC,vf2,vf3
	vmula.wz  ACC,vf2,vf3
	vmula.xy  ACC,vf2,vf3
	vmula.xz  ACC,vf2,vf3
	vmula.yz  ACC,vf2,vf3
	vmula.wxy  ACC,vf2,vf3
	vmula.wxz  ACC,vf2,vf3
	vmula.wyz  ACC,vf2,vf3
	vmula.xyz  ACC,vf2,vf3
	vmula.wxyz  ACC,vf2,vf3
	vmulai.w  ACC,vf2,I
	vmulai.x  ACC,vf2,I
	vmulai.y  ACC,vf2,I
	vmulai.z  ACC,vf2,I
	vmulai.wx  ACC,vf2,I
	vmulai.wy  ACC,vf2,I
	vmulai.wz  ACC,vf2,I
	vmulai.xy  ACC,vf2,I
	vmulai.xz  ACC,vf2,I
	vmulai.yz  ACC,vf2,I
	vmulai.wxy  ACC,vf2,I
	vmulai.wxz  ACC,vf2,I
	vmulai.wyz  ACC,vf2,I
	vmulai.xyz  ACC,vf2,I
	vmulai.wxyz  ACC,vf2,I
	vmulaq.w  ACC,vf2,Q
	vmulaq.x  ACC,vf2,Q
	vmulaq.y  ACC,vf2,Q
	vmulaq.z  ACC,vf2,Q
	vmulaq.wx  ACC,vf2,Q
	vmulaq.wy  ACC,vf2,Q
	vmulaq.wz  ACC,vf2,Q
	vmulaq.xy  ACC,vf2,Q
	vmulaq.xz  ACC,vf2,Q
	vmulaq.yz  ACC,vf2,Q
	vmulaq.wxy  ACC,vf2,Q
	vmulaq.wxz  ACC,vf2,Q
	vmulaq.wyz  ACC,vf2,Q
	vmulaq.xyz  ACC,vf2,Q
	vmulaq.wxyz  ACC,vf2,Q
	vmulaw.w  ACC,vf2,vf3
	vmulaw.x  ACC,vf2,vf3
	vmulaw.y  ACC,vf2,vf3
	vmulaw.z  ACC,vf2,vf3
	vmulaw.wx  ACC,vf2,vf3
	vmulaw.wy  ACC,vf2,vf3
	vmulaw.wz  ACC,vf2,vf3
	vmulaw.xy  ACC,vf2,vf3
	vmulaw.xz  ACC,vf2,vf3
	vmulaw.yz  ACC,vf2,vf3
	vmulaw.wxy  ACC,vf2,vf3
	vmulaw.wxz  ACC,vf2,vf3
	vmulaw.wyz  ACC,vf2,vf3
	vmulaw.xyz  ACC,vf2,vf3
	vmulaw.wxyz  ACC,vf2,vf3
	vmulax.w  ACC,vf2,vf3
	vmulax.x  ACC,vf2,vf3
	vmulax.y  ACC,vf2,vf3
	vmulax.z  ACC,vf2,vf3
	vmulax.wx  ACC,vf2,vf3
	vmulax.wy  ACC,vf2,vf3
	vmulax.wz  ACC,vf2,vf3
	vmulax.xy  ACC,vf2,vf3
	vmulax.xz  ACC,vf2,vf3
	vmulax.yz  ACC,vf2,vf3
	vmulax.wxy  ACC,vf2,vf3
	vmulax.wxz  ACC,vf2,vf3
	vmulax.wyz  ACC,vf2,vf3
	vmulax.xyz  ACC,vf2,vf3
	vmulax.wxyz  ACC,vf2,vf3
	vmulay.w  ACC,vf2,vf3
	vmulay.x  ACC,vf2,vf3
	vmulay.y  ACC,vf2,vf3
	vmulay.z  ACC,vf2,vf3
	vmulay.wx  ACC,vf2,vf3
	vmulay.wy  ACC,vf2,vf3
	vmulay.wz  ACC,vf2,vf3
	vmulay.xy  ACC,vf2,vf3
	vmulay.xz  ACC,vf2,vf3
	vmulay.yz  ACC,vf2,vf3
	vmulay.wxy  ACC,vf2,vf3
	vmulay.wxz  ACC,vf2,vf3
	vmulay.wyz  ACC,vf2,vf3
	vmulay.xyz  ACC,vf2,vf3
	vmulay.wxyz  ACC,vf2,vf3
	vmulaz.w  ACC,vf2,vf3
	vmulaz.x  ACC,vf2,vf3
	vmulaz.y  ACC,vf2,vf3
	vmulaz.z  ACC,vf2,vf3
	vmulaz.wx  ACC,vf2,vf3
	vmulaz.wy  ACC,vf2,vf3
	vmulaz.wz  ACC,vf2,vf3
	vmulaz.xy  ACC,vf2,vf3
	vmulaz.xz  ACC,vf2,vf3
	vmulaz.yz  ACC,vf2,vf3
	vmulaz.wxy  ACC,vf2,vf3
	vmulaz.wxz  ACC,vf2,vf3
	vmulaz.wyz  ACC,vf2,vf3
	vmulaz.xyz  ACC,vf2,vf3
	vmulaz.wxyz  ACC,vf2,vf3
	vnop
	vnop
	vnop
	vnop
	vnop
	vnop
	vnop
	vopmula.xyz  ACC,vf2,vf3
	vnop
	vnop
	vnop
	vnop
	vnop
	vnop
	vopmsub.xyz  vf1,vf2,vf3
	vrget.x vf2,R
	vrinit   R,vf2y
	vrnext.x vf2,R
	vrsqrt Q,vf2x,vf3y
	vrxor  R,vf2y
	vsqi.w  vf4,(vi2++)
	vsqi.x  vf4,(vi2++)
	vsqi.y  vf4,(vi2++)
	vsqi.z  vf4,(vi2++)
	vsqi.wx  vf4,(vi2++)
	vsqi.wy  vf4,(vi2++)
	vsqi.wz  vf4,(vi2++)
	vsqi.xy  vf4,(vi2++)
	vsqi.xz  vf4,(vi2++)
	vsqi.yz  vf4,(vi2++)
	vsqi.wxy  vf4,(vi2++)
	vsqi.wxz  vf4,(vi2++)
	vsqi.wyz  vf4,(vi2++)
	vsqi.xyz  vf4,(vi2++)
	vsqi.wxyz  vf4,(vi2++)
	vsqd.w  vf4,(--vi2)
	vsqd.x  vf4,(--vi2)
	vsqd.y  vf4,(--vi2)
	vsqd.z  vf4,(--vi2)
	vsqd.wx  vf4,(--vi2)
	vsqd.wy  vf4,(--vi2)
	vsqd.wz  vf4,(--vi2)
	vsqd.xy  vf4,(--vi2)
	vsqd.xz  vf4,(--vi2)
	vsqd.yz  vf4,(--vi2)
	vsqd.wxy  vf4,(--vi2)
	vsqd.wxz  vf4,(--vi2)
	vsqd.wyz  vf4,(--vi2)
	vsqd.xyz  vf4,(--vi2)
	vsqd.wxyz  vf4,(--vi2)
	vsqrt	Q,vf4w
	vsub.w  vf1,vf2,vf3
	vsub.x  vf1,vf2,vf3
	vsub.y  vf1,vf2,vf3
	vsub.z  vf1,vf2,vf3
	vsub.wx  vf1,vf2,vf3
	vsub.wy  vf1,vf2,vf3
	vsub.wz  vf1,vf2,vf3
	vsub.xy  vf1,vf2,vf3
	vsub.xz  vf1,vf2,vf3
	vsub.yz  vf1,vf2,vf3
	vsub.wxy  vf1,vf2,vf3
	vsub.wxz  vf1,vf2,vf3
	vsub.wyz  vf1,vf2,vf3
	vsub.xyz  vf1,vf2,vf3
	vsub.wxyz  vf1,vf2,vf3
	vsubi.w  vf1,vf2,I
	vsubi.x  vf1,vf2,I
	vsubi.y  vf1,vf2,I
	vsubi.z  vf1,vf2,I
	vsubi.wx  vf1,vf2,I
	vsubi.wy  vf1,vf2,I
	vsubi.wz  vf1,vf2,I
	vsubi.xy  vf1,vf2,I
	vsubi.xz  vf1,vf2,I
	vsubi.yz  vf1,vf2,I
	vsubi.wxy  vf1,vf2,I
	vsubi.wxz  vf1,vf2,I
	vsubi.wyz  vf1,vf2,I
	vsubi.xyz  vf1,vf2,I
	vsubi.wxyz  vf1,vf2,I
	vsubq.w  vf1,vf2,Q
	vsubq.x  vf1,vf2,Q
	vsubq.y  vf1,vf2,Q
	vsubq.z  vf1,vf2,Q
	vsubq.wx  vf1,vf2,Q
	vsubq.wy  vf1,vf2,Q
	vsubq.wz  vf1,vf2,Q
	vsubq.xy  vf1,vf2,Q
	vsubq.xz  vf1,vf2,Q
	vsubq.yz  vf1,vf2,Q
	vsubq.wxy  vf1,vf2,Q
	vsubq.wxz  vf1,vf2,Q
	vsubq.wyz  vf1,vf2,Q
	vsubq.xyz  vf1,vf2,Q
	vsubq.wxyz  vf1,vf2,Q
	vsubw.w  vf1,vf2,vf3
	vsubw.x  vf1,vf2,vf3
	vsubw.y  vf1,vf2,vf3
	vsubw.z  vf1,vf2,vf3
	vsubw.wx  vf1,vf2,vf3
	vsubw.wy  vf1,vf2,vf3
	vsubw.wz  vf1,vf2,vf3
	vsubw.xy  vf1,vf2,vf3
	vsubw.xz  vf1,vf2,vf3
	vsubw.yz  vf1,vf2,vf3
	vsubw.wxy  vf1,vf2,vf3
	vsubw.wxz  vf1,vf2,vf3
	vsubw.wyz  vf1,vf2,vf3
	vsubw.xyz  vf1,vf2,vf3
	vsubw.wxyz  vf1,vf2,vf3
	vsubx.w  vf1,vf2,vf3
	vsubx.x  vf1,vf2,vf3
	vsubx.y  vf1,vf2,vf3
	vsubx.z  vf1,vf2,vf3
	vsubx.wx  vf1,vf2,vf3
	vsubx.wy  vf1,vf2,vf3
	vsubx.wz  vf1,vf2,vf3
	vsubx.xy  vf1,vf2,vf3
	vsubx.xz  vf1,vf2,vf3
	vsubx.yz  vf1,vf2,vf3
	vsubx.wxy  vf1,vf2,vf3
	vsubx.wxz  vf1,vf2,vf3
	vsubx.wyz  vf1,vf2,vf3
	vsubx.xyz  vf1,vf2,vf3
	vsubx.wxyz  vf1,vf2,vf3
	vsuby.w  vf1,vf2,vf3
	vsuby.x  vf1,vf2,vf3
	vsuby.y  vf1,vf2,vf3
	vsuby.z  vf1,vf2,vf3
	vsuby.wx  vf1,vf2,vf3
	vsuby.wy  vf1,vf2,vf3
	vsuby.wz  vf1,vf2,vf3
	vsuby.xy  vf1,vf2,vf3
	vsuby.xz  vf1,vf2,vf3
	vsuby.yz  vf1,vf2,vf3
	vsuby.wxy  vf1,vf2,vf3
	vsuby.wxz  vf1,vf2,vf3
	vsuby.wyz  vf1,vf2,vf3
	vsuby.xyz  vf1,vf2,vf3
	vsuby.wxyz  vf1,vf2,vf3
	vsubz.w  vf1,vf2,vf3
	vsubz.x  vf1,vf2,vf3
	vsubz.y  vf1,vf2,vf3
	vsubz.z  vf1,vf2,vf3
	vsubz.wx  vf1,vf2,vf3
	vsubz.wy  vf1,vf2,vf3
	vsubz.wz  vf1,vf2,vf3
	vsubz.xy  vf1,vf2,vf3
	vsubz.xz  vf1,vf2,vf3
	vsubz.yz  vf1,vf2,vf3
	vsubz.wxy  vf1,vf2,vf3
	vsubz.wxz  vf1,vf2,vf3
	vsubz.wyz  vf1,vf2,vf3
	vsubz.xyz  vf1,vf2,vf3
	vsubz.wxyz  vf1,vf2,vf3
	vsuba.w  ACC,vf2,vf3
	vsuba.x  ACC,vf2,vf3
	vsuba.y  ACC,vf2,vf3
	vsuba.z  ACC,vf2,vf3
	vsuba.wx  ACC,vf2,vf3
	vsuba.wy  ACC,vf2,vf3
	vsuba.wz  ACC,vf2,vf3
	vsuba.xy  ACC,vf2,vf3
	vsuba.xz  ACC,vf2,vf3
	vsuba.yz  ACC,vf2,vf3
	vsuba.wxy  ACC,vf2,vf3
	vsuba.wxz  ACC,vf2,vf3
	vsuba.wyz  ACC,vf2,vf3
	vsuba.xyz  ACC,vf2,vf3
	vsuba.wxyz  ACC,vf2,vf3
	vsubai.w  ACC,vf2,I
	vsubai.x  ACC,vf2,I
	vsubai.y  ACC,vf2,I
	vsubai.z  ACC,vf2,I
	vsubai.wx  ACC,vf2,I
	vsubai.wy  ACC,vf2,I
	vsubai.wz  ACC,vf2,I
	vsubai.xy  ACC,vf2,I
	vsubai.xz  ACC,vf2,I
	vsubai.yz  ACC,vf2,I
	vsubai.wxy  ACC,vf2,I
	vsubai.wxz  ACC,vf2,I
	vsubai.wyz  ACC,vf2,I
	vsubai.xyz  ACC,vf2,I
	vsubai.wxyz  ACC,vf2,I
	vsubaq.w  ACC,vf2,Q
	vsubaq.x  ACC,vf2,Q
	vsubaq.y  ACC,vf2,Q
	vsubaq.z  ACC,vf2,Q
	vsubaq.wx  ACC,vf2,Q
	vsubaq.wy  ACC,vf2,Q
	vsubaq.wz  ACC,vf2,Q
	vsubaq.xy  ACC,vf2,Q
	vsubaq.xz  ACC,vf2,Q
	vsubaq.yz  ACC,vf2,Q
	vsubaq.wxy  ACC,vf2,Q
	vsubaq.wxz  ACC,vf2,Q
	vsubaq.wyz  ACC,vf2,Q
	vsubaq.xyz  ACC,vf2,Q
	vsubaq.wxyz  ACC,vf2,Q
	vsubaw.w  ACC,vf2,vf3
	vsubaw.x  ACC,vf2,vf3
	vsubaw.y  ACC,vf2,vf3
	vsubaw.z  ACC,vf2,vf3
	vsubaw.wx  ACC,vf2,vf3
	vsubaw.wy  ACC,vf2,vf3
	vsubaw.wz  ACC,vf2,vf3
	vsubaw.xy  ACC,vf2,vf3
	vsubaw.xz  ACC,vf2,vf3
	vsubaw.yz  ACC,vf2,vf3
	vsubaw.wxy  ACC,vf2,vf3
	vsubaw.wxz  ACC,vf2,vf3
	vsubaw.wyz  ACC,vf2,vf3
	vsubaw.xyz  ACC,vf2,vf3
	vsubaw.wxyz  ACC,vf2,vf3
	vsubax.w  ACC,vf2,vf3
	vsubax.x  ACC,vf2,vf3
	vsubax.y  ACC,vf2,vf3
	vsubax.z  ACC,vf2,vf3
	vsubax.wx  ACC,vf2,vf3
	vsubax.wy  ACC,vf2,vf3
	vsubax.wz  ACC,vf2,vf3
	vsubax.xy  ACC,vf2,vf3
	vsubax.xz  ACC,vf2,vf3
	vsubax.yz  ACC,vf2,vf3
	vsubax.wxy  ACC,vf2,vf3
	vsubax.wxz  ACC,vf2,vf3
	vsubax.wyz  ACC,vf2,vf3
	vsubax.xyz  ACC,vf2,vf3
	vsubax.wxyz  ACC,vf2,vf3
	vsubay.w  ACC,vf2,vf3
	vsubay.x  ACC,vf2,vf3
	vsubay.y  ACC,vf2,vf3
	vsubay.z  ACC,vf2,vf3
	vsubay.wx  ACC,vf2,vf3
	vsubay.wy  ACC,vf2,vf3
	vsubay.wz  ACC,vf2,vf3
	vsubay.xy  ACC,vf2,vf3
	vsubay.xz  ACC,vf2,vf3
	vsubay.yz  ACC,vf2,vf3
	vsubay.wxy  ACC,vf2,vf3
	vsubay.wxz  ACC,vf2,vf3
	vsubay.wyz  ACC,vf2,vf3
	vsubay.xyz  ACC,vf2,vf3
	vsubay.wxyz  ACC,vf2,vf3
	vsubaz.w  ACC,vf2,vf3
	vsubaz.x  ACC,vf2,vf3
	vsubaz.y  ACC,vf2,vf3
	vsubaz.z  ACC,vf2,vf3
	vsubaz.wx  ACC,vf2,vf3
	vsubaz.wy  ACC,vf2,vf3
	vsubaz.wz  ACC,vf2,vf3
	vsubaz.xy  ACC,vf2,vf3
	vsubaz.xz  ACC,vf2,vf3
	vsubaz.yz  ACC,vf2,vf3
	vsubaz.wxy  ACC,vf2,vf3
	vsubaz.wxz  ACC,vf2,vf3
	vsubaz.wyz  ACC,vf2,vf3
	vsubaz.xyz  ACC,vf2,vf3
	vsubaz.wxyz  ACC,vf2,vf3
	vwaitq
blah:
	bc2f blah1
	nop
blah1:	
	bc2fl blah2
	nop
blah2:	
	bc2t blah3
	nop
blah3:	
	bc2tl blah4
	nop
blah4:	
	cfc2	$5,$3
 	cfc2.i	$5,$3
 	cfc2.ni	$5,$3
	ctc2	$5,$3
 	ctc2.i	$5,$3
 	ctc2.ni	$5,$3

end:
7:	
#	exit with RC=0
	dli	$4,0x0000
	break 1023
	nop
	b	7b
	nop

error:
8:	
#	exit with RC=16
	dli	$4,0x0010
	break	1023
	nop
	b	8b
	nop
