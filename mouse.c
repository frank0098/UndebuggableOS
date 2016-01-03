#include "mouse.h"


typedef struct position {
    int32_t x;
    int32_t y;
} pos_t;
static pos_t mouse;
//static pos_t prev_mouse;

void send_data(uint8_t data, uint8_t port);
uint8_t read_mouse();
void move(int32_t dx, int32_t dy);


/*bytes cannot be read from port 0x60 until bit 0 (value=1) of port 0x64 is set.*/
uint8_t read_mouse() {
	while((inb(0x64) & 1) == 0)
	{

	}
	return inb(0x60);
}

/*All output to port 0x60 or 0x64 must be preceded by waiting 
for bit 1 (value=2) of port 0x64 to become clear*/
void send_data(uint8_t data, uint8_t port) {
	while((inb(0x64) & 2) != 0)
	{

	}
	/*Sending a command or data byte to the mouse (to port 0x60)
	 must be preceded by sending a 0xD4 byte to port 0x64*/
	outb(0xD4, 0x64);
	while((inb(0x64) & 2) != 0)
	{

	}
	outb(data, port);
}


extern void mouse_int_handle()
{
	uint8_t flag;
	if ((inb(0x64) & 1) == 0)
	{
		flag = 0;
	}
	else
	{
		flag = inb(0x60);
	}

	if (flag != 0)
	{
		if (flag == 0xFA)/*ACK*/
		{

		}
		else	
		{
			if (((flag & (1 << 3)) != 0) &&
                ((flag & (1 << 6)) == 0) &&
                ((flag & (1 << 7)) == 0)) 
			{
                int32_t dx = read_mouse();
                if (flag & (1 << 4)) {
                    dx |= 0xFFFFFF00;
                }
                int32_t dy = read_mouse();
                if (flag & (1 << 5)) {
                    dy |= 0xFFFFFF00;
                }
                move(dx, dy);
			}
		}
	}

	send_eoi(12);
	return;
}

void init_mouse()
{
	send_data(0xFF, 0x60);//Reset the mouse
	send_data(0x20,0x64);
	uint8_t status = read_mouse();
	/*After you get the Status byte, you need to set bit number 1 (value=2, Enable IRQ12), 
	and clear bit number 5 (value=0x20, Disable Mouse Clock). 
	Then send command byte 0x60 ("Set Compaq Status") to port 0x64, 
	followed by the modified Status byte to port 0x60. */
	status |= 2;
	status &= 0xDF;
	send_data(0x60,0x64);
	while((inb(0x64) & 2) != 0)
	{

	}
	outb(status, 0x60);

	mouse.x = 0;
	mouse.y = 0;
	/*prev_mouse.x = 0;
	prev_mouse.y = 0;*/

	send_data(0xF4, 0x60);
}


void move(int32_t dx, int32_t dy)
{
	/*prev_mouse.x = mouse.x / 4;
	prev_mouse.y = mouse.y / 8;*/
	/*Send to terminal*/

	mouse.x += dx;
	if (mouse.x < 0)
	{
		mouse.x = 0;
	} 
	else if (mouse.x >= 80 * 4)
	{
		mouse.x = 80 * 4 - 1;
	}

	mouse.y -= dy;
	if (mouse.y < 0)
	{
		mouse.y = 0;
	} 
	else if (mouse.y >= 25 * 8)
	{
		mouse.y = 25 * 8 - 1;
	}

	terminal_mouse(mouse.x / 4, mouse.y / 8);

	return;
}















