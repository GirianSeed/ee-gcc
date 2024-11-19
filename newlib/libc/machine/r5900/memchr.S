/* Assembler version of memchr using quadword instructions.
	
   Jeff Johnston, Cygnus Solutions, Feb 10/1999.

   ============================================================
   Copyright (C) 1999 by Cygnus Solutions. All rights reserved.
 
   Permission to use, copy, modify, and distribute this
   software is freely granted, provided that this notice
   is preserved.
   ============================================================  */
	
	.text
	.align	2
	.globl	memchr
	.text
	.ent	memchr
memchr:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	
#ifndef __OPTIMIZE_SIZE__

	sltu	$2,$6,16
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L2
	andi	$5,$5,0x00ff
	.set	macro
	.set	reorder

	andi	$2,$4,0xf
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L2
	move	$7,$4
	.set	macro
	.set	reorder

/* build a quadword value consisting of the byte to search for */

$L6:
	dsll	$3,$5,8
	move	$10,$4
	daddu	$8,$3,$5
	dli	$2,0x0101010101010101
	dsll	$4,$8,16
	dli	$3,0x8080808080808080
	daddu	$8,$8,$4

	.set	noreorder
	.set	nomacro
	bnel	$10,$0,$L2
	move	$4,$7
	.set	macro
	.set	reorder

/* load a quadword at a time and xor with a quadword made of
   the desired byte.  This leaves a null byte in the result
   wherever the desired byte is located.  To find if any
   bytes are null, we perform the following algorithm on
   the intermediate result which we will call X:
   
   (((X) - 0x01010101010101010101010101010101) & 
      ~(X) & 0x80808080808080808080808080808080)

   The result will be non-zero if a null byte is found and
   hence if the loaded quadword contains the desired byte. */
   
	move	$4,$2
	pcpyld	$9,$8,$8

$L10:		
	lq	$2,0($7)
	pcpyld	$8,$3,$3
	pxor	$2,$2,$9
	pcpyld	$10,$4,$4
	pnor	$3,$0,$2
	psubb	$2,$2,$10
	pand	$2,$2,$3
	pand	$2,$2,$8
	pcpyud	$3,$2,$9
	or	$2,$2,$3
	.set	noreorder
	.set	nomacro
	
/* if byte found in quadword, must search a byte at a time
   to find actual position, otherwise continue checking
   quadwords at a time if the length remaining warrants */
   
	beql	$2,$0,$L32
	addu	$6,$6,-16
	.set	macro
	.set	reorder

	move	$4,$7
	move	$3,$0
	
/* loop through quadword to find actual location of byte in src */

$L15:
	lbu	$2,0($4)
	#nop
	.set	noreorder
	.set	nomacro
	bne	$2,$5,$L16
	addu	$3,$3,1
	.set	macro
	.set	reorder

$L31:
	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$4
	.set	macro
	.set	reorder

$L16:
	sltu	$2,$3,16
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L15
	addu	$4,$4,1
	.set	macro
	.set	reorder

	addu	$6,$6,-16

/* continue quadword loop if >= 16 bytes remain to be searched */

$L32:
	sltu	$2,$6,16
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L10
	addu	$7,$7,16
	.set	macro
	.set	reorder

	move	$4,$7
	
#else  /* __OPTIMIZE_SIZE__ */

	andi	$5,$5,0x00ff
	
#endif /* __OPTIMIZE_SIZE__ */

/* while bytes to search >= 0 and not found, search a byte at a time */

$L2:
	li	$2,4294901760			# 0xffff0000
	addu	$6,$6,-1
	ori	$2,$2,0xffff
	beq	$6,$2,$L20
	li	$3,4294901760			# 0xffff0000
	ori	$3,$3,0xffff
$L21:
	lbu	$2,0($4)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$5,$L31
	addu	$6,$6,-1
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$6,$3,$L21
	addu	$4,$4,1
	.set	macro
	.set	reorder

$L20:
	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$0
	.set	macro
	.set	reorder
	
#ifdef __OPTIMIZE_SIZE__
	
$L31:
	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$4
	.set	macro
	.set	reorder

#endif /* __OPTIMIZE_SIZE__ */
	
	.end	memchr
