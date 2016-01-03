#ifndef EXT2_H
#define EXT2_H

#define BLOCK_SIZE 512
#define FILE_SYS_BLOCK_SIZE 4096
#define DEVICENUMBER 1
#define SECTOR 1
#define EXT2_NAME_LEN 255
#define SUPERBLOCK_ADDR 65
#define FS_DESCRIPTOR_ADDR 67
#define EXT2_BAD_INO 1 
#define EXT2_ROOT_INO 2
#define MEM_ADDR 0
#define TOTAL_SECTOR 800
#define INFO_ADDR 2000

#include "types.h"
#include "harddisk.h"
#include "x86_desc.h"

typedef struct { 
     uint32_t      inodeCount; 
     uint32_t      blockCount; 
     uint32_t      reservedBlockCount; 
     uint32_t      unallocatedBlockCount; 
     uint32_t      unalloactedInodeCount; 
     uint32_t      firstDataBlock; 
     uint32_t      blockSizeLog2; 
     uint32_t      fragmentSizeLog2; 
     uint32_t      blocksInGroupCount; 
     uint32_t      fragmentsInGroupCount; 
     uint32_t      inodesInGroupCount; 
     uint32_t      lastMountTime; 
     uint32_t      lastWriteTime; 
     uint16_t      mountCountSinceCheck; 
     uint16_t      maxMountCountBeforeCheck; 
     uint16_t      ext2Signature; 
     uint16_t      filesystemState; 
     uint16_t      errorHandling; 
     uint16_t      versionMinor; 
     uint32_t      consistencyCheckTime; 
     uint32_t      intervalConsistencyCheckTime; 
     uint32_t      operatingSystemId; 
     uint32_t      versionMajor; 
     uint16_t      reservedBlocksUid; 
     uint16_t      reservedBlocksGid; 
} ext2_superblock_t; 

typedef struct { 
    uint32_t      firstNonReservedInode; 
    uint16_t      inodeSize; 
    uint16_t      superblockBlockgroup; 
    uint32_t      optionalFeatures; 
    uint32_t      requiredFeatures; 
    uint32_t      readonlyFeatures; 
    char        fileSystemId[16]; 
    char        volumeName[16]; 
    char        lastMountPath[64]; 
    uint32_t      compressionUsed; 
    uint8_t       preallocateFileBlockCount; 
    uint8_t       preallocateDirectoryBlockCount; 
    uint16_t      _reserved; 
 char        journalId[16]; 
 uint32_t      journalInode; 
 uint32_t      journalDevice; 
 uint32_t      orphanInodeListHead; 
 } ext2_superblock_extended_t; 
 
 

 typedef struct { 
     uint32_t      blockUsageBitmapAddress; 
     uint32_t      inodeUsageBitmapAddress; 
     uint32_t      inodeTableStart; 
     uint16_t      unallocatedBlockCount; 
     uint16_t      unallocatedInodeCount; 
     uint16_t      directoryCount; 
     char          _reserved[14]; 
 } ext2_blockdescriptor_t; 

typedef struct { 
    uint16_t      typeAndPermissions; 
    uint16_t      uid; 
    uint32_t      sizeLow; 
    uint32_t      atime; 
    uint32_t      ctime; 
    uint32_t      mtime; 
    uint32_t      dtime; 
    uint16_t      gid; 
    uint16_t      linkCount; 
    uint32_t      sectorCount; 
    uint32_t      flags; 
    uint32_t      ossValue; 
    uint32_t      directPointerBlock[12]; //pointers to data blocks
    uint32_t      singleIndirectPointerBlock; 
    uint32_t      doubleIndirectPointerBlock; 
    uint32_t      tripleIndirectPointerBlock; 
    uint32_t      generatorNumber; 
    uint32_t      fileACL; 
    union { 
        uint32_t      sizeHigh; 
        uint32_t      directoryACL; 
    }; 
} ext2_inode_t; 



typedef struct { 
    uint32_t      inode_nr; 
    uint16_t      rec_len; 
    uint8_t       name_len; 
    uint8_t       file_type; 
    char        name[EXT2_NAME_LEN]; 
} ext2_dir_t; 

typedef struct { 
    // Pointers to pre-read items 
    ext2_superblock_t       *superblock; 
    ext2_blockdescriptor_t  *block_descriptor; 
    void                    *block_bitmap; 
    void                    *inode_bitmap; 

    // Precalced values 
    uint32_t                  block_size; 
    uint32_t                  group_count; 
    uint32_t                  sectors_per_block; 
    uint32_t                  first_group_start; 
    uint32_t                  group_descriptor_count; 
} ext2_info_t; 


void init_ext2_sys();
void copy_data_to_hard_disk(char* mem,int block_number);
void copy_data_from_hard_disk(char* mem,int block_number);
void clean_up_hard_disk(int block_number);
int32_t sync_with_hard_disk();
// int32_t read_file_ext2 (int32_t fd, void* buf, int32_t nbytes);
// int32_t write_file_ext2 (int32_t fd, const void* buf, int32_t nbytes);
// int32_t open_file_ext2 (const uint8_t* filename);
// int32_t close_file_ext2 (int32_t fd);

#endif

