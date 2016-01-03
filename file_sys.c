#include "file_sys.h"

//4KB per block
#define BLOCKSIZE 4096  
//1023 per inode, 4kb per data block
#define MAX_FILE_SIZE 1023*4  
//for testing
#define MAX_BUFFER_SIZE 40000
#define MAX_WRITE_BUFFER 8192
#define MAX_PROCESS 6


//local variable to store the file system information
static boot_block_t* boot_block_addr;
static uint32_t inode_number;
static uint32_t data_block_number;
static uint32_t inode_start_addr;
static uint32_t data_block_start_addr;
static uint32_t multiboot_addr;

//variable for file system write
static uint8_t available_inodes[MAX_FILE];
static uint8_t available_datablock[MAX_DATA_BLOCK];
static void init_block_info();
static int32_t write_helper(int32_t fd, const void* buf, int32_t nbytes);
static char filename_buffer[MAX_PROCESS][MAX_FILE_NAME_LENGTH];



/**
	init_file_sys
	DESCRIPTION: Function to initialize the file system
	@param INPUTS: pointer for the location of the file system
	OUTPUTS: none
	@return  none
*/
void init_file_sys(uint32_t input)
{
	cli();
	int i;
	//boot block addr is passed in by system
	boot_block_addr = (boot_block_t*) input;
	multiboot_addr = input;

	//inode and data block number is read from boot block
	inode_number = boot_block_addr->inodes_N;
	data_block_number = boot_block_addr->data_blocks_D;

	//inode start addr and data block start addr is get from the inode and data block number
	inode_start_addr = input+BLOCKSIZE;
	data_block_start_addr =  inode_start_addr + inode_number*BLOCKSIZE;

	if(sync_with_hard_disk() == 1)
	{
		// copy_data_to_hard_disk((char*) (input),0);
		// copy_data_from_hard_disk((char*) (input),0);

		for(i=0;i<inode_number+data_block_number+1;i++)
		{
			copy_data_to_hard_disk((char*) (input+i*BLOCKSIZE),i);
		}
		for(i=0;i<inode_number+data_block_number+1;i++)
		{
			copy_data_from_hard_disk((char*) (input+i*BLOCKSIZE),i);
		}
	}
	else
	{
		
		for(i=0;i<inode_number+data_block_number+1;i++)
		{
			copy_data_from_hard_disk((char*) (input+i*BLOCKSIZE),i);
		}
	}

	init_block_info();//EXTRA CREDIT
}


/**
	read_file
	Function to read a file
	INPUTS:
	@param file descriptor
	@param buffer for reading
	@param bytes to read
	OUTPUTS: none
	@return 0 if success, -1 if fails
*/

int32_t read_file (int32_t fd, void* buf, int32_t nbytes)
{

	//termporary variable for read bytes
	int32_t bytes_read;

	//find the curr fd struct
	fd_t* curr_fd = &(current_process->fd_array[fd]);

	//if not directory, read the data and updating the file_pos
	if(curr_fd->flags.is_directory!=1)  
		 {
		 	bytes_read = read_data(curr_fd->inode, curr_fd->file_pos, buf, nbytes);
		 	curr_fd->file_pos += bytes_read;
		 	return bytes_read;
		 }
	//if to read is directory, copy the file name one by one , update the file_pos 1 a time
	//when it reaches a point that the file name is null , the reading is finished 
	else 
	{
		if(strlen((int8_t*)(boot_block_addr->dir_entries[curr_fd->file_pos].filename))!=0)
		{
			curr_fd->file_pos ++;
			strncpy(buf,(void*)(boot_block_addr->dir_entries[curr_fd->file_pos].filename),
				strlen((char*)boot_block_addr->dir_entries[curr_fd->file_pos].filename));
			return strlen((char*)boot_block_addr->dir_entries[curr_fd->file_pos].filename);
		}
		else 
			return 0;
		
	}


}

/**
	write_file
	Function to write a file
	INPUTS:
	@param file descriptor
	@param buffer for reading
	@param bytes to WRITE  ---MUST BE NO GREATER THAN 4096
	OUTPUTS: none
	@return This function should alwasy fail because this system is read only
*/
int32_t write_file (int32_t fd, const void* buf, int32_t nbytes)
{
	if(nbytes>BLOCKSIZE)
	return -1;
	return write_helper(fd,buf,nbytes);
}

/**
	Function to open a file
	@param file name
	@return  0 if success, -1 if fails
*/
int32_t open_file (const uint8_t* filename)
{
	dentry_t mydir;
	return read_dentry_by_name (filename, &mydir);
}
/**
	Function to close a file
	@param file name
	@return 0 if success, -1 if fails
*/
int32_t close_file (int32_t fd)
{
	return 0;
}


/**
	Function to read directory entry by name
	@param file name
	@param pass back a dentry
	@return 0 if success, -1 if fails
*/

int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
{
	int i;
	cli();
	if(current_process!=NULL)
	{
		for(i=0;i<MAX_FILE_NAME_LENGTH;i++)
		filename_buffer[current_process->pid][i] = ' ';
		strncpy(filename_buffer[current_process->pid], (int8_t*) fname,MAX_FILE_NAME_LENGTH);//the max file name is 32
		// printf("			file name is %s currprocess pid    %d\n", fname,current_process->pid);
		
	}

	//if fname is empty,this function fails
	if (fname == NULL)	
	{
		dentry = NULL;
		return -1;
	}
	else
	{
		//traverse the file_directory_array  MAX FILE IS 63
		for(i = 0; i < MAX_FILE_SIZE; i++)  
		{
			//if the the filename and input is the same, and the lengths of both are the same, this function successfully return the dentry for that filename
			int filename_length = strlen((int8_t*)((boot_block_addr->dir_entries[i]).filename)); 
			int input_length = strlen((int8_t*)fname);
			if(filename_length == input_length && strncmp((int8_t*)fname,(int8_t*)(boot_block_addr->dir_entries[i]).filename,filename_length) == 0 && filename_length>0)	//if filename is found exactly the same
			{
				//copy the data the pointer passed in 
				dentry->file_type = (boot_block_addr->dir_entries[i]).file_type;
				dentry->inode_number = (boot_block_addr->dir_entries[i]).inode_number;  
				return 0;
			}
		}

		//if does not find acoording to given file name, this function fails
		dentry = NULL;
		return -1;
	}

}

	
/**
	Function to read directory entry by index
	@param index number
	@param dentry is the directory entry pass back
	@return 0 if success, -1 if fails
*/
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry)
{
	//if requested number is greater or equal to 63, this functon should fail
	if (index >= MAX_FILE)	
	{
		dentry = NULL;
		return -1;
	}
	//else, just return the dentry according to the given index
	else
	{
		//copy the data the pointer passed in 
		dentry->file_type = (boot_block_addr->dir_entries[index]).file_type;
		dentry->inode_number = (boot_block_addr->dir_entries[index]).inode_number;
		return 0;
	}

}

/**
	Function to read data
	@param inode number
	@param memory offset
	@param buffer to pass back the data
	@param the length expect to read
	@return 0 if success, -1 if fails
*/

int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
	cli();
	uint32_t byte_to_read;
	uint32_t byte_already_read;
	uint32_t file_length_in_byte;
	uint32_t cur_idx;
	uint32_t cur_data_block;
	uint32_t position_in_data_block;

	//if the inode is greater than the inode given in the super block, this call should fail
	if(inode >= inode_number) 
			return -1;

	//find the inode addr for the given inode idx, and the info needed
	inode_t* node_to_find;
	node_to_find = (inode_t*) (inode_start_addr+inode*BLOCKSIZE); 
	file_length_in_byte = node_to_find->inode_length;

	//if invalid offset, nothing is read
	if(offset >= file_length_in_byte) 
	 	return 0;
	

	byte_to_read = length;
	if(node_to_find->inode_length <= offset+length)
	 	byte_to_read = node_to_find->inode_length - offset;

	 //get the data block index according to the offset info
	 cur_idx = offset/BLOCKSIZE;  
	 //printf("the cur index is %d\n",cur_idx);
	 if(offset<BLOCKSIZE)
	 	cur_idx = 0;

	 //already read 0 byte
	 byte_already_read = 0; 	

	 //the data block corresponding to the cur idx
	 cur_data_block = node_to_find->inode_data[cur_idx];  
	 
	  //the location of data in the data block
	 position_in_data_block = offset%BLOCKSIZE;			

	 //When there are more bytes to read
	 while(byte_to_read>byte_already_read)		
	 {
	 	//for debug
	 	//this should not exceed file length , return -2
	 	if(byte_already_read >= file_length_in_byte) 	
	 	{
	 		printf("reading byte exceed the file size\n");
	 		return -2;
	 	}
	 	//should not read data block out of range, return -1
	 	if(cur_data_block >= data_block_number)  
	 		return -1;

	 	//copy the data from data block to the buffer
	 	memcpy(buf+byte_already_read,
	 		(uint8_t*) (data_block_start_addr+cur_data_block*BLOCKSIZE+position_in_data_block),sizeof(int8_t));	
	 	
	 	//increase the offset in the data block
	 	position_in_data_block++;	
	 	//increase the byte already read	
	 	byte_already_read++;			

	 	//already finished reading one data block, move to the next
	 	if(position_in_data_block == BLOCKSIZE)		
	 	{
	 		//go to the next data block
	 		cur_idx++;			
	 		//find the data block according to the index		
	 		cur_data_block = node_to_find->inode_data[cur_idx];		

	 		//should read file starting from offset 0
	 		position_in_data_block = 0;		
	 		
	 		//for debug
	 		// if exceed the max data block number, bad thing happen, return -2
	 		if(cur_idx > MAX_DATA_BLOCK)		
	 			{
	 				//FOR DEBUG
	 				printf("accessing invalid inode data number\n");
	 				return -2;
	 			}
	 	}

	 }
	 return byte_already_read;	//return the bytes read in total
}

/**
	Function FILE LOADER
	Fill the file into the physical memory
	@return  none
*/

int32_t file_loader(const uint8_t* filename, uint8_t* buf, uint32_t nbytes)
{
	dentry_t  mydir; 
	
	if(filename == NULL) return -1;
	//if read dentry by name fails, file loader fails
	if(read_dentry_by_name(filename,&mydir) !=0)                     return -1;
	//if read data fails, file loader fails
	if(read_data(mydir.inode_number,0,buf,nbytes) < 0)              return -1;
	//otherwise this success
	return 0;
}


/**
	Function for extra credit
	To find out the available inode blocks and data blocks
	@return  none
*/
static void init_block_info()
{
	int i,j;
	
	for(i =0; i < MAX_DATA_BLOCK; i++)
		available_datablock[i] = 0;

	//traverse the dir and mark 
	for(i = 0; i<MAX_FILE;i++)
	{
		if(strlen((int8_t*)boot_block_addr->dir_entries[i].filename)) //if the file name is not 0
			{
				uint32_t inode = boot_block_addr->dir_entries[i].inode_number;
				// printf("the inode is %d\n",inode );
				uint32_t file_length_in_byte;
				uint32_t data_block_count;
				available_inodes[inode] = 1;

				inode_t* node_to_find;
				node_to_find = (inode_t*) (inode_start_addr+inode*BLOCKSIZE); //find the inode addr for the given inode idx
				file_length_in_byte = node_to_find->inode_length;
				data_block_count = file_length_in_byte/BLOCKSIZE+1;

				for(j=0;j<data_block_count;j++)
				{
					uint32_t data_block;
					data_block = node_to_find->inode_data[j];
					available_datablock[data_block] = 1;
				}

			}
		else
			available_inodes[i] = 0;
	}

}



/**
	HELPER FUNCTION FOR EXTRA CREDIT
	@param file descriptor
	@param buffer for reading
	@param bytes to read
	@return BYTES OF DATA WRITETEN SUCCESFULLY OR -1 IF FAILED 
*/

static int32_t write_helper(int32_t fd, const void* buf, int32_t nbytes)
{
	cli();
	uint32_t file_length_in_byte;
	uint32_t data_block_count;
	int i;
	for(i = 0; i < 63; i++)  //traverse the file_directory_array  MAX FILE IS 63
		{
			int filename_length = strlen((int8_t*)((boot_block_addr->dir_entries[i]).filename)); 
			int input_length = strlen((int8_t*)filename_buffer[current_process->pid]);
			if(filename_length == input_length && strncmp((int8_t*)filename_buffer[current_process->pid],(int8_t*)(boot_block_addr->dir_entries[i]).filename,filename_length) == 0 && filename_length>0)	//if filename is found exactly the same
			{
				
				return -1;
			}
		}

	fd_t* curr_fd = &(current_process->fd_array[fd]);
	//cannot write a directory...
	if(curr_fd->flags.is_directory==1)   return -1;//if not directory

	//check if the first , find a free inode block
	if(curr_fd->inode > MAX_FILE)
 	{
 		for(i=0;i<MAX_FILE;i++)
 			if(available_inodes[i] == 0)
 			{
 				available_inodes[i] = 1; //mark it as occpied
 				curr_fd->inode = i; 
 				break;
 			}
 	}

 		// IF does not find, create a new file
	for(i=0;i<63;i++)//first find a new empty dentry
		{
			int filename_length = strlen((int8_t*)((boot_block_addr->dir_entries[i]).filename)); 
			if(filename_length == 0)//an empty dentry found!
			{
				// printf("			file name is %s currprocess pid    %d\n", fname,current_process->pid);
				(boot_block_addr->dir_entries[i]).inode_number = curr_fd->inode; //initialize with invalid number
				strncpy((int8_t*)(boot_block_addr->dir_entries[i].filename), filename_buffer[current_process->pid],32);//the max file name is 32
				(boot_block_addr->dir_entries[i]).file_type = DENTRY_FILE;

				copy_data_to_hard_disk((char*)multiboot_addr,0);
				break;
			}

		}

 	if(curr_fd->file_pos % BLOCKSIZE == 0) //SHOULD ALLOCATE NEW DATA BLOCK
 	{
 		// printf("                       here\n");
 		for(i = 0; i < MAX_DATA_BLOCK;i++)
 		{
 			if(available_datablock[i] == 0)
 			{
 				available_datablock[i] = 1;
 				inode_t* node_to_find;
				node_to_find = (inode_t*) (inode_start_addr+curr_fd->inode*BLOCKSIZE); //find the inode addr for the given inode idx
				node_to_find->inode_length += nbytes;
				file_length_in_byte = node_to_find->inode_length;
				data_block_count = file_length_in_byte/BLOCKSIZE;
				node_to_find->inode_data[data_block_count] = i; //the data block is this!
				memcpy( (uint8_t*) (data_block_start_addr+i*BLOCKSIZE),buf,nbytes);	//copy the whole thing!
				// printf("datablock found is %d \n",curr_fd->inode );
 		// printf("the inode is %d\n", curr_fd->inode);
				copy_data_to_hard_disk((char*) (multiboot_addr+(1+inode_number+i)*BLOCKSIZE),1+inode_number+i);
				curr_fd->file_pos+=nbytes;
				 // printf("       data block found is %d\n",i );
				break;

 			}
 		}
 	}
 	copy_data_to_hard_disk((char*) (multiboot_addr+(1+curr_fd->inode)*BLOCKSIZE),1+curr_fd->inode);
 	return nbytes;
		
}


// // /**
// // 	FUNCTION FOR TESTING IN CP2
// // 	Temporary function to test print out the directory
// // */
// int32_t test_read_directory(void)
// {
// 	//printf("\n testing reading directory...\n");
// 	int i;
// 	for(i=0;i<MAX_FILE;i++)
// 	{	
// 		if(strlen((int8_t*)boot_block_addr->dir_entries[i].filename))
// 		{
// 			printf(" filename %d is %s\n",i,boot_block_addr->dir_entries[i].filename );
// 		}
// 	}
// 	return 0;
// }
























