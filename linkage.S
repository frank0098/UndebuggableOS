#global variables for linkage function
.globl keyboard_link,rtc_link,sched_link,mouse_link


.align 4

# Function for keyboard linkage
keyboard_link:
	#Clear interrupt flags
	cli
	#push all the registers
	push %es
	push %ds
	pushal
	pushfl
	#Call the interrupt handler
	call keyboard_int_handler
	#Restore all the registers
	popfl
	popal
	popl %ds
	popl %es
	#Restore int flags
	sti
	#Return
	iret


# Function for rtc linkage
rtc_link:
	#Clear interrupt flags
	cli
	#push all the registers
	push %es
	push %ds
	pushal
	pushfl
	#Call the interrupt handler
	call rtc_int_handle
	#Restore all the registers
	popfl
	popal
	popl %ds
	popl %es
	#Restore int flags
	sti
	#Return
	iret


# Function for mouse linkage
mouse_link:
	#Clear interrupt flags
	cli
	#push all the registers
	push %es
	push %ds
	pushal
	pushfl
	#Call the interrupt handler
	call mouse_int_handle
	#Restore all the registers
	popfl
	popal
	popl %ds
	popl %es
	#Restore int flags
	sti
	#Return
	iret

# Function for scheduler linkage
sched_link:
	#Clear interrupt flags
	cli
	#push all the registers
	push %es
	push %ds
	pushal
	pushfl
	#Call the interrupt handler
	call scheduling
	#Restore all the registers
	popfl
	popal
	popl %ds
	popl %es
	#Restore int flags
	sti
	#Return
	iret


