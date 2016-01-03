#include "pit.h"
#include "i8259.h"
#include "syscall.h"
#include "lib.h"


int finishing = 0; 

#define  base_frq      1193180
#define  channel_zero  0x40
#define  mask_8        0xFF
#define  eight         8
#define  command_port  0x43
#define  command_byte  0x36
#define  sound_port    0x61
#define  channel_two   0x42
#define  sound_mask    0xFC
#define  command_mask  0xb6
#define PIT_IRQ 0
#define ini_pit_freq 100
#define Do 262  
#define Re 294 
#define Mi 330 
#define Fa 349 
#define Sol 392 
#define La 440 
#define Si 494 
#define hDo 523
#define hRe 587


/**
 * @brief      initlize pit
 *
 * @param[in]  frequency   frequency of pit
 *
 * @return     nothing.
 */
void init_pit(int32_t frequency)  //source:http://www.jamesmolloy.co.uk/tutorial_html/5.-IRQs%20and%20the%20PIT.html
{
	enable_irq(PIT_IRQ);
	int divisor = base_frq / frequency;   // get the divior

   	outb(command_byte, command_port);     //out command byte

  
   uint8_t l = (uint8_t)(divisor & mask_8);      // get low 8 bits
   uint8_t h = (uint8_t)((divisor>>eight) & mask_8 ); //get high 8 bits

  
   outb(l, channel_zero);  //out low bit and high bit
   outb(h, channel_zero);

}
// void pit_interrput()
// {
// 	init_pit(ini_pit_freq);
// }
// //Play sound using built in speaker 
// //source:http://wiki.osdev.org/PC_Speaker




 void play_sound(uint32_t nFrequence) {
 	uint32_t Div;
 	uint8_t tmp;
 
        //Set the PIT to the desired frequency
 	Div = 1193180 / nFrequence;
 	outb(0xb6,0x43);
 	outb((uint8_t) (Div),0x42  );
 	outb((uint8_t) (Div >> 8),0x42);
 
        //And play the sound using the PC speaker
 	tmp = inb(0x61);
  	if (tmp != (tmp | 3)) {
 		outb( tmp | 3,0x61);
 	}
 }
 
 //make it shutup
void nosound() {
 	uint8_t tmp = inb(0x61) & 0xFC;
 
 	outb(tmp,0x61 );
 }
// set finishing in signal handler 
                   // if you want to really stop.

void
wait( int seconds )
{   // this function needs to be finetuned for the specific microprocessor
    int i, j, k;
    int wait_loop0 = 3000;
int wait_loop1 = 1500;
    for(i = 0; i < seconds; i++)
    {
        for(j = 0; j < wait_loop0; j++)
        {
            for(k = 0; k < wait_loop1; k++)
            {   // waste function, volatile makes sure it is not being optimized out by compiler
                int volatile t = 120 * j * i + k;
                t = t + 5;
            }
        }
    }
}
//KELUODIYAKUANGXIANGQU
void music()
{
	
	play_sound(hDo);
	wait(1);
	play_sound(Mi);
	wait(2);
	play_sound(La);
	wait(2);
	play_sound(Si);
	wait(2);
	play_sound(hDo);
	wait(2);
	play_sound(Mi);
	wait(2);
	play_sound(La);
	wait(2);
	play_sound(hDo);
	wait(2);
	play_sound(hRe);
	wait(2);
	play_sound(Mi);
	wait(2);
	play_sound(hDo);
	wait(2);
	play_sound(Mi);
	wait(2);
	play_sound(Si);
	wait(2);
	play_sound(Mi);
	wait(2);
	play_sound(La);
	wait(2);
	play_sound(Mi);
	wait(2);

	play_sound(Si);
	wait(2);
	play_sound(Mi);
	wait(2);
	play_sound(415);
	wait(2);
	play_sound(La);
	wait(2);
	play_sound(Si);
	wait(2);
	play_sound(Mi);
	wait(2);
	play_sound(415);
	wait(2);
	play_sound(Si);
	wait(2);
	play_sound(hDo);
	wait(2);
	play_sound(Mi);
	wait(2);
	play_sound(Si);
	wait(2);
	play_sound(Mi);
	wait(2);
	play_sound(La);
	wait(2);
	play_sound(Mi);
	wait(2);
	play_sound(659);
	wait(2);
	play_sound(Mi);
	wait(2);

	play_sound(hDo);
	wait(2);
	play_sound(Mi);
	wait(2);
	play_sound(La);
	wait(2);
	play_sound(Si);
	wait(2);
	play_sound(hDo);
	wait(2);
	play_sound(Mi);
	wait(2);
	play_sound(La);
	wait(2);
	play_sound(hDo);
	wait(2);
	play_sound(hRe);
	wait(2);
	play_sound(Mi);
	wait(2);
	play_sound(hDo);
	wait(2);
	play_sound(Mi);
	wait(2);
	play_sound(Si);
	wait(2);
	play_sound(Mi);
	wait(2);
	play_sound(La);
	wait(2);
	play_sound(Mi);
	wait(2);

	play_sound(659);
	wait(2);
	play_sound(Mi);
	wait(2);
	play_sound(415);
	wait(2);
	play_sound(Mi);
	wait(2);
	play_sound(hDo);
	wait(2);
	play_sound(Mi);
	wait(2);
	play_sound(Si);
	wait(2);
	play_sound(Mi);
	wait(2);
	play_sound(La);
	wait(2);
	play_sound(Mi);
	wait(2);
	play_sound(659);
	wait(2);
	play_sound(Mi);
	wait(2);
	play_sound(La);
	wait(2);
	play_sound(La);
	wait(2);
	play_sound(La);
	wait(2);
	nosound();
	return;
}