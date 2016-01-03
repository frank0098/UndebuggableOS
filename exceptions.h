#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "lib.h"
#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
/*All the exceptions given by the Intel manual*/
extern void division_by_zero();
extern void debug_exception();
extern void non_maskable_int();
extern void break_point();
extern void into_overflow();
extern void out_of_bounds();
extern void invalid_opcode();
extern void device_not_available();
extern void double_fault();
extern void co_segment_overrun();
extern void invalid_tss();
extern void segment_not_present();
extern void stack_fault();
extern void general_protection();
extern void page_fault();

extern void x87_floating_point();
extern void alignment_check();
extern void machine_check();
extern void SIMD_floating_point();

#endif


