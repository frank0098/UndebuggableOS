#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"

/** @brief User program starts at virtual address 128MB. */
#define USER_PROGRAM_START 0x08048000

/** @brief User program lives in a 4MB page. */
#define USER_PAGE_SIZE 0x400000

/** @brief User stack starts at end of page. */
#define USER_STACK_START 0x08400000

/** @brief Video memory physical address */
#define VIDEO_MEM_BASE_ADDR 0xB8000

/** @brief Video memory lives in a 4KB page. */
#define VIDEO_PAGE_SIZE 0x1000

/** @brief Virtual video memory physical address */
#define VIRTUAL_VIDEO_MEM_BASE_ADDR 0x0A000000


void init_paging();


int32_t switch_pd(uint32_t pid);


int32_t update_video_pt();

#endif /* _PAGING_H */
