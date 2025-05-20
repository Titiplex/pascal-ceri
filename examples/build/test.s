			# This code was produced by the CERI Compiler
	.data
	.align 8
EAIMsg:	.string "Array index out of bounds\n"
FormatStringInt:	.string "%llu\n"
FormatStringDouble:	.string "%f\n"
FormatStringChar:	.string "%c\n"
FormatStringString:	.string "%s\n"
TrueString:	.string "TRUE\n"
FalseString:	.string "FALSE\n"
a:	.space 256 # String
	.text		# The following lines contain the program
ErrorArrayIndex:
	leaq  EAIMsg(%rip), %rcx
	xor   %eax, %eax
	call  printf
	mov   $1, %edi # exit(1)
	call  exit
	.globl main	# The main function must be visible from outside
main:			# The main function body :
	movq %rsp, %rbp	# Save the position of the stack's top
	.section .rodata
String1:	.string "Hello World"
	.text
	leaq String1(%rip), %rax
	push %rax
	pop %rsi
	leaq a(%rip), %rdi
	mov $256, %rcx
	cld
	rep movsb
