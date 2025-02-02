.section .data
 my_string: .asciz "Return value %d"
.section .text
.globl main
main:
	pushq %rbp
	movq %rsp, %rbp
	subq $16, %rsp
L1:
	movq $1, 0(%rsp)
L3:
	movq $12, 8(%rsp)
L4:
	movq 8(%rsp), %rax
	cmp $0, %rax
	jle L2
L5:
	movq 0(%rsp), %rax
	imulq 8(%rsp), %rax
	movq %rax, 0(%rsp)
L6:
	movq 8(%rsp), %rax
	subq $1, %rax
	movq %rax, 8(%rsp)
	jmp L4
L2:
	lea my_string(%rip), %rdi
	movq (%rsp), %rsi
	call printf
	movq %rbp, %rsp
	popq %rbp
	ret
