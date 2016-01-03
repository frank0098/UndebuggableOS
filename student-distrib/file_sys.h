#ifndef _FILESYS_H
#define _FILESYS_H

#include "lib.h"
#include "types.h"
#include "harddisk.h"
#include "process.h"
#include "ext2.h"

/*
	The structure for boot block
*/
#define MAX_FILE 63
#define DENTRY_RESERVED 24 
#define BOOT_BLOCK_RESERVED 52
#define MAX_DATA_BLOCK 1023
#define MAX_FILE_NAME_LENGTH 32
/*
	Index for file type
*/
#define DENTRY_RTC 0
#define DENTRY_DIRECTORY 1
#define DENTRY_FILE 2
/*
	The structure for dentry_t
*/
typedef struct dentry_t
{
	uint8_t  filename[MAX_FILE_NAME_LENGTH];
	uint32_t file_type;
	uint32_t  inode_number;
	uint8_t 	reserved[DENTRY_RESERVED];

}dentry_t;


/*
	The structure for inode
*/
typedef struct inode_t
{
	uint32_t inode_length;
	uint32_t inode_data[MAX_DATA_BLOCK];
	
}inode_t;

/*
	The structure for superblock
*/
typedef struct boot_block_t  
{
	uint32_t dir_entries_num;
	uint32_t inodes_N;
	uint32_t data_blocks_D;
	uint8_t  reserved[BOOT_BLOCK_RESERVED];
	dentry_t dir_entries[MAX_FILE];

}boot_block_t;

int32_t read_file (int32_t fd, void* buf, int32_t nbytes);

int32_t write_file (int32_t fd, const void* buf, int32_t nbytes); 

int32_t open_file (const uint8_t* filename);

int32_t close_file (int32_t fd);

void init_file_sys(uint32_t input);

int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);

int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);

int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t file_loader(const uint8_t* filename, uint8_t* buf, uint32_t nbytes);

//Function for TESTING

// int32_t test_read_directory(void);

// int32_t test_read_file(char* filename,uint32_t offset,uint32_t length,int32_t index);

#endif /* _FILESYS_H */
