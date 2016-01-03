#include "ext2.h"

#define MAX_FILE_SIZE 1023*4  //1023 per inode, 4kb per data block
#define MAX_BUFFER_SIZE 40000
#define MAX_FILE_NAME_LENGTH 32

static ext2_blockdescriptor_t block_descriptor;
static int file_sys_addr;
// static uint16_t sector_addr[TOTAL_SECTOR];//size:100*16    = 1600 byte  need 4 sectors

void init_ext2_sys()
{
	init_hard_disk();
	char buf[BLOCK_SIZE];
	ide_read_sectors(DEVICENUMBER,SECTOR,FS_DESCRIPTOR_ADDR,tss.ss0,(int)&buf);
	memcpy(&block_descriptor,buf,sizeof(ext2_blockdescriptor_t));
	// file_sys_addr = block_descriptor.inodeTableStart+SUPERBLOCK_ADDR+1;
	file_sys_addr = MEM_ADDR;
}

int32_t sync_with_hard_disk()
{
	// int i = 0;
	// int j = 0;
	int k;
	int flag = 0;
	char buf[FILE_SYS_BLOCK_SIZE];
	ide_read_sectors(DEVICENUMBER,8*SECTOR,file_sys_addr,tss.ss0,(int)&buf);
	for(k=0;k<FILE_SYS_BLOCK_SIZE;k++)
				{
					if(buf[k] !=0x00)
					{
						flag=1;
						break;
					}
				}
	if(flag == 0) //first used, need to find out the disk usage
		{
			// printf("flag is 0\n");
			// char tmp_buf[BLOCK_SIZE];
			// while(i<TOTAL_SECTOR)
			// {
			// 	ide_read_sectors(DEVICENUMBER,SECTOR,file_sys_addr+j,tss.ss0,(int)&tmp_buf);
			// 	flag = 0;
			// 	for(k=0;k<BLOCK_SIZE;k++)
			// 	{
			// 		if(tmp_buf[k] !=0x00)
			// 		{
			// 			flag=1;
			// 			break;
			// 		}
			// 	}
			// 	if(flag == 0)
			// 	{
					
			// 		sector_addr[i] = file_sys_addr+j;
			// 		i++;
			// 		j++;
							
			// 	}
			// 	else
			// 		{
			// 			j++;
			// 			printf("this should not happen\n");
			// 		}

			// }
			// ide_write_sectors(DEVICENUMBER, 4*SECTOR, INFO_ADDR, tss.ss0, (int) &sector_addr);
			// for(i = 0;i<TOTAL_SECTOR;i++)
			// 	{
			// 		printf("i%d sector %d ",i,sector_addr[i] );
			// 		if(i%16 == 0)
			// 			printf("\n");
			// 	}
			return 1;
		}
	else
	{
		
		// ide_read_sectors(DEVICENUMBER,4*SECTOR,INFO_ADDR,tss.ss0,(int)&sector_addr);
		// for(i = 0;i<TOTAL_SECTOR;i++)
		// 		{
		// 			printf("i%d sector %d ",i,sector_addr[i] );
		// 			if(i%16 == 0)
		// 				printf("\n");
		// 		}
		
		return 0;
	}
}
void copy_data_to_hard_disk(char* mem,int block_number)
{
	int i;
	for(i=0;i<8;i++)
		{
			// ide_write_sectors(DEVICENUMBER, SECTOR, sector_addr[8*block_number+i], tss.ss0, (int) mem);
			ide_write_sectors(DEVICENUMBER, SECTOR, 8*block_number+i, tss.ss0, (int) (mem+i*BLOCK_SIZE));
		}
}

void copy_data_from_hard_disk(char* mem,int block_number)
{
	int i,j;
	char buf[BLOCK_SIZE];
	
	for(i=0;i<8;i++)
		{
			for(j=0;j<BLOCK_SIZE;j++)
				buf[j] = 0;
			ide_read_sectors(DEVICENUMBER, SECTOR, 8*block_number+i, tss.ss0, (int) (mem+i*BLOCK_SIZE));
			// memcpy(mem+i*BLOCK_SIZE,buf,sizeof(BLOCK_SIZE));
			
		}
	
}

void clean_up_hard_disk(int block_number)
{
	int i;
	char buf[BLOCK_SIZE];
	for(i=0;i<BLOCK_SIZE;i++)
		buf[i]=0;

	ide_write_sectors(DEVICENUMBER, SECTOR, block_number, tss.ss0, (int) &buf);
}

// int32_t read_file_ext2 (int32_t fd, void* buf, int32_t nbytes)
// {
// 	return 0;
// }

// int32_t write_file_ext2 (int32_t fd, const void* buf, int32_t nbytes)
// {
// 	return 0;
// }

// int32_t open_file_ext2 (const uint8_t* filename)
// {
// 	dentry_t mydir;
// 	return read_dentry_by_name_ext2 (filename, &mydir);
// }
// int32_t close_file_ext2 (int32_t fd)
// {
// 	return 0;
// }
//d485 0100 0000 0000 3400 2000 0300 2800

/*
	Function to initialize the super block info
*/
// static void load_ext2_data()
// {
	

	//1000 0000 1e00 0000 4200 0000 0000 0000  
	// for(i=SUPERBLOCK_ADDR;i<16384;i++)  //find out where the file system located at
	// {
	// 	ide_read_sectors(DEVICENUMBER,SECTOR,i,tss.ss0,(int)&buf);
	// 	if(buf[0] == 0x10 && buf[1]==  0x00 && buf[2] == 0x00 && buf[3]==  0x00 &&
	// 		buf[4]==  0x1e && buf[5] == 0x00 && buf[6]==  0x00 && buf[7] == 0x00 &&
	// 		 buf[8]==  0x42 && buf[9] == 0x00 && buf[10]==  0x00 && buf[11] ==  0x00 && 
	// 		  buf[12] ==  0x00 && buf[13] == 0x00 && buf[14] == 0x00 && buf[15] == 0x00)  //check the magic numbers
	// 			{
	// 				ece391_boot_block_addr = i;
	// 				break;
	// 			}
	// }
	// printf("the ece391 boot addr %d\n",ece391_boot_block_addr );
	// for(j=0;j<8;j++)
	// 	ide_read_sectors(DEVICENUMBER,SECTOR,ece391_boot_block_addr+j,tss.ss0,(int)&buf[j*BLOCK_SIZE]);
	// memcpy(&ece391_boot_block,buf,sizeof(ece391_boot_block));
// }

// /**
// 	Function to read directory entry by name
// 	@param file name
// 	@param pass back a dentry
// 	@return 0 if success, -1 if fails
// */

// int32_t read_dentry_by_name_ext2 (const uint8_t* fname, dentry_t* dentry)
// {
// 	if (fname == NULL)	//if fname is empty
// 	{
// 		dentry = NULL;
// 		return -1;
// 	}
// 	else
// 	{
// 		int i;
// 		for(i = 0; i < MAX_FILE_SIZE; i++)  //traverse the file_directory_array  MAX FILE IS 63
// 		{
// 			int filename_length = strlen((int8_t*)((ece391_boot_block.dir_entries[i]).filename)); 
// 			int input_length = strlen((int8_t*)fname);
// 			if(filename_length == input_length && strncmp((int8_t*)fname,(int8_t*)(ece391_boot_block.dir_entries[i]).filename,filename_length) == 0 && filename_length>0)	//if filename is found exactly the same
// 			{
// 				dentry->file_type = (ece391_boot_block.dir_entries[i]).file_type;
// 				dentry->inode_number = (ece391_boot_block.dir_entries[i]).inode_number;  //copy the data the pointer passed in 
// 				return 0;
// 			}
// 		}
// 		dentry = NULL;
// 		return -1;
// 	}

// }

// int32_t test_read_directory_ext2(void)
// {
// 	//printf("\n testing reading directory...\n");
// 	int i;
// 	for(i=0;i<63;i++)
// 	{	
// 		if(strlen((int8_t*)ece391_boot_block.dir_entries[i].filename))
// 		{
// 		printf(" filename %d is %s inode is %d \n",i,ece391_boot_block.dir_entries[i].filename,ece391_boot_block.dir_entries[i].inode_number );
// 		}
// 	}
// 	return 0;
// }

// int32_t read_data_ext2 (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
// {
	
// }

// /**
// 	Temporary function to test read a file
// */

// int32_t test_read_file_ext2(char* filename,uint32_t offset,uint32_t length,int32_t index)
// {
// 	int i;
// 	dentry_t directory;
// 	uint32_t inode;
// 	uint8_t data[MAX_BUFFER_SIZE];
// 	int32_t return_value;
	

// 	if(index == -1)
// 	{
// 		if(read_dentry_by_name_ext2((uint8_t*)filename,&directory) == -1)
// 		{
// 			printf("\n Fail to find the file %s\n\n",filename);
// 			return -1;
// 		}
// 	}
// 	else
// 	{
// 		return -1;
// 	}

// 	inode = directory.inode_number;
// 	if((return_value = read_data_ext2(inode,offset,data,length)) <0 )
// 	{
// 		printf("\n read data failed!!!!!\n\n");
// 		return -1;
// 	}
// 	// printf(" the data read is %d\n",return_value);
// 	//data[return_value] = '\0';
// 	////printf("%s\n",data);
// 	// for(i=0;i<return_value;i++)
// 	// 	printf("%c",data[i] );
// 	//printf("\n");
// 	//printf("print finished\n");
// 	return 0;
	

// }


