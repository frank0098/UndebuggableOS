#include "syscall.h"
#include "file_sys.h"
#include "process.h"
#include "terminal.h"
#include "paging.h"
#include "scheduler.h"
#include "rtc.h"

#define USER_EFLAGS 0x3200
#define MAX_FILE_FD 8
#define MAX_FILE_NAME 32
#define MAX_FILE 63
#define STDIN_FD 0
#define STDOUT_FD 1
#define USER_VIRTUAL_ADDRESS_LOW 0x8000000
#define USER_VIRTUAL_ADDRESS_HIGH 0x84000000
#define FILELENGTHINBYTE 4
#define FILE_BUFFER 0x100000

//magic number for entry point
#define exe0 0x7f
#define exe1 0x45
#define exe2 0x4C
#define exe3 0x46
#define entry_point1 24 
#define entry_point2 25
#define entry_point3 26
#define entry_point4 27

static int32_t valid_fd(int32_t fd);

void parse(const uint8_t* command_with_args, uint8_t* command, uint8_t* argument);

/**
 * halt system call
 *
 * @param  system status
 * @return 
 */
int32_t halt(uint8_t status) {
    /*printf("halting process %d\n",current_process->pid );*/
    uint32_t retval = (uint32_t)status;
    pcb_t* parent_process = current_process->parent;
    uint32_t prev_esp = current_process->prev_esp;
    uint32_t prev_ebp = current_process->prev_ebp;
    
    
    //cleanup pcb
    end_process(current_process->pid);
    
    //decrease the program counts
    program_count--;

    //if the parent process is not null
    if (parent_process != NULL) {       
        //recover parent 
        current_process = parent_process;

        // restore previous paging
        switch_pd(current_process->pid);

        // restore tss
        tss.esp0 = kernel_stack_address(current_process->pid);
        tss.ss0 = KERNEL_DS;
        
    }
     else {
    	end_process(current_process->pid);
        current_process = NULL;
        execute((const uint8_t*)"shell");
    }

    // return to execute with return value
    asm volatile("              \
        movl    %0, %%ecx;      \
        movl    %1, %%edx;      \
        movl    %2, %%eax;      \
                                \
        movl    %%ecx, %%esp;   \
        movl    %%edx, %%ebp;   \
                                \
        jmp     halt_return;    "
     : 
     : "r" (prev_esp), "r"(prev_ebp), "r" (retval)
     : "eax", "ecx", "edx", "memory");

    // should never reach here
    return 0;
}

/**
 * execute system call
 *
 * @param  user command
 * @return 
 */
int32_t execute(const uint8_t* command)
{
    //save the user prog ebp esp
    
    // if(current_process)
    // {
    //     asm volatile("movl %%esp, %0" : "=r" (current_process->prev_esp));
    //     asm volatile("movl %%ebp, %0" : "=r" (current_process->prev_ebp));
    // }
    uint8_t buf[FILELENGTHINBYTE];
    uint8_t command_without_args[MAX_ARG_LENGTH] = {'\0'};
    uint8_t args[MAX_ARG_LENGTH] = {'\0'};
    uint8_t* user_program_start = (uint8_t*)USER_PROGRAM_START;
    int32_t pid;
  	parse(command, command_without_args, args);
  
    // check file existance
    if (file_loader(command_without_args, buf, FILELENGTHINBYTE) != 0) {
        return -1;
    }
    /*The first 4 bytes of the file represent a “magic number” that identifies the file as an executable. These
	bytes are, respectively, 0: 0x7f; 1: 0x45; 2: 0x4c; 3: 0x46. If the magic number is not present, the execute system
	call should fail.*/
    if ((buf[0] != exe0) && (buf[1] != exe1) && (buf[2] != exe2) && (buf[0] != exe3)) {
        return -1;
    }

    // apply a new pid
    pid = new_process(NULL);
    if (pid < 0) {
        return -1;
    }
    // switch page directory
    if (switch_pd(pid) != 0) {
        return -1;
    }
    // load program image
    if (file_loader(command_without_args, user_program_start, FILE_BUFFER) != 0) {
        return -1;
    }
    /*Copy the args into pcb  */
    strncpy( (int8_t*)current_process->args, (int8_t*)args,MAX_ARG_LENGTH);

    /* increase program counts*/
    program_count++;

    /*Save current esp ebp inside pcb*/
    asm volatile("movl %%esp, %0" : "=r" (current_process->prev_esp));
    asm volatile("movl %%ebp, %0" : "=r" (current_process->prev_ebp));
    
    //update the kernel 
    tss.esp0 = kernel_stack_address(pid);
    tss.ss0 = KERNEL_DS;

    /*Keep track of the the virtual address of the first instruction that should be executed. 
    This information is stored as a 4-byte unsigned integer in bytes 24-27 of the executable*/
    
    uint32_t entry_point = (user_program_start[entry_point1]) | (user_program_start[entry_point2] << 8) | (user_program_start[entry_point3] << 16) | (user_program_start[entry_point4] << 24);

    //update the terminal_info_t
    if (terminal_info[current_tid].pid < 0) {
            add_termianl(current_tid);
        }

    terminal_info[current_tid].pid = pid;

    //Increase the program counter
    //switch_to_user_mode();
    
    /*Move USER_DS to DS*/
    /*Push USER_DS*/
    /*Push ESP*/
    /*Push Flags*/
    /*Push USER_CS*/
    /*Push program entry point(%eip)*/
    asm volatile("              \
        cli;                    \
                                \
        movw    $0x002B, %%ax;  \
        movw    %%ax, %%ds;     \
                                \
        pushl   $0x002B;        \
        pushl   $0x083FFFFC;    \
        pushl   $0x3200;        \
        pushl   $0x0023;        \
        pushl   %0;             \
                                \
        iret;                   \
                                \
halt_return:                    \
        leave;                  \
        ret;                    "
    :
    : "r" (entry_point)
    : "eax", "memory");

    // should never reach here
    return 0;
}
/**
 * read system call
 *
 * @param  file descriptor
 * @param  user buffer
 * @param  bytes to read 
 * @return a file descriptor integer for use in future system calls
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes)
{

    return (fd < 0 || fd >= MAX_FILE_FD || current_process->fd_array[fd].fops_table.read ==NULL ||  current_process->fd_array[fd].flags.can_read == 0 )?-1:current_process->fd_array[fd].fops_table.read(fd,buf,nbytes);

}
/**
 * write system call
 *
 * @param  file descriptor
 * @param  user buffer
 * @param  bytes to write 
 * @return 
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes)
{
    return (fd < 0 || fd >= MAX_FILE_FD || current_process->fd_array[fd].fops_table.write ==NULL || current_process->fd_array[fd].flags.can_write == 0 )?-1:current_process->fd_array[fd].fops_table.write(fd,buf,nbytes);
}
/**
 * open system call
 *
 * @param  filename
 * @return 
 */

int32_t open(const uint8_t* filename)
{
 /*   fops_table_t file_fops_table;
printf("oepn file file name is %s\n",filename );*/
     int fd;
    int empty_flag=0;
    dentry_t mydir;

    

    for(fd=2;fd<MAX_FILE_FD;fd++)                                            /*// check for free file_descriptor*/
    {
        if(current_process->fd_array[fd].flags.inuse==0)
        {
            empty_flag=1;
            break;
        }
    }

    fd_t* cur_file_fd = &current_process->fd_array[fd]; 
    cur_file_fd->inode = MAX_FILE+1;

     //UNCOMMENT IT FOR ECE391 CP5
    // if(cur_file_fd == NULL)
    //     return -1;

   
    int ret = read_dentry_by_name (filename, &mydir);


    // if( ret != 0 ) return -1;              
    if(empty_flag==0) 
        {
           /* printf("cannot open\n");*/
            return -1;
        }


    cur_file_fd->flags.inuse=1;              //set flag in use

    cur_file_fd->flags.can_read = 1;        //file is readable

    cur_file_fd->flags.can_write = 1;       //file is writable
                                             
    cur_file_fd->file_pos = 0;
    
    cur_file_fd->inode = mydir.inode_number; 

    if(mydir.file_type == DENTRY_DIRECTORY)
        cur_file_fd->flags.is_directory = 1;
    else
        cur_file_fd->flags.is_directory = 0;

    /* printf("the fd is %d the inode number is %d\n",fd,cur_file_fd->inode );
 
     printf("                                          open fd %d\n",fd );*/
    
    if(ret==-1)
    {
        cur_file_fd->fops_table.open = open_file;
        cur_file_fd->fops_table.read = read_file;
        cur_file_fd->fops_table.write = write_file;
        cur_file_fd->fops_table.close = close_file;
    }

    if(mydir.file_type == DENTRY_FILE || mydir.file_type == DENTRY_DIRECTORY)
    {
        cur_file_fd->fops_table.open = open_file;
        cur_file_fd->fops_table.read = read_file;
        cur_file_fd->fops_table.write = write_file;
        cur_file_fd->fops_table.close = close_file;
        if(ret!=-1)
        open_file(filename);
    }
    
    else if (mydir.file_type == DENTRY_RTC)
    {

        cur_file_fd->fops_table.open = rtc_open;
        cur_file_fd->fops_table.read = rtc_read;
        cur_file_fd->fops_table.write = rtc_write;
        cur_file_fd->fops_table.close = rtc_close;
        if(ret!=-1)
        rtc_open(filename);
    }

    return fd;
}

/**
 * close system call
 * cannot close stdin and stdout
 * @param  file descriptor
 * @return 0 on success, 1 on fail
 */

int32_t close(int32_t fd)
{
    
    if( valid_fd(fd) == 0  && fd!=STDIN_FD && fd!=STDOUT_FD  ) //if fd is valid and not equals 0 and 1 
        {
            // printf("               ...closingvalid fd %d\n",fd);
            fd_t* cur_file_fd = &(current_process->fd_array[fd]);
            if(cur_file_fd == NULL)
                return -1;

            cur_file_fd->fops_table.close(fd);//this casue
                                              //s bug
            cur_file_fd->flags.inuse = 0;
            cur_file_fd->flags.can_read = 0;
            cur_file_fd->flags.can_write = 0; 
            cur_file_fd->fops_table.open = NULL;
            cur_file_fd->fops_table.close = NULL;
            cur_file_fd->fops_table.read = NULL;
            cur_file_fd->fops_table.write = NULL;
            cur_file_fd->inode = NULL;
            cur_file_fd->file_pos = 0;
            return 0;
        }
    else
    {
   /*printf("                closing...invalid fd %d\n", fd);*/
    return -1;
    }
}
/**
 * getargs system call
 *
 * @param  user buffer 
 * @param  bytes to read
 * @return -1
 */

int32_t getargs(uint8_t* buf, int32_t nbytes)
{
    if (strlen((int8_t*)current_process->args) == 0) {
        return -1;
    }
    strncpy((int8_t*)buf, (int8_t*)current_process->args, nbytes);

    return 0;
}

/**
 * vidmap system call
 *
 * @param 
 * @return -1
 */
int32_t vidmap(uint8_t** screen_start)
{
    //printf("the location is %d\n",screen_start );
    if((uint32_t)screen_start >= USER_VIRTUAL_ADDRESS_LOW && (uint32_t)screen_start < USER_VIRTUAL_ADDRESS_HIGH)
    {
        *screen_start = (uint8_t*)VIRTUAL_VIDEO_MEM_BASE_ADDR;
        return 0;
    }
    //
    //printf("therefore i failed\n");
    return -1;
}
/**
 * sethandler system call
 *
 * @param 
 * @return -1
 */
int32_t set_handler(int32_t signum, void* handler_address)
{
    return -1;
}
/**
 * sigreturn system call
 * @return -1
 */
int32_t sigreturn(void)
{
    return -1;
}

/**
 * valid_fd
 *    helper function to judge if a fd passed in is valid
 * @param  system status
 * @return 0 if valid, -1 if invalid
 */
static int32_t valid_fd(int32_t fd)
{
    fd_t* cur_file_fd = &(current_process->fd_array[fd]);
    if(cur_file_fd->flags.inuse == 0 || fd < 0 || fd >=MAX_FILE_FD )  //if close a invalid file
        return -1;
    else 
        return 0;

}



/*Reference: http://www.jamesmolloy.co.uk/tutorial_html/10.-User%20Mode.html*/
/*The IRET instruction expects, when executed, the stack to have the following contents (starting from the stack pointer - the lowermost address upwards):
Stack prior to IRET.
The instruction to continue execution at - the value of EIP.
The code segment selector to change to.
The value of the EFLAGS register to load.
The stack pointer to load.
The stack segment selector to change to.*/

/*void switch_to_user_mode()
{
   // Set up a stack structure for switching to user mode.///
   asm volatile("          \
     cli;                 \
     mov $0x23, %ax;     \
     mov %ax, %ds;         \
     mov %ax, %es;         \
     mov %ax, %fs;         \
     mov %ax, %gs;         \
                           \
     movl %esp, %eax;     \
     pushl $0x23;         \
     pushl %eax;         \
     pushfl;            \
     pushl $0x1B;         \
     push $1f;             \
     iret;                 \
   1:                     \
     ");
}*/

/**
 * Function to parse the input arguments
 * @return -1
 */
void parse(const uint8_t* command_with_args, uint8_t* command, uint8_t* argument)
{
        int i=0;
        int j=0;
/*        int press_enter=0;
        int press_space=0;*/

        //Go through all the arguments
        while (command_with_args[i] != ' ' && command_with_args[i] != '\0') { //check  space
            command[i] = command_with_args[i];
            i++;
        }

        command[i] = '\0';
        i++;

        while(command_with_args[i] != '\0') {  //check enter
            argument[j] = command_with_args[i];
            i++;
            j++;
        }

        argument[j] = '\0';
/*            if(command[i] == ' ')       //check  space
            {
                press_space = 1;
            }
            if(command[i] == '\n')      //check enter
            {
                press_enter = 1;
            }
            if(press_enter == 1)
            {   
                argument[i] = '\0'; //end
                break;
            }

            if(press_space ==1)            //write arg buffer
            {
                argument[j] = command[i+1];
                j++;
                i++;
            }
            else
            {
                i++;
            }

        }*/
        return ; 
}  


