#ifndef _MOUSE_H
#define _MOUSE_H

#include "lib.h"
#include "x86_desc.h"
#include "types.h"
#include "i8259.h"
#include "multiboot.h"
#include "debug.h"
#include "terminal.h"

void init_mouse();

extern void mouse_int_handle();

void move(int32_t x, int32_t y);
#endif






















