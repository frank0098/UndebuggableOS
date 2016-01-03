#ifndef _PROCESS_H
#define _PROCESS_H

#include "x86_desc.h"
#include "file_sys.h"

#define MAX_FILE_NUMBER 8
#define MAX_PROCESS 6
#define MAX_ARG_LENGTH 128

typedef int32_t (*open_func)(const uint8_t* filename);
typedef int32_t (*close_func)(int32_t fd);
typedef int32_t (*write_func)(int32_t, const void*, int32_t);
typedef int32_t (*read_func)(int32_t, void*, int32_t);


/*Structure For fops table*/
typedef struct fops_table_t{
	open_func open;
	read_func read;
	write_func write;
	close_func close;
}fops_table_t;
/*Structure For fd */
typedef struct file_descriptor_flag{
	int8_t   inuse;
	int8_t   is_directory;
	int8_t   can_read;
	int8_t   can_write;
}flag_t;
/*Structure For fd*/
typedef struct file_descriptor{
	fops_table_t  fops_table;  /*  POINTERS TO FUNCTIONS*/
	uint32_t      inode;
	uint32_t       file_pos;
	flag_t         flags;

}fd_t;

/*Structure For pcb*/
typedef struct process_control_block{

	uint32_t pid;
	struct process_control_block* parent; 
	register_t registers;//all saved registers
	void* user_stack;
	void* kernel_stack;
    fd_t    fd_array[MAX_FILE_NUMBER]; // file descriptor; 
    uint8_t args[MAX_ARG_LENGTH];  // args for the system call
    uint32_t prev_esp;
    uint32_t prev_ebp;
    uint8_t tid;
    uint32_t sche_esp;/*Kernel stack address*/
    uint32_t sche_ebp;
}pcb_t;


// pointer to current process
pcb_t* current_process;  


pcb_t* get_pcb_info(uint32_t pid);	
int32_t init_process();
int32_t new_process(uint8_t* command);
int32_t end_process(uint32_t pid);
int32_t find_availale_pid();
int32_t kernel_stack_address(uint32_t pid);
/*int32_t kernel_pcb_address(uint32_t pid);*/

#endif /* _PROCESS_H */
