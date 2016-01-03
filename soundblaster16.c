#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "multiboot.h"
#include "x86_desc.h"
#include "debug.h"
#include "pit.h"
#include "syscall.h"
#define reset_port 0x6 
#define status_port 0xE
#define read_port  0xA
#define write_port 0xC
#define ack_port   0xF
#define base_port  0x220

#define DMA1_maskR 0x0A
#define DMA2_maskR 0xD4
#define DMA1_clear 0x0C
#define DMA2_clear 0xD8
#define DMA1_mode  0x0B
#define DMA2_mode  0xD6
#define DMA1_baseaddr  0x00
#define DMA2_baseaddr  0xC0
#define DMA1_count  0x00
#define DMA2_count  0xC0

#define pagechannel_0  0x87
#define pagechannel_1  0x83
#define pagechannel_2  0x81
#define pagechannel_3  0x82
#define pagechannel_5  0x8B
#define pagechannel_6  0x89
#define pagechannel_7  0x8A

#define countchannel_0  0x01
#define countchannel_1  0x03
#define countchannel_2  0x05
#define countchannel_3  0x07
#define countchannel_4  0xC2
#define countchannel_5  0xC6
#define countchannel_6  0xCA
#define countchannel_7  0xCE
#define sample_output   0x41
/*2xAh - DSP Read
2xCh - DSP Write (Command/Data), DSP write-buffer status (Bit 7)
2xEh - DSP Read-buffer status (Bit 7), DSP interrupt acknowledge
2xFh - DSP 16-bit interrupt acknowledge*/

void sb_outb(uint8_t data, uint16_t port)
{
	outb(data,port+base_port);
	return;
}
uint16_t sb_inb(uint16_t port)
{
	return inb(base_port + port);
}


int Reset_DSP()
{
	int r=0;
	
	outb(1,reset_port);
	int i =10000;
	while(i>0)
	{
		i--;
	}
	outb(0,reset_port);
	
	while(1)
	{
		r = sb_inb(status_port);
		if(r == 0x7)  break;
	}
	while(1)
	{
		r = sb_inb(read_port);
		if(r== 0xAA)  break;
	}
	return 0;

}
void Write_DSP(uint8_t data)
{
	uint16_t r;
	while(1)
	{
		r= sb_inb(write_port);
		if(r==0x7) break;
	}
	sb_outb(data,write_port);
}
uint16_t Read_DSP()
{
	int r;
	while(1)
	{
		r= sb_inb(status_port);
		if(r==0x7) break;
	}
	return sb_inb(read_port);
}
uint16_t get_linear_addr()
{
	return 0;
}

int write_pageport(int channel, uint16_t linear_addr)
{
	linear_addr = linear_addr / 65536;
	switch(channel){
	case 0:
	{
		sb_outb(linear_addr,pagechannel_0);
		return 0;
	}
	case 1:
	{
		sb_outb(linear_addr,pagechannel_1);
		return 0;
	}
	case 2:
	{
		sb_outb(linear_addr,pagechannel_2);
		return 0;
	}
	case 3:
	{
		sb_outb(linear_addr,pagechannel_3);
		return 0;
	}
	case 5:
	{
		sb_outb(linear_addr,pagechannel_5);
		return 0;
	}
	case 6:
	{
		sb_outb(linear_addr,pagechannel_6);
		return 0;
	}
	case 7:
	{
		sb_outb(linear_addr,pagechannel_7);
		return 0;
	}
	default:
	{
		return -1;
	}
 }
}
int transferlength(int channel,uint16_t tranfer_len)
{
	tranfer_len --;
	uint8_t lo_len = (tranfer_len & 0x00FF);
	uint8_t hi_len = (tranfer_len & 0xFF00)>>8;
	switch(channel){
	case 0:
	
		sb_outb(lo_len,countchannel_0);
		sb_outb(hi_len,countchannel_0);
		return 0;
	
	case 1:
	
		sb_outb(lo_len,countchannel_1);
		sb_outb(hi_len,countchannel_1);
		return 0;
	
	case 2:
	
		sb_outb(lo_len,countchannel_2);
		sb_outb(hi_len,countchannel_2);
		return 0;
	
	case 3:
	
		sb_outb(lo_len,countchannel_3);
		sb_outb(hi_len,countchannel_3);
		return 0;
	
	case 4:
	
		sb_outb(lo_len,countchannel_4);
		sb_outb(hi_len,countchannel_4);
		return 0;
	
	case 5:
	
		sb_outb(lo_len,countchannel_5);
		sb_outb(hi_len,countchannel_5);
		return 0;
	
	case 6:
	
		sb_outb(lo_len,countchannel_6);
		sb_outb(hi_len,countchannel_6);
		return 0;
	
	case 7:
	
		sb_outb(lo_len,countchannel_7);
		sb_outb(hi_len,countchannel_7);
		return 0;
	
	default:return -1;}
	
		
	
}

int disable_sound(int DMA, int channel)
{
	uint8_t  maskport_bit  =0;
	maskport_bit = 4 + channel % 4;
	if(DMA ==1)
	{
	sb_outb(maskport_bit, DMA1_maskR);
	return 0;
	}
	else if(DMA==2)
	{
	sb_outb(maskport_bit, DMA2_maskR); 
	return 0;
	}
	else return -1;
	

}

int enable_sound(int DMA, int channel)
{
	uint8_t maskport_bit = channel % 4; 
	if(DMA==1)
	{
		sb_outb(maskport_bit, DMA1_maskR);
		return 0;
	}
	else if(DMA==2)
	{
		sb_outb(maskport_bit, DMA2_maskR);
		return 0;
	}
	else return -1;
}

int clear_port(int DMA)
{
	if(DMA==1)
	{
		sb_outb(0,DMA1_clear);  
		return 0;
	}
	else if(DMA==2)
	{
		sb_outb(0,DMA2_clear);  
		return 0;
	}
	else return -1;
}

int modebit_set(int DMA , uint8_t modebit, int channel)
{	
	modebit = modebit | channel % 4;
	if(DMA==1)
	{
		sb_outb(modebit,DMA1_mode);
		return 0;
	}
	else if(DMA==2)
	{
		sb_outb(modebit,DMA2_mode);
		return 0;
	}
	else return -1;
}

int buffoff_set(int DMA, uint16_t linear_addr )
{	
	uint16_t bufferoffset = 0;      
	uint8_t lobuff = 0;
	uint8_t hibuff = 0;
	if(DMA==1)
	{
		bufferoffset = linear_addr % 65536; 
		 lobuff = bufferoffset & 0x00FF;
		 hibuff = (bufferoffset & 0xFF00)>>8;
		sb_outb(lobuff,DMA1_baseaddr);
		sb_outb(hibuff,DMA1_baseaddr);
		return 0;
	}
	else if(DMA==2)
	{
		bufferoffset = (linear_addr / 2 )% 65536 ; 
		 lobuff = bufferoffset & 0x00FF;
		 hibuff = (bufferoffset & 0xFF00)>>8;
		sb_outb(lobuff,DMA2_baseaddr);
		sb_outb(hibuff,DMA2_baseaddr);
		return 0;
	}
	else return -1;
}

int DMA_transfer(int channel, uint8_t modebit/*???*/, uint16_t tranfer_len/***/)
{
	int DMA = 0;
	uint16_t linear_addr = get_linear_addr();  ///?


	if(channel>7 || channel <0) return -1;
	if(channel>=0 && channel <=3 ) DMA = 1;
	else                           DMA = 2;
    

	
	if(disable_sound(DMA,channel)==-1)              return -1;
	if(clear_port(DMA)==-1)                         return -1;
	if(modebit_set(DMA,modebit,channel)==-1)        return -1;
	if(buffoff_set(DMA,linear_addr) == -1)          return -1;
	if(transferlength(channel,tranfer_len) == -1)   return -1;
	if(write_pageport(channel,linear_addr)==-1)     return -1; 
	if(enable_sound(DMA,channel) ==-1)              return -1;
	return 0;
	
		
	//uint8_t  mode_bit2 = 0;
	//mode_bit2 = mode_bit2 | 0x40;        //single mode
	//mode_bit2 = mode_bit2 | 0x20;			//address increment
	//mode_bit2 = mode_bit2 | 0x10;          //auto-initialization bit /
	//modebit = modebit + channel %4 ;     //// write DMA mode
	
	
	
}



void set_sampling_rate(uint16_t rate)
{
	uint8_t lo = rate & 0x00FF;
	uint8_t hi = (rate & 0xFF00) >>8 ;
	Write_DSP(sample_output);
	Write_DSP(hi);
	Write_DSP(lo);
}

void soundIO()
{

}



