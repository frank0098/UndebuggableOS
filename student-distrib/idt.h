
#ifndef _IDT_H
#define _IDT_H

#include "lib.h"
#include "x86_desc.h"
#include "types.h"
#include "exceptions.h"
#include "syscall_wrapper.h"
#include "linkage.h"


/**
     * The function to initialize the table
     */
void idt_initialize();

#endif

