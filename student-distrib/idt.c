#include "idt.h"

#define PIT_IDX 0x20
#define KEYBOARD_IDX 0x21
//#define KEYBOARD_IDX_2 0x29
#define RTC_IDX 0x28

#define MOUSE_IDX 0x2c

#define SYSCALL_IDX 0x80


#define EXCEPTION_COUNT 20
/*
 * idt_initialize
 *   DESCRIPTION: The function to initilize the idt table
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Call SET_IDT_ENTRY to set all the idt
 */ 
void idt_initialize()
{
	/*Loop variables*/
	int i = 0;
	for (i = 0; i < EXCEPTION_COUNT; ++i)
	{
		idt[i].seg_selector = KERNEL_CS;
		idt[i].reserved4 = 0;
		idt[i].reserved3 = 0; 
		idt[i].reserved2 = 1;
		idt[i].reserved1 = 1;
		idt[i].size = 1;
		idt[i].reserved0 = 0;
		idt[i].dpl = 0;
		idt[i].present = 1;
	}	

	SET_IDT_ENTRY(idt[0], division_by_zero);  
	SET_IDT_ENTRY(idt[1], debug_exception);  
	SET_IDT_ENTRY(idt[2], non_maskable_int); 
	SET_IDT_ENTRY(idt[3], break_point); 	
	SET_IDT_ENTRY(idt[4], into_overflow);	
	SET_IDT_ENTRY(idt[5], out_of_bounds);	
	SET_IDT_ENTRY(idt[6], invalid_opcode);	
	SET_IDT_ENTRY(idt[7], device_not_available);	
	SET_IDT_ENTRY(idt[8], double_fault);	
	SET_IDT_ENTRY(idt[9], co_segment_overrun);	
	SET_IDT_ENTRY(idt[10], invalid_tss);
	SET_IDT_ENTRY(idt[11], segment_not_present);	
	SET_IDT_ENTRY(idt[12], stack_fault);	
	SET_IDT_ENTRY(idt[13], general_protection); 
	SET_IDT_ENTRY(idt[14], page_fault);

	SET_IDT_ENTRY(idt[16], x87_floating_point);
	SET_IDT_ENTRY(idt[17], alignment_check);
	SET_IDT_ENTRY(idt[18], machine_check);	
	SET_IDT_ENTRY(idt[19], SIMD_floating_point);	
	//SET_IDT_ENTRY(idt[0x20],pit_interrput);
	/*Scheduler part*/
	
	idt[PIT_IDX].seg_selector = KERNEL_CS;
	idt[PIT_IDX].reserved4 = 0;
	idt[PIT_IDX].reserved3 = 0;
	idt[PIT_IDX].reserved2 = 1;
	idt[PIT_IDX].reserved1 = 1;
	idt[PIT_IDX].size = 1;
	idt[PIT_IDX].reserved0 = 0;
	idt[PIT_IDX].dpl = 0;
	idt[PIT_IDX].present = 1;	
	SET_IDT_ENTRY(idt[PIT_IDX],sched_link);


	/*Keyboard part*/
	
	idt[KEYBOARD_IDX].seg_selector = KERNEL_CS;
	idt[KEYBOARD_IDX].reserved4 = 0;
	idt[KEYBOARD_IDX].reserved3 = 0;
	idt[KEYBOARD_IDX].reserved2 = 1;
	idt[KEYBOARD_IDX].reserved1 = 1;
	idt[KEYBOARD_IDX].size = 1;
	idt[KEYBOARD_IDX].reserved0 = 0;
	idt[KEYBOARD_IDX].dpl = 0;
	idt[KEYBOARD_IDX].present = 1;	
	SET_IDT_ENTRY(idt[KEYBOARD_IDX],keyboard_link);


	/*RTC part*/

	
	idt[RTC_IDX].seg_selector=KERNEL_CS;
	idt[RTC_IDX].reserved4 = 0;
	idt[RTC_IDX].reserved3 = 0;
	idt[RTC_IDX].reserved2 = 1;
	idt[RTC_IDX].reserved1 = 1;
	idt[RTC_IDX].size = 1;
	idt[RTC_IDX].reserved0 = 0;
	idt[RTC_IDX].dpl = 0;
	idt[RTC_IDX].present= 1 ;	
	SET_IDT_ENTRY(idt[RTC_IDX],rtc_link);

	/*Mouse part*/
	idt[MOUSE_IDX].seg_selector=KERNEL_CS;
	idt[MOUSE_IDX].reserved4 = 0;
	idt[MOUSE_IDX].reserved3 = 0;
	idt[MOUSE_IDX].reserved2 = 1;
	idt[MOUSE_IDX].reserved1 = 1;
	idt[MOUSE_IDX].size = 1;
	idt[MOUSE_IDX].reserved0 = 0;
	idt[MOUSE_IDX].dpl = 0;
	idt[MOUSE_IDX].present= 1 ;	
	SET_IDT_ENTRY(idt[MOUSE_IDX],mouse_link);

	/*System call part*/
	idt[SYSCALL_IDX].seg_selector=KERNEL_CS;
	idt[SYSCALL_IDX].reserved4 = 0;
	idt[SYSCALL_IDX].reserved3 = 0;
	idt[SYSCALL_IDX].reserved2 = 1;
	idt[SYSCALL_IDX].reserved1 = 1;
	idt[SYSCALL_IDX].size = 1;
	idt[SYSCALL_IDX].reserved0 = 0;
	idt[SYSCALL_IDX].dpl = 3; /*Specially for sys call*/
	idt[SYSCALL_IDX].present= 1 ;	
	SET_IDT_ENTRY(idt[SYSCALL_IDX],syscall_wrapper);


}


