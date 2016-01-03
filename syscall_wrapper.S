#global variable for syscall warpper
.global syscall_wrapper



# This function is a syscall wrapper

syscall_wrapper:
	
	cli



	pushal
	pushfl
	#Check if the number of syscall is valid(1-10) or not
	cmpl $1, %eax
	jl invalid
	cmpl $10, %eax
	jg invalid

	pushl %edx
	pushl %ecx
	pushl %ebx



	decl %eax
	# Call to according syscall
	call *jumptable(, %eax, 4)
	movl %eax, temp_eax

	popl %ebx
	popl %ecx
	popl %edx

	popfl
	popal

	movl temp_eax, %eax

	sti
	iret


invalid:

	popfl
	popal

	sti
	# If invalid, return -1
	movl $-1, %eax
	iret

#jump table for functions
jumptable:
	.long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn
#temp var 
temp_eax:
	.long 0
