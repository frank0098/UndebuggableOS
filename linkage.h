#ifndef _LINKAGE_H
#define _LINKAGE_H
#include "rtc.h"
#include "keyboard.h"

#include "mouse.h"

#include "scheduler.h"


/*
 * keyboard_link
 *   DESCRIPTION: The linkage function for the keyboard
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Call the keyboard interrupt handler
 */ 
extern void keyboard_link();

/*
 * rtc_link
 *   DESCRIPTION: The linkage function for the rtc
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Call the rtc interrupt handler
 */ 
extern void rtc_link();


/*
 * mouse_link
 *   DESCRIPTION: The linkage function for the mouse
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Call the mouse interrupt handler
 */ 
extern void mouse_link();


/*
 * sched_link
 *   DESCRIPTION: The linkage function for scheduler
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Call the schedule interrupt handler
 */ 
extern void sched_link();


#endif



