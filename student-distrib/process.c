#include "process.h"
#include "terminal.h"
#include "paging.h"

#define KERNEL_ADDR  0x800000
#define PDE_NUMBER 1024
#define PCB_BLOCK_SIZE 0x400  //pcb size<-----------------------subject to change
#define PROCESS_BLOCK_SIZE   0x2000 
#define STDIN_FD 0
#define STDOUT_FD 1

pcb_t* current_process;  // pointer to current process

static pcb_t pcb[MAX_PROCESS];

static uint8_t process_status[MAX_PROCESS];
static int32_t open_std(int32_t pid, int32_t fd);
/**
 	get_pcb_info
	Function get the pointer to specific pcb structure
	@param pid number
	@return the address of the pcb structure for a specific pid
*/
pcb_t* get_pcb_info(uint32_t pid)
{
	if(pid<MAX_PROCESS)
		return &pcb[pid];
	else
		return NULL;
} 
/**
 	init_process
	Function to initialize the process status
	@param pid number
	@return 0 if success, -1 if fails
*/
int32_t init_process()
{
	
	int i;
	//Initialize the process status
	for(i = 0; i < MAX_PROCESS; i++)
		{
			process_status[i] = 0;
			pcb[i].pid = -1;
		}
	current_process = NULL;

	return 0;
}
/**
 	new_process
	Function to create a new process
	@param command
	@return 0 if success, -1 if fails
*/
int32_t new_process(uint8_t* command)
{

	int32_t pid = find_availale_pid(); 
	int32_t i;
	//printf("the new process pid is %d\n",pid);
	if(pid == -1)//if open the 7th program,i should fail
		return -1;
	process_status[pid] = 1;

	pcb[pid].pid = pid;	//fill up the pcb

	if(current_process == NULL) {

		current_process = &pcb[pid];
		current_process->parent = NULL;
	}
	else {
		pcb[pid].parent = current_process;
		pcb[pid].tid = current_process->tid;
		current_process = &pcb[pid];
	}

	current_process->tid = current_tid;
	current_process->sche_esp = kernel_stack_address(pid);
	current_process->sche_ebp = kernel_stack_address(pid);
	current_process->prev_esp = 0;
	current_process->prev_ebp = 0;
	
	for(i=0;i<MAX_FILE_NUMBER;i++)
	{

		current_process->fd_array[i].flags.inuse = 0;
		current_process->fd_array[i].flags.is_directory = 0;
		current_process->fd_array[i].flags.can_read = 0;
		current_process->fd_array[i].flags.can_write = 0;
		current_process->fd_array[i].inode = 0;
		current_process->fd_array[i].file_pos = 0;
		current_process->fd_array[i].fops_table.open = NULL;
		current_process->fd_array[i].fops_table.close = NULL;
		current_process->fd_array[i].fops_table.read = NULL;
		current_process->fd_array[i].fops_table.write = NULL;
	}


	update_video_pt();//update videa page directory
	open_std(pid,STDIN_FD);  //open stdin
	open_std(pid,STDOUT_FD);  //open stdout
	
	// put current pid in terminal info
	return pid;
}
/**
 	open_std
	helper open the stdin and stdout ,and put the function pointer in pcb
 	input pid number
 	input fd file descripter
	@return 0 if success, -1 if fails
*/
static int32_t open_std(int32_t pid, int32_t fd)
{
	pcb[pid].fd_array[fd].fops_table.open = open_terminal;
	pcb[pid].fd_array[fd].fops_table.write = write_terminal;
	pcb[pid].fd_array[fd].fops_table.read = read_terminal;
	pcb[pid].fd_array[fd].fops_table.close = close_terminal;
	//assign read and write privilage
	if(fd == STDIN_FD)
	{
		pcb[pid].fd_array[fd].flags.can_read = 1;
		pcb[pid].fd_array[fd].flags.can_write = 0;
	}
	else if(fd == STDOUT_FD)
	{
		pcb[pid].fd_array[fd].flags.can_read = 0;
		pcb[pid].fd_array[fd].flags.can_write = 1;
	}
	return 0;
}

/**
 	end_process
	Function to end a process
	@param pid number
	@return 0 if success, -1 if fails
*/
int32_t end_process(uint32_t pid)
{
	int i;
	if(pid>= MAX_PROCESS)
		return -1;


	process_status[pid] = 0;
	pcb_t* empty_pcb = &(pcb[pid]);

	// remove current pid from terminal info
	// terminal_info[empty_pcb->tid].pid =  empty_pcb->parent->pid;  
	terminal_info[empty_pcb->tid].pid = empty_pcb->parent == NULL ? -1 : empty_pcb->parent->pid; 

	// terminal_info[current_process->tid].pid = parent_process->pid;
	//close all the files opened
	for(i=0;i<MAX_FILE_NUMBER;i++)
	{
		if(empty_pcb->fd_array[i].fops_table.close != NULL)
		empty_pcb->fd_array[i].fops_table.close(i);
	}
	// clear current pcb
	empty_pcb->pid = -1;
	empty_pcb->parent = NULL;
	empty_pcb->user_stack = NULL;
	empty_pcb->kernel_stack = NULL;
	for(i=0;i<MAX_FILE_NUMBER;i++)
	{

		empty_pcb->fd_array[i].flags.inuse = 0;
		empty_pcb->fd_array[i].flags.is_directory = 0;
		empty_pcb->fd_array[i].flags.can_read = 0;
		empty_pcb->fd_array[i].flags.can_write = 0;
		empty_pcb->fd_array[i].inode = 0;
		empty_pcb->fd_array[i].file_pos = 0;
		empty_pcb->fd_array[i].fops_table.open = NULL;
		empty_pcb->fd_array[i].fops_table.close = NULL;
		empty_pcb->fd_array[i].fops_table.read = NULL;
		empty_pcb->fd_array[i].fops_table.write = NULL;
	}
	empty_pcb->prev_esp = 0;
	empty_pcb->prev_ebp = 0;	
	empty_pcb->sche_esp = 0;
	empty_pcb->sche_ebp = 0;
	empty_pcb->tid = -1;

	return 0;
}

/**	
 	find_availale_pid
	Function to find an avaiable pid
	@param pid number
	@return pid(0-5) if success, -1 if fails
*/
int32_t find_availale_pid()
{
	int i;
	for(i=0;i<MAX_PROCESS;i++)
	{
		if(process_status[i] == 0)
			return i;
	}
	return -1;

}
/**
 	kernel_stack_address
	Function to return a process's kernel stack addr
	@param pid number
	@return 0 if success, -1 if fails
*/
int32_t kernel_stack_address(uint32_t pid)
{
	return KERNEL_ADDR-(1+pid)*PROCESS_BLOCK_SIZE;
}



