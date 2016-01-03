#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "multiboot.h"
#include "x86_desc.h"
#include "debug.h"

// #define RTC_DATA_REG 0x71
#define RTC_PORT_REG 0x70
#define RTC_PORT_INFO 0x71
#define disable_flag 0x80
#define enable_flag 0x7F
#define REG_A 0x8A
#define REG_B 0x8B
#define REG_C 0x8C
#define rtc_irq 8
#define NMI_mask 0x40
#define initial_freq 2
#define freq_upper 1024
#define set_mask 0xF0
#define max_freq 16
#define sixty    60
#define twentyfour 24
#define ten       10
#define RTC_IRQ 8
#define base_num 32768
#define fifteen  15

uint8_t status[80];
extern  int time[3];
extern  int current_tid;
#define VIDEO 0xB8000

//RTC FLAG
//volatile  int INT = 0; 

 /**
  *Disable_Non_Maskable_Interrupts
 * @brief     Disable non-maskable intterupts
  * input:none
  * output:none
 */
void Disable_Non_Maskable_Interrupts(void) {
	 cli();
	 uint8_t temp = inb(RTC_PORT_REG);
     outb(temp | disable_flag, RTC_PORT_REG);
}


 /**
  *Enable_Non_Maskable_Interrupts
 * @brief     Enable non-maskable interrupts
 * input:none
  * output:none
 */  
void Enable_Non_Maskable_Interrupts(void) {
     uint8_t temp = inb(RTC_PORT_REG);
	 outb(temp & enable_flag, RTC_PORT_REG);
	 sti();
}


/**
 *initial_rtc
 * @brief     initiliazaion rtc function
 * input:none
 * output:none
 */
void initial_rtc()
{
	
	Disable_Non_Maskable_Interrupts();
	INT = 0;

    outb(REG_B,RTC_PORT_REG);
	uint8_t temp = inb(RTC_PORT_INFO);	      
	//temp = temp | NMI_mask;           //mask all the NMI
	outb(REG_B, RTC_PORT_REG);		
	outb(NMI_mask|temp, RTC_PORT_INFO);

	/*Set the frequency to 2Hz*/
	rtc_change_freq(2);  //change to inital freq = 2
	Enable_Non_Maskable_Interrupts();

	
}


/**
 *rtc_int_handle
 * @brief      rtc interrupt handler function 
 * input:none
 * output:none
 */
void rtc_int_handle()
{	
	/*For debug and demo*/
	//test_interrupts();
	outb(REG_C,RTC_PORT_REG);        // first write REG_C to portreg 
	inb(RTC_PORT_INFO);				// read the portinfo 
	INT += 1 ;						//set the INT =1
	/*Send end of int signal*/
	send_eoi(rtc_irq);
}

/**
 *rtc_open
 * @brief       rtc open function.  open the rtc and call initial_rtc
 * input none
 *
 * @return      0 
 */
int32_t rtc_open()
{
	initial_rtc();
	return 0;
}

/**
 * @brief       rtc close function just return 0
 *
 * @return      0
 */
int32_t rtc_close()
{
	return 0;
}

/**
 * @brief       rtc_wait function which will be called by rtc read.
 * 
 * this function will wait fot the interrupt
 *
 * @return     0
 */
int32_t rtc_wait()
{
	//INT = 0;
	sti();
	/*Wait for next interrupt*/
	int32_t temp = INT;
	while(1)
	{
		if(INT != temp) break;
	}
	
	return 0;
}

/**
 * @brief      system call  rtc read
 *
 * @param[in]  fd      file_descriptor (ignored)
 * @param[in]  buf     buffer          (ignored)
 * @param[in]  nbytes  nbytes          (ignored)
 *
 * @return     0
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes)
{
	rtc_wait();
	return 0;
}

/**
 * @brief      change the frequency of rtc
 *
 * @param[in]  freq   the frequency we need
 *
 * @return     0 if succeed     -1 if failed
 */
int32_t rtc_change_freq(int32_t freq)
{
	if(freq > freq_upper || freq < initial_freq)  //check the frequency range
	{
		printf(" freq is out of range" );
		return -1;
	}
	//if(check_power(freq) == 0)                   //check if is power of 2
	if ((freq & (freq-1)) != 0)
	{
		printf(" freq is not power of 2");
		return -1;
	}
	
	//int32_t rate = transfer_to_rate(freq);      //transfer the frequency to rate
	 int32_t rate = 2;
	 for (;rate<=fifteen;rate++) { 
		if ( (base_num >> (rate-1)) == freq ) {
			break;
		}
	}

	//cli();
	Disable_Non_Maskable_Interrupts();

	/*
	int temp = inb(RTC_PORT_INFO);             //set ports
	temp = (temp & set_mask) | rate;
	outb(REG_A,RTC_PORT_REG);
	outb(temp,RTC_PORT_INFO);
    */
    outb(REG_A, RTC_PORT_REG );
	int8_t prev = inb(RTC_PORT_INFO);
    outb(REG_A, RTC_PORT_REG );
	outb( (prev & set_mask)|rate, RTC_PORT_INFO);

//DEBUG
//curr_freq = freq;
    
    Enable_Non_Maskable_Interrupts();
	//sti();
	
	return 0;

}

/**
 * @brief      system call rtc_write
 *
 * @param[in]  fd      file_descriptor (ignored)
 * @param[in]  buf     buffer          (ignored)
 * @param[in]  nbytes  nbytes          (ignored)
 *
 * @return     the rate of rtc
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes)
{
	if(buf==NULL)  return -1;
	int32_t freq = *((int32_t*)buf);          //get the freq and call rtc_change_freq
	
	int32_t value = rtc_change_freq(freq);
	return value;

}


/**
 * @brief     transfer the frequency to the rate
 *
 * @param[in]  freq  frequency we need 
 *
 * @return     rate 
 */
int32_t transfer_to_rate(int32_t freq)
{
	int32_t temp = 0;                       //log2 of freq
	while(freq != 1)
	{
		freq = freq / 2;
		temp++;
	}
	return max_freq - temp ;
}

/**
 * @brief      check if it is power of 2 
 *
 * @param[in]  x    frequency
 *
 * @return     1 it is power of 2  0 it is not
 */
int check_power(int32_t x)              
{
	 return ((x != 0) && !(x & (x - 1)));  //check power of two 
}

/**
 * @brief     set time array
 *
 * @param[in]  none
 *
 * @return     none
 */
void set_time()                    //set the time array 
{
	time[2]++;                      //time[2] is seconds
	if(time[2] == sixty)
	{
		time[2] = 0;
		time[1]++;
		if(time[1] == sixty)           //time[1] is minutes
		{
			time[1] = 0;
			time[0]++;
			if(time[0] == twentyfour)        //time[0] is hour
			{
				time[0]=0;
				time[1]=0;
				time[2]=0;
			}
		}
	}
	// int h1=time[0]/ten;                 //get each number
	// int h2=time[0]%ten;
	// int m1=time[1]/ten;
	// int m2=time[1]%ten;
	// int s1=time[2]/ten;
	// int s2=time[2]%ten;
	
}

void status_bar()
{
    int i, j=0;
    for (i=0; i<80; i++)
        status[i]=' ';
    status[0]='T';
    status[1]='e';
    status[2]='r';
    status[3]='m';
    status[4]='i';
    status[5]='n';
    status[6]='a';
    status[7]='l';
    if (current_tid==0)
        status[9]='1';
    if (current_tid==1)
        status[9]='2';
    if (current_tid==2)
        status[9]='3';
    int h1=time[0]/10;
    int h2=time[0]%10;
    int m1=time[1]/10;
    int m2=time[1]%10;
    int s1=time[2]/10;
    int s2=time[2]%10;
    status[40] =  (char)(s2+48);
    status[39] =  (char)(s1+48);
    status[38] = ':';
    status[37] =  (char)(m2+48);
    status[36] =  (char)(m1+48);
    status[35] = ':';
    status[34] =  (char)(h2+48);
    status[53] =  (char)(h1+48);
    status[79] = 'X';
    status[78] = 'E';
    status[77] = 'L';
    status[76] = 'O';
    status[75] = 'R';

 
    for (i=(24)*80; i<(25)*80; i++, j++)
    {
        *(uint8_t*)(VIDEO+(i<<1)+1)=0x3F;
        *(uint8_t*)(VIDEO+(i<<1))=status[j];

    }

}
void clear_statusbar()
{
	int i,j;
	for (i=(24)*80; i<(25)*80; i++, j++)
    {
        *(uint8_t*)(VIDEO+(i<<1)+1)=0x00;

    }
}



