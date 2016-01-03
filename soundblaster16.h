#ifndef _SOUNDBLASTER16_H
#define _SOUNDBLASTER16_H
#include "types.h"
uint16_t sb_inb(uint16_t port);
void sb_outb(uint8_t data, uint16_t port);

int Reset_DSP();
void Write_DSP(uint8_t data);
uint16_t Read_DSP();

int DMA_transfer(int channel, uint8_t modebit, uint16_t tranfer_len);

uint16_t get_linear_addr();
int disable_sound(int DMA, int channel);
int clear_port(int DMA);
int modebit_set(int DMA , uint8_t modebit);
int buffoff_set(int DMA, uint16_t linear_addr );
int transferlength(int channel,uint16_t tranfer_len);
int write_pageport(int channel, uint16_t linear_addr);
int enable_sound(int DMA, int channel);

void set_sampling_rate(uint16_t rate);
#endif