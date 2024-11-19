	.macro start
	.text
	.globl _start
	.set noreorder
_start:
	nop
	nop
	.endm


	.macro exit0
	li	$4, 0
	break 1023
	nop
	.endm


	.macro exit47
	li	$4, 47
	break 1023
	nop
	.endm



	.macro load reg hi lo
	.data
	.align 3
1:	.quad \hi
2:	.quad \lo
	.text
	ld \reg, 1b ($0)
	pcpyld	\reg, \reg, $0
	ld \reg, 2b ($0)
	.endm


	.macro check10 hi lo
	.data
	.align 3	
1:	.quad \hi
2:	.quad \lo	
	.text
	pcpyud	$11, $10, $0
	ld	$5, 1b ($0)
	beq	$11, $5, 3f
	nop
	li	$4, 47
	break	1023
	nop
3:	nop
	ld	$5, 2b ($0)
	beq	$10, $5, 3f
	nop
	li	$4, 47
	break	1023
	nop
3:	nop
	.endm

	.macro checkHI hi lo
	.data
	.align 3
1:	.quad \hi
2:	.quad \lo
	.text
	mfhi1	$11
	ld	$5, 1b ($0)
	beq	$11, $5, 3f
	nop
	li	$4, 47
	break	1023
	nop
3:	nop
	mfhi	$11
	ld	$5, 2b ($0)
	beq	$11, $5, 3f
	nop
	li	$4, 47
	break	1023
	nop
3:	nop
	.endm


	.macro checkLO hi lo
	.data
	.align 3
1:	.quad \hi
2:	.quad \lo
	.text
	mflo1	$11
	ld	$5, 1b ($0)
	beq	$11, $5, 3f
	nop
	li	$4, 47
	break	1023
	nop
3:	nop
	mflo	$11
	ld	$5, 2b ($0)
	beq	$11, $5, 3f
	nop
	li	$4, 47
	break	1023
	nop
3:	nop
	.endm


	.macro loadfp reg val
	.data
	.align 2
1:	.float \val
	.text
	lwc1 \reg, 1b ($0)
	.endm


	.macro loadfpx reg val
	.data
	.align 2
1:	.long \val
	.text
	lwc1 \reg, 1b ($0)
	.endm


	.macro loadfpmax reg
	.data
	.align 2
1:	.word 0x7fffffff
	.text
	lwc1 \reg, 1b ($0)
	.endm


	.macro clearfcsr val
	ctc1 $0, $31
	nop
	nop
	.endm


	.macro checkfcsr psw
	.data
	.align 2
1:	.word \psw | 1
	.text
	cfc1 $10, $31
	nop
	nop
	nop
	lw $11, 1b ($0)
	beq $10, $11, 2f
	nop
	li	$4, 47
	break	1023
	nop
2:
	.endm


	.macro checkfp psw reg val
	.data
	.align 2
1:	.word \psw | 1
	.text
	cfc1 $10, $31
	nop
	nop
	nop
	lw $11, 1b ($0)
	beq $10, $11, 2f
	nop
	li	$4, 47
	break	1023
	nop
2:	
	.data
	.align 2
1:	.float \val
	.text
	lwc1 $f10, 1b ($0)
	c.eq.s \reg, $f10
	nop
	nop
	nop
	bc1t 2f
	nop
	li	$4, 47
	break	1023
	nop
2:
	.endm


	.macro checkfpx nr psw reg val
	.data
	.align 2
1:	.word \psw | 1
	.text
	cfc1 $10, $31
	nop
	nop
	nop
	lw $11, 1b ($0)
	beq $10, $11, 2f
	nop
	li	$4, \nr
	break	1023
	nop
2:	
	.data
	.align 2
1:	.long \val
	.text
	lwc1 $f10, 1b ($0)
	c.eq.s \reg, $f10
	nop
	nop
	nop
	bc1t 2f
	nop
	li	$4, \nr
	break	1023
	nop
2:
	.endm


	.macro checkfpmax psw reg
	.data
	.align 2
1:	.word \psw | 1
	.text
	cfc1 $10, $31
	nop
	nop
	nop
	lw $11, 1b ($0)
	beq $10, $11, 2f
	nop
	li	$4, 47
	break	1023
	nop
2:	
	.data
	.align 2
1:	.word 0x7fffffff
	.text
	lwc1 $f10, 1b ($0)
	c.eq.s \reg, $f10
	nop
	nop
	nop
	bc1t 2f
	nop
	li	$4, 47
	break	1023
	nop
2:
	.endm


	.macro loadacc val
	.data
	.align 2
1:	.float \val
	.text
	lwc1 $f10, 1b ($0)
	suba.s $f10, $f0
	.endm


	.macro loadaccx val
	.data
	.align 2
1:	.long \val
	.text
	lwc1 $f10, 1b ($0)
	suba.s $f10, $f0
	.endm


	.macro checkacc psw val
	.data
	.align 2
1:	.word \psw | 1
	.text
	cfc1 $10, $31
	nop
	nop
	nop
	lw $11, 1b ($0)
	beq $10, $11, 2f
	nop
	li	$4, 47
	break	1023
	nop
2:	
	.data
	.align 2
1:	.float \val
	.text
	lwc1 $f10, 1b ($0)
	msub.s $f11, $f0, $f0
	c.eq.s $f11, $f10
	nop
	nop
	nop
	bc1t 2f
	nop
	li	$4, 47
	break	1023
	nop
2:
	.endm


	.macro checkaccx nr psw val
	.data
	.align 2
1:	.word \psw | 1
	.text
	cfc1 $10, $31
	nop
	nop
	nop
	lw $11, 1b ($0)
	beq $10, $11, 2f
	nop
	li	$4, \nr
	break	1023
	nop
2:	
	.data
	.align 2
1:	.word \val
	.text
	lwc1 $f10, 1b ($0)
	msub.s $f11, $f0, $f0
	c.eq.s $f11, $f10
	nop
	nop
	nop
	bc1t 2f
	nop
	li	$4, \nr
	break	1023
	nop
2:
	.endm


	.macro checkaccmax psw
	.data
	.align 2
1:	.word \psw | 1
	.text
	cfc1 $10, $31
	nop
	nop
	nop
	lw $11, 1b ($0)
	beq $10, $11, 2f
	nop
	li	$4, 47
	break	1023
	nop
2:	
	.data
	.align 2
1:	.long 0x7fffffff
	.text
	lwc1 $f10, 1b ($0)
	msub.s $f11, $f0, $f0
	c.eq.s $f11, $f10
	nop
	nop
	nop
	bc1t 2f
	nop
	li	$4, 47
	break	1023
	nop
2:
	.endm


	FCSR_C  = (0x00800000)
	FCSR_I  = (0x20000 | 0x40)
	FCSR_D  = (0x10000 | 0x20)
	FCSR_O  = (0x08000 | 0x10)
	FCSR_U  = (0x04000 | 0x08)
