#include "scheduler.h"
#include "terminal.h"
#include "rtc.h"

#define TOTAL_TERMINAL 3
#define FIRST_TERMIANL 0
#define PIT_IRQ 0

//local structures to store the terminal information
static int32_t current_sched_id = 0;
static int32_t sche_info[3];
static uint8_t terminal_count;
int    times;


/**
	init_sche
	Function to initialize the scheduler structure sche_info
	INPUTS:
	@param none
	OUTPUTS: none
	@return none
*/
void init_sche()
{
	int i;


	//sche_info for other terminal is 0
	for(i=0;i<TOTAL_TERMINAL;i++)
		sche_info[i] = 0;

	//the first terminal will run shell in kernel.c
	sche_info[FIRST_TERMIANL] = 1;
	terminal_count = 1;
}

/**
	scheduling
	Scheduling hander
	This function will be called each time PIT fires
	INPUTS:
	@param none
	OUTPUTS: none
	@return none
*/
void scheduling()
{

	times++;
	if (times%100==0)
	{
		set_time();
		times=0;
	}
	clear_statusbar();
	status_bar();


	//send eoi
	send_eoi(PIT_IRQ);
	cli();
	//temporary variable to store the current esp and ebp
	uint32_t esp,ebp;  
	//temporary variable that equals to find next scheduled ID
	uint32_t next_sche_id = current_sched_id;

	//Move current esp ebp value to the temporary variable
	asm volatile("movl %%esp, %0" : "=r" (esp));
	asm volatile("movl %%ebp, %0" : "=r" (ebp));
	
	//Find the next to_schedule ID according to the informatio in sche_info
	//If sche_info[i] == 1 then the process in terminal i can be scheduled
	if(terminal_count!=1)
	{
		while(1)
		{
			next_sche_id++;
			if(next_sche_id >= TOTAL_TERMINAL)	
				next_sche_id = 0;
			if(sche_info[next_sche_id] == 1)
				break;
		}
	}
	//The only case that the current_process is null is before the first shell running
	if(current_process !=NULL)
	{
		//store the esp ebp in the current process pcb
	    current_process->sche_esp = esp;
	    current_process->sche_ebp = ebp;
	

		if(terminal_count>1)
		{
			//find the pcb info for next scheduled process
			pcb_t* next_proc_pcb = get_pcb_info(terminal_info[next_sche_id].pid);
			
		    //switch the current process to the next process to be scheduled
			current_process = next_proc_pcb;  

			//restore the esp and ebp from the pcb of the next process
			esp = current_process->sche_esp;
		    ebp = current_process->sche_ebp;

		    //switch the page directory
			switch_pd(terminal_info[next_sche_id].pid);

			//change the esp0 and ss0
			tss.esp0 = kernel_stack_address(current_process->pid);
	        tss.ss0 = KERNEL_DS;

	        //move the esp and ebp value into the register
		    asm volatile("movl %0, %%esp" : : "b" (esp));
		    asm volatile("movl %0, %%ebp" : : "b" (ebp));
			
		}
	}
	//just in case for some magic reasons current process is null
	else
		{
			current_process = get_pcb_info(next_sche_id);
			switch_pd(next_sche_id);
		}

	//change current schedule id
	current_sched_id = next_sche_id;

}

/**
	add_termianl
	This function will be called each time the a shell is executed in a new terminal
	INPUTS:
	@param current_tid
	OUTPUTS: none
	@return none
*/
void add_termianl(uint8_t current_tid)                       
{
	//if it appears that terminal does not have a shell run yet
	if(sche_info[current_tid] != 1)
	{
		//update the sche_info
		sche_info[current_tid] = 1;
		terminal_count++;
		
		//current sched_id is the one before curr sche id so that the next schedued process is the shell itself
		current_sched_id = current_tid-1;
		if(current_sched_id<0)
			current_sched_id = TOTAL_TERMINAL-1;
	}
}
/**
	add_new_proc_sche
	This function will be called each time the new process (except shell in a new terminal) is executed
	INPUTS:
	@param current_tid
	OUTPUTS: none
	@return none
*/
void add_new_proc_sche(uint8_t current_tid)    
{
	current_sched_id = current_tid;
	while(1)
	{
		current_sched_id--;
		if(sche_info[current_sched_id] != 0)
			break;
		if(current_sched_id == 0)
			current_sched_id = TOTAL_TERMINAL;
	}
} 





