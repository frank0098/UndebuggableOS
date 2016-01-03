/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "idt.h"
#include "paging.h"
#include "file_sys.h"
#include "terminal.h"
#include "process.h"
#include "syscall.h"
#include "pit.h"
#include "ece391syscall.h"
#include "mouse.h"
#include "vga_font.h"
//#include "harddisk.h"
//#include "ext2.h"
#define RTC_IRQ 8
#define PIT_IRQ 0
#define KEY_IRQ 1
#define CAS_IRQ 2
#define PIT_TIME 100
#define TERMINAL1 0
#define PROCESS1_KERNEL_STACK 0
/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
void
entry (unsigned long magic, unsigned long addr)
{
	multiboot_info_t *mbi;
	int32_t file_system_addr;

	/* Clear the screen. */
	clear();

	/* Am I booted by a Multiboot-compliant boot loader? */
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		printf ("Invalid magic number: 0x%#x\n", (unsigned) magic);
		return;
	}

	/* Set MBI to the address of the Multiboot information structure. */
	mbi = (multiboot_info_t *) addr;

	/* Print out the flags. */
	printf ("flags = 0x%#x\n", (unsigned) mbi->flags);

	/* Are mem_* valid? */
	if (CHECK_FLAG (mbi->flags, 0))
		printf ("mem_lower = %uKB, mem_upper = %uKB\n",
				(unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);

	/* Is boot_device valid? */
	if (CHECK_FLAG (mbi->flags, 1))
		printf ("boot_device = 0x%#x\n", (unsigned) mbi->boot_device);

	/* Is the command line passed? */
	if (CHECK_FLAG (mbi->flags, 2))
		printf ("cmdline = %s\n", (char *) mbi->cmdline);

	if (CHECK_FLAG (mbi->flags, 3)) {
		int mod_count = 0;
		int i;
		module_t* mod = (module_t*)mbi->mods_addr;
		file_system_addr = mod->mod_start;
		while(mod_count < mbi->mods_count) {
			printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start);
			printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
			printf("First few bytes of module:\n");
			for(i = 0; i<16; i++) {
				printf("0x%x ", *((char*)(mod->mod_start+i)));
			}
			printf("\n");
			mod_count++;
			mod++;
		}

	}
	/* Bits 4 and 5 are mutually exclusive! */
	if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5))
	{
		printf ("Both bits 4 and 5 are set.\n");
		return;
	}

	/* Is the section header table of ELF valid? */
	if (CHECK_FLAG (mbi->flags, 5))
	{
		elf_section_header_table_t *elf_sec = &(mbi->elf_sec);

		printf ("elf_sec: num = %u, size = 0x%#x,"
				" addr = 0x%#x, shndx = 0x%#x\n",
				(unsigned) elf_sec->num, (unsigned) elf_sec->size,
				(unsigned) elf_sec->addr, (unsigned) elf_sec->shndx);
	}

	/* Are mmap_* valid? */
	if (CHECK_FLAG (mbi->flags, 6))
	{
		memory_map_t *mmap;

		printf ("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
				(unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
		for (mmap = (memory_map_t *) mbi->mmap_addr;
				(unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
				mmap = (memory_map_t *) ((unsigned long) mmap
					+ mmap->size + sizeof (mmap->size)))
			printf (" size = 0x%x,     base_addr = 0x%#x%#x\n"
					"     type = 0x%x,  length    = 0x%#x%#x\n",
					(unsigned) mmap->size,
					(unsigned) mmap->base_addr_high,
					(unsigned) mmap->base_addr_low,
					(unsigned) mmap->type,
					(unsigned) mmap->length_high,
					(unsigned) mmap->length_low);
	}

	/* Construct an LDT entry in the GDT */
	{
		seg_desc_t the_ldt_desc;
		the_ldt_desc.granularity    = 0;
		the_ldt_desc.opsize         = 1;
		the_ldt_desc.reserved       = 0;
		the_ldt_desc.avail          = 0;
		the_ldt_desc.present        = 1;
		the_ldt_desc.dpl            = 0x0;
		the_ldt_desc.sys            = 0;
		the_ldt_desc.type           = 0x2;

		SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
		ldt_desc_ptr = the_ldt_desc;
		lldt(KERNEL_LDT);
	}

	/* Construct a TSS entry in the GDT */
	{
		seg_desc_t the_tss_desc;
		the_tss_desc.granularity    = 0;
		the_tss_desc.opsize         = 0;
		the_tss_desc.reserved       = 0;
		the_tss_desc.avail          = 0;
		the_tss_desc.seg_lim_19_16  = TSS_SIZE & 0x000F0000;
		the_tss_desc.present        = 1;
		the_tss_desc.dpl            = 0x0;
		the_tss_desc.sys            = 0;
		the_tss_desc.type           = 0x9;
		the_tss_desc.seg_lim_15_00  = TSS_SIZE & 0x0000FFFF;

		SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);

		tss_desc_ptr = the_tss_desc;

		tss.ldt_segment_selector = KERNEL_LDT;
		tss.ss0 = KERNEL_DS;
		tss.esp0 = 0x800000;
		ltr(KERNEL_TSS);
	}

	/* Set up IDT */
	idt_initialize();
	lidt(idt_desc_ptr);
	
	/* Init the PIC */
	i8259_init();

	/* Initialize devices, memory, filesystem, enable device interrupts on the
	 * PIC, any other initialization stuff... */
	enable_irq(KEY_IRQ);
	enable_irq(12);
	init_mouse();
	enable_irq(CAS_IRQ);

	/* Enable paging */
	init_paging();

	/* initial process info*/
	init_process();

	write_font();

	/* Initialize the terminal */
	init_terminal();

	/* Initialize the scheduler*/
	init_sche();

	/* Initialize RTC */
	enable_irq(RTC_IRQ);
	initial_rtc();

	/* Initialize scheduler */
	init_sche();																			

	/* Initialize the file system */
	init_ext2_sys();
	init_file_sys((uint32_t)file_system_addr);
	clear();


																	
	/* Enable interrupts */
	 // Do not enable the following until after you have set up your
	 // * IDT correctly otherwise QEMU will triple fault and simple close
	 // * without showing you any output 

	/* Initialize pit*/
	init_pit(PIT_TIME);
	music();

	/*kernel stack addr*/
	tss.esp0 = kernel_stack_address(PROCESS1_KERNEL_STACK);

    sti();
	/* Manually switch to the first terminal */ 
	switch_terminal(TERMINAL1);

	// while (1) {
	// 	ece391_execute((const uint8_t*)"shell"); 
	// }
	
	/* Spin (nicely, so we don't chew up cycles) */
	asm volatile(".1: hlt; jmp .1;");
}





	/** TESTCASES BEGIN **/

    //rtc test code uncomment it with test_interrupt() in rtc_handler() rtc.c, comment other part 
    //rtc_int_handle();   
/*
    int32_t fd, nbytes;
    int32_t status=rtc_open();
    if(status != 0) printf("ERROR: failed to open rtc\n");
    int32_t freq=64;
    void * buf = &freq;
    void * bufff;
    status = rtc_write(fd,buf,nbytes);
    if(status != 0) printf("ERROR: failed to write rtc\n");
    for (fd = 0; fd < 256; fd++) {
    	rtc_read(fd,bufff,nbytes);
    	printf(" %d\n", fd);
    }
    status = rtc_close();
     if(status != 0) printf("ERROR: failed to close rtc\n" );
*/	
	//*
	//	THESE ARE TESTS FOR FILE SYSTEM
	//	SET THE LAST ARGUMENT -1 IF WANT TO TEST FIND FILE BY NAME
	///
	
	
		//*TEST FOR READING DIRECTORY
		//test_read_directory();//test to test read directory
		
		//*TEST FOR READING TEXT FILE
		//test_read_file("anything",0,10000,1);  //test to read text file "frame1.txt"
		
		//*TEST FOR  NON TEXT FILE/
		//test_read_file("cat",0,20,-1);  //test to read non-text file
		
		//* TEST FOR READING A BIG FILE/
		//test_read_file("verylargetxtwithverylongname.tx",0,100000,-1);  //test to read big file
		
		//TEST FOR INVALID OFFSET/
		//printf("test for invalid OFFSET\n" );
		//test_read_file("syserr",10000,3,-1);  //test an invalid offset
		
		//TEST FOR INVALID FILE NAME//
		//printf("test for invalid filename\n" );
		//test_read_file("wtf",0,200,-1);  //test an invalid filename
		//
		//int r;
		//for(r=0;r<15;r++)
		//	test_read_file("",0,10,r);


	/* Terminal testcases */


/*
	{
		int32_t testterminali;
		uint8_t* testterminalopen;
		uint8_t testterminalbuf0[1024] = {"This is a test text.\n\n12345678!\n-----\n"};

		void* testterminalbuf = (void*)testterminalbuf0;

		open_terminal(testterminalopen);

		write_terminal(0, testterminalbuf, strlen(testterminalbuf));

		read_terminal(0, testterminalbuf, 12);

		for (testterminali = 0; testterminali < 10; testterminali++) {
			rtc_read(0, testterminalbuf, 12);
			write_terminal(0, testterminalbuf, strlen(testterminalbuf));
		}

		close_terminal(0);
	}
*/
	/** TESTCASES END **/



