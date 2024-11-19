# Copyright (c) 1998 Cygnus Solutions
# Minimal interrupt handler - load two copies

	.section ".eit_v","ax"
	.global int_count
	.global int_cause
	.global int_handler

	.org 0x80	# offset 0x80 from base 0x180
	.ent int_handler
int_handler:
	# increment global int_count
	ld      $26,int_count
        addi    $26,$26,1
        sw      $26,int_count

	# store & clear CAUSE
	mfc0	$26,$13
	sw	$26,int_cause
	mtc0	$0,$13
		
	# return properly
	eret
	nop
	.end int_handler
