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
