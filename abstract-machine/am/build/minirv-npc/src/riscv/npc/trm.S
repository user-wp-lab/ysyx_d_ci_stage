#include "/home/wp/Desktop/ysyx-workbench/abstract-machine/tools/minirv/inst-replace.h"
	.file	"trm.c"
	.option nopic
	.attribute arch, "rv32e1p9_zicsr2p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 4
	.text
	.section	.text.halt,"ax",@progbits
	.align	2
	.globl	halt
	.hidden	halt
	.type	halt, @function
halt:
#APP
# 22 "/home/wp/Desktop/ysyx-workbench/abstract-machine/am/src/riscv/npc/trm.c" 1
	mv a0, a0; ebreak
# 0 "" 2
#NO_APP
.L2:
	j	.L2
	.size	halt, .-halt
	.section	.text._trm_init,"ax",@progbits
	.align	2
	.globl	_trm_init
	.hidden	_trm_init
	.type	_trm_init, @function
_trm_init:
	addi	sp,sp,-12
	lla	a0,mainargs
	sw	ra,8(sp)
	call	main
#APP
# 22 "/home/wp/Desktop/ysyx-workbench/abstract-machine/am/src/riscv/npc/trm.c" 1
	mv a0, a0; ebreak
# 0 "" 2
#NO_APP
.L5:
	j	.L5
	.size	_trm_init, .-_trm_init
	.section	.rodata.mainargs,"a"
	.align	2
	.type	mainargs, @object
	.size	mainargs, 64
mainargs:
	.string	"t"
	.zero	62
	.hidden	heap
	.globl	heap
	.section	.sdata.heap,"aw"
	.align	2
	.type	heap, @object
	.size	heap, 8
heap:
	.word	_heap_start
	.word	_pmem_start+134217728
	.ident	"GCC: (Ubuntu 12.3.0-1ubuntu1~23.04) 12.3.0"
	.section	.note.GNU-stack,"",@progbits
