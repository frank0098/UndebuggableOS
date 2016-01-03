/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"
#define MASTER_IRQ_NUM 8
#define INITIAL_MASK 0XFF
 #define SLAVE_PORT 2
/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask = INITIAL_MASK; /* IRQs 0-7 */
uint8_t slave_mask = INITIAL_MASK; /* IRQs 8-15 */

 

 /*
 * i8259_init
 *   DESCRIPTION: Initialize the 8259 PIC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Output bits to 8259 chip
 */ 
void
i8259_init(void)
{
    /*master_mask = 0xFF;
    slave_mask = 0xFF;*/
    uint32_t flags;
    cli_and_save(flags);

	outb(master_mask,MASTER_8259_PORT_2);
	outb(slave_mask,SLAVE_8259_PORT_2);

	/*
	outb Initial a wide range of PC hardware
	*/
	outb(ICW1, MASTER_8259_PORT);
	outb(ICW2_MASTER, MASTER_8259_PORT_2);
	outb(ICW3_MASTER, MASTER_8259_PORT_2);
	outb(ICW4, MASTER_8259_PORT_2);

	outb(ICW1, SLAVE_8259_PORT);
	outb(ICW2_SLAVE, SLAVE_8259_PORT_2);
	outb(ICW3_SLAVE, SLAVE_8259_PORT_2);
	outb(ICW4, SLAVE_8259_PORT_2);


	/*restore master irq mask*/
	outb(master_mask, MASTER_8259_PORT_2);
	outb(slave_mask, SLAVE_8259_PORT_2);
    
    restore_flags(flags);
	printf("the PIC initialized!!\n");
}

 /*
 * enable_irq
 *   DESCRIPTION: Enable (unmask) the specified IRQ
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Output bits to 8259 chip
 */
void
enable_irq(uint32_t irq_num)
{

	uint16_t mask;
	if (irq_num < MASTER_IRQ_NUM)		/*if irq is in the master*/
    {
        mask = ~(1 << irq_num);
    	master_mask &= mask;
    	outb(master_mask, MASTER_8259_PORT_2);
    }
    else  					/*if irq is in the slave*/
    {
        mask = ~(1 << (irq_num % MASTER_IRQ_NUM));
    	slave_mask &= mask;
    	outb(slave_mask, SLAVE_8259_PORT_2);
    }         
}

 /*
 * enable_irq
 *   DESCRIPTION: Disable (mask) the specified IRQ
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Output bits to 8259 chip
 */
void
disable_irq(uint32_t irq_num)
{

	uint16_t mask;

	if (irq_num < MASTER_IRQ_NUM)		//if irq is in the master
    {
        mask = 1 << irq_num;
    	master_mask |= mask;
    	outb(master_mask, MASTER_8259_PORT_2);
    }
    else  					//if irq is in the slave
    {
        mask = 1 << ((irq_num) % MASTER_IRQ_NUM);
    	slave_mask |= mask;
    	outb(slave_mask, SLAVE_8259_PORT_2);
    }         
}
 /*
 * send_eoi
 *   DESCRIPTION: Send end-of-interrupt signal for the specified IRQ
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Output bits to 8259 chip
 */
void
send_eoi(uint32_t irq_num)
{

	if (irq_num >= MASTER_IRQ_NUM)		/*if irq is in the slave*/
    {
    	outb((EOI | (irq_num % MASTER_IRQ_NUM)), SLAVE_8259_PORT);
        outb((EOI | SLAVE_PORT), MASTER_8259_PORT);
    }
    else  					/*if irq is in the master*/
    {
    	outb(EOI | irq_num, MASTER_8259_PORT);	
    }   

}

