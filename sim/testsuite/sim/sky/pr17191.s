# Test the cop0 timer (cop0 regs 9 (count) and 11 (compare)).

	.include "t-macros.inc"

	.globl _start
	.ent _start
	.set noreorder
_start:

	# Patch the interrupt handler, setting it to an instruction
	# sequence that jumps to this programs interrupt code.
	la $4, handler
	la $5, +0xffffffffBFC00200 + 0x200
	lw $6, 0($4)  # j $9
	sw $6, 0($5)
	lw $6, 4($4)  # nop
	sw $6, 4($5)

	# establish two global pointer registers:	
	la $8, flag         # r8 contains the address of FLAG
	la $9, real_handler # r9 contains the address of the
			    # real handler

	# Set count/compare so the timer goes off in 256 cycles.
	# We give ourselves enough slack in the count to get set up properly.
	# We also assume the interrupt is masked at power on, but
	# that should be a given.
	li $4, 0
	mtc0 $4, $9		# Set count to 0.
	li $4, 256
	mtc0 $4, $11		# Set compare to 256.
	nop ; nop ; nop

	# enable interrupt delivery
	mfc0 $4, $12
	nop ; nop ; nop
	la $5, 0x18001
	or $4, $4, $5                # eie,ie,im7 = 1
	or $4, $4, 6 ; xor $4, $4, 6 # ensure error/exception level = 0
	mtc0 $4, $12
	nop ; nop ; nop

	# Loop for some small amount that is sufficiently
	# long that the timer will go off.
	li $5, 256
loop:
	addi $5, $5, -1
	bne $5, $0, loop
	nop

	# Was FLAG set?
	lw $4, 0($8)
	bne $4, $5, pass

# Fail.

	exit47

# Pass.
# write (1, str, 6)

pass:
	sw $0, 0($8)	
	li	$4, 1
	la	$5, str
	li	$6, 6
	la	$2,+0xffffffffbfc00504
	lw	$2, 0($2)
	jal	$2
	nop

	exit0

	# Instruction sequence that jumps to
	# the real handler (address in r9).
handler:
	j $9
	nop

	# local interrupt handler, set FLAG and return
real_handler:
	li $10, 1
	sw $10, 0($8) # flag = 1
	eret
	nop


	.end	_start

flag:
	.word 0
str:	.asciiz "pass\r\n"
