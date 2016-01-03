#ifndef _SCHEDULER_H
#define _SCHEDULER_H


#include "types.h"
#include "syscall.h"
#include "paging.h"
#include "process.h"
#include "i8259.h"
#include "rtc.h"

void init_sche();
void scheduling();
void add_termianl(uint8_t current_tid);
void add_new_proc_sche(uint8_t current_tid);   

#endif /* _SCHEDULER_H */







