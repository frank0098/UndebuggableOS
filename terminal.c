#include "terminal.h"
#include "keyboard.h"
#include "paging.h"
#include "scheduler.h"
#include "ece391syscall.h"
#include "process.h"
#include "vga_font.h"
//#include "gbk.h"

/**
 * @brief Video memory starting address
 * 
 * Copied from lib.c
 */
#define VIDEO 0xB8000

/**
 * @brief Number of terminal columns
 * 
 * Copied from lib.c
 */
#define NUM_COLS 80

/**
 * @brief Number of terminal rows
 * 
 * Copied from lib.c
 */
#define NUM_ROWS 24


/**
 * @brief Video memory attribute
 * 
 * Copied from lib.c
 */
#define ATTRIB 0x7

#define page_size 1024

#define INI_TID 0xff

#define EOF 0xff

#define column_num 80
 
#define LOW_PORT 0x0F
 
#define HIGH_PORT 0x0E
 
#define BASE_PORT 0x3D4
 
#define BASE_PORT_N 0x3D5
 
#define MASK_8 0xFF
 
/**
 * @brief Pointer to video memory
 * 
 * Copied from lib.c
 */
//static char* video_mem = (char*)VIRTUAL_VIDEO_MEM_BASE_ADDR; //(char *)VIDEO;

// static uint32_t terminal_buffer[MAX_TERMINAL_NUMBER][VIDEO_PAGE_SIZE];

/** @brief Terminal input buffer */
//static uint8_t input_buffer[TERMINAL_INPUT_BUFFER_SIZE];

/** @brief Cursor x coordinate */
//int32_t cursor_x = 0;

/** @brief Cursor y coordinate */
//int32_t cursor_y = 0;

/** @brief Top of input buffer */
//uint32_t buffer_top = 0;

/**
 * @brief Flag to indicate wether enter is pressed.
 * 
 * 1 if pressed, 0 if not pressed. 
 * Used to sync between read_termianl() and input_char_terminal().
 */
//volatile int32_t enter_pressed = 0;

/**
 * @brief Flag to indicate wether terminal is being read.
 * 
 * 1 if being read, 0 if not.
 * Used to sync between read_termianl() and input_char_terminal().
 */
//volatile int32_t reading_terminal = 0;


/* Function declarations */


/**
 * @brief      Push a character onto input buffer.
 *
 * @param[in]  character  The character to be pushed
 *
 * @return     0 on success, -1 on buffer overflow.
 */
int32_t buffer_push(uint8_t character, uint8_t tid);

/**
 * @brief      Pop a character from input buffer.
 *
 * @return     The character being poped. NUL (0x0) on buffer underflow.
 */
uint8_t buffer_pop(uint8_t tid);

/**
 * @brief      Scroll the screen.
 *
 * @param[in]  tid   The terminal to scroll
 */
void scroll(uint8_t tid);

/**
 * @brief      Print a character on screen.
 *
 * @param[in]  character  Character to be print
 * @param[in]  tid        The terminal to print
 *
 * @return     0 on success, -1 on failure
 */
int32_t print_char(uint8_t character, uint8_t tid);

/**
 * @brief      Draw a character at specified location.
 *
 * @param[in]  character  The character to be drawn
 * @param[in]  pos_x      X coordinate of the character
 * @param[in]  pos_y      Y coordinate of the character
 * @param[in]  tid        The terminal to draw
 *
 * @see        putc()
 */
void draw_char(uint8_t character, uint32_t pos_x, uint32_t pos_y, uint8_t tid);



/**
 * @brief      Move cursor to specified position.
 *
 *             Modified from http://wiki.osdev.org/Text_Mode_Cursor
 *
 * @param[in]  row   New y coordinate of cursor
 * @param[in]  col   New y coordinate of cursor
 * @param[in]  tid   The terminal to update
 */


void update_cursor(uint32_t row, uint32_t col, uint8_t tid);

void print_gbk(uint16_t character, uint8_t tid);

void draw_gbk(uint16_t character, uint32_t pos_x, uint32_t pos_y, uint8_t tid);

/* Function definitions */

/**
 * @brief      Initialize the terminal.
 * Inputs:none
 * Outputs:none
 */
void init_terminal() {
    /*Loop variable*/
    int i;
    /*Clear program count*/
    program_count = 0;
    /*Clear current tid*/
    current_tid = INI_TID;
    /*initialize three terminal*/
    for (i = 0; i < MAX_TERMINAL_NUMBER; i++) {
        terminal_info[i].pid = -1;
        terminal_info[i].cursor_x = 0;
        terminal_info[i].cursor_y = 0;
        terminal_info[i].buffer_top = 0;
        terminal_info[i].enter_pressed = 0;
        terminal_info[i].reading_terminal = 0;
    }
    clear_terminal();
}
/**
 * @brief      Switch the terminal to specified tid.
 *
 * @param[in]  tid   The terminal to switch to
 * Outputs: none
 */
void switch_terminal(uint8_t tid) {
    /*Loop variable and video page address*/ 
    uint32_t* video_page = (uint32_t*)VIDEO;
    uint32_t i;

    if (tid != current_tid && tid < MAX_TERMINAL_NUMBER) {

        // if any terminal exists
        if (current_tid != 0xFF) {
             for (i = 0; i < page_size; i++) {
                // copy current video memory to buffer
                terminal_buffer(current_tid)[i] = video_page[i];
                // copy new buffer to video memory
                video_page[i] = terminal_buffer(tid)[i];
            }           
        }

        // switch to new terminal
        current_tid = tid;

        // switch video page table
        update_video_pt();

        // update cursor
        update_cursor(terminal_info[current_tid].cursor_y, terminal_info[current_tid].cursor_x, current_tid);


        // start a new shell if current terminal is empty
        if (terminal_info[current_tid].pid < 0) {
            //printf("prog count %d\n",program_count );

            if(program_count < 6) {
                     cli();
                    current_process = NULL;
                    ece391_execute((const uint8_t*)"shell");

            }
        }      
    } 
}
/**
 * @brief      Reads the input buffer from the terminal.
 *
 * @param[in]  fd      Ignored
 * @param      buf     Read into this buffer
 * @param[in]  nbytes  Number of characters to read
 *
 * @return     The number of bytes read, or -1 on failure
 */
int32_t read_terminal(int32_t fd, void* buf, int32_t nbytes) {
     /*Fail to read the buffer*/
     if(buf == NULL)
        return -1;

    /*temp variables*/
    uint32_t i;
    int32_t bytes_to_read;
    /*find the current terminal to read*/
    terminal_t* current_terminal = &terminal_info[current_process->tid];
    /*Read fail*/
    if(current_terminal == NULL)
        return -1;

    // we need to disable interrupts whenever we update sync variables
    cli();
    /*start condition*/
    current_terminal->enter_pressed = 0;
    current_terminal->reading_terminal = 1;

    sti();
    
    // wait until enter is pressed by user
    while (current_terminal->enter_pressed != 1) {}

    cli();

    current_terminal->enter_pressed = 0;
    current_terminal->reading_terminal = 0;
    /*Handle buffer overflow*/
    bytes_to_read = (nbytes < TERMINAL_INPUT_BUFFER_SIZE) ? nbytes : TERMINAL_INPUT_BUFFER_SIZE;
    bytes_to_read = (bytes_to_read < current_terminal->buffer_top) ? bytes_to_read : current_terminal->buffer_top;
    /*Copy into buffer*/
    for (i = 0; i < bytes_to_read; i++) {
        ((uint8_t*)buf)[i] = current_terminal->input_buffer[i];
    }

    // add LF to the end
    ((uint8_t*)buf)[bytes_to_read] = ENTER_ascii;

    // terminate string if nbytes > 
    if (bytes_to_read < nbytes) {
        ((uint8_t*)buf)[bytes_to_read + 1] = '\0';
    }
    /*End reading condition*/
    current_terminal->buffer_top = 0;

    sti();

    return bytes_to_read + 1;
}
/**
 * @brief      Write a string to terminal.
 * 
 * Terminal should display the string on screen immediately.
 *
 * @param[in]  fd      Ignored
 * @param[in]  buf     The string to be written
 * @param[in]  nbytes  Size of the string
 *
 * @return     The number of bytes written, or -1 on failure
 */
int32_t write_terminal(int32_t fd, const void* buf, int32_t nbytes) {
    int32_t bytes_written = 0;/*Inilitize*/
    /*Write fail*/
    if(buf == NULL)
        return -1;
    cli();
    /*Keep writing untill the end of the buffer*/
    while (bytes_written < nbytes) {
        if (((uint8_t*)buf)[bytes_written] == EOF) {
            sti();
            return bytes_written;
        } else {
            /*Print the char to current terminal*/
            uint8_t char_to_point = ((uint8_t*)buf)[bytes_written];
            if (char_to_point >> 7) {
                uint16_t gbk_to_print = ((uint16_t)char_to_point << 8) + ((uint8_t*)buf)[bytes_written + 1];
                print_gbk(gbk_to_print, current_process->tid);
                bytes_written += 2;
            } else {
                print_char(((uint8_t*)buf)[bytes_written], current_process->tid);
                bytes_written++;
            }
        }
    }

    // should never reach here
    sti();
    return bytes_written;
}
/**
 * @brief      Initialize the terminal for read and write.
 *
 * @param[in]  filename  Ignored
 *
 * @return     0
 */
int32_t open_terminal(const uint8_t* filename) {   
    return 0;
}
/**
 * @brief      Close the terminal.
 * 
 * Literally does nothing.
 *
 * @param[in]  fd    Ignored
 *
 * @return     0
 */
int32_t close_terminal(int32_t fd) {
    return 0;
}
/**
 * @brief      Clears the terminal.
 * Inputs: none
 * Outputs: none
 * This function is called by keyboard interrupt handler. It should never wait.
 */
void clear_terminal() {
    /*update current terminal*/
    terminal_t* current_terminal = &terminal_info[current_tid];

    clear();
    /*Clear cursor and buffer*/
    current_terminal->cursor_x = 0;
    current_terminal->cursor_y = 0;
    current_terminal->buffer_top = 0;
    current_terminal->enter_pressed = 0;
    current_terminal->reading_terminal = 0;
    update_cursor(0, 0, current_tid);
}
/**
 * @brief      Write a character to terminal.
 * 
 * This function is called by keyboard interrupt handler. It should never wait.
 * 
 * Character goes into input buffer.
 *
 * @param[in]  character  ASCII value of the character to be written
 *
 * @return     0 on success, -1 on failure
 */
int32_t input_char_terminal(uint8_t character) {
    /*Update current terminal*/
    terminal_t* current_terminal = &terminal_info[current_tid];
    /*Handle edge condition*/
    if(current_terminal == NULL)
        return -1;
    /*Cases for character*/
    switch (character) {
        /*Pressed backspace*/
        case BACKSPACE_ascii:
            if (current_terminal->buffer_top == 0) {
                return -1;
            }
            /*Update cursor*/
            current_terminal->cursor_x--;
            if (current_terminal->cursor_x < 0) {
                current_terminal->cursor_x = NUM_COLS -1;
                current_terminal->cursor_y--;
                if (current_terminal->cursor_y < 0) {
                    current_terminal->cursor_x = 0;
                    current_terminal->cursor_y = 0;
                }
            }
            /*Update the buffer*/
            buffer_pop(current_tid);
            draw_char(' ', current_terminal->cursor_x, current_terminal->cursor_y, current_tid);
            update_cursor(current_terminal->cursor_y, current_terminal->cursor_x, current_tid);
            return 0;
            /*PRessed enter/newline*/
        case ENTER_ascii:
            current_terminal->cursor_x = 0;
            current_terminal->cursor_y++;
            /*Check scroll condition*/
            if (current_terminal->cursor_y >= NUM_ROWS) {
                scroll(current_tid);
                current_terminal->cursor_y = NUM_ROWS -1  ;
            }
            if (current_terminal->reading_terminal == 1) {
                current_terminal->enter_pressed = 1;
            } else {
                current_terminal->buffer_top = 0;
            }
            /*Update cursor*/
            update_cursor(current_terminal->cursor_y, current_terminal->cursor_x, current_tid);

            return 0;

        default:
            /*Handle buffer overflow*/
            if (buffer_push(character, current_tid) == -1) {
                return -1;
            }
            /*Print char*/
            return print_char(current_terminal->input_buffer[current_terminal->buffer_top - 1], current_tid);
    }
    
}
/**
 * @brief      Print a character on screen.
 *
 * @param[in]  character  Character to be print
 * @param[in]  tid        The terminal to print
 *
 * @return     0 on success, -1 on failure
 */
int32_t print_char(uint8_t character, uint8_t tid) {
    /*update current terminal*/
    terminal_t* current_terminal = &terminal_info[tid];
    if(current_terminal == NULL)
        return -1;
    /*Handle char*/
    switch (character) {
        case '\n':
        case '\r':
        /*Newline char*/
            current_terminal->cursor_x = 0;
            current_terminal->cursor_y++;
            if (current_terminal->cursor_y >= NUM_ROWS) {
                scroll(tid);
                /*Check scroll*/
                current_terminal->cursor_y = NUM_ROWS -1;
            }
            /*Update cursor*/
            update_cursor(current_terminal->cursor_y, current_terminal->cursor_x, tid);

            break;
        /*Most char case*/    
        default:
            /*Check scroll*/
            if ((current_terminal->cursor_y == NUM_ROWS -1 ) && (current_terminal->cursor_x == NUM_COLS -1)) {
                draw_char(character, current_terminal->cursor_x, current_terminal->cursor_y, tid);
                scroll(tid);
                current_terminal->cursor_x = 0;
            } else {
                /*Newline*/
                if (current_terminal->cursor_x >= NUM_COLS) {
                    current_terminal->cursor_x = 0;
                    current_terminal->cursor_y++;
                    if (current_terminal->cursor_y >= NUM_ROWS) {
                        scroll(tid);
                        current_terminal->cursor_y = NUM_ROWS -1;
                    }
                }
                /*Draw character*/
                draw_char(character, current_terminal->cursor_x, current_terminal->cursor_y, tid);
                current_terminal->cursor_x++;
            }
            /*Update cursor*/
            update_cursor(current_terminal->cursor_y, current_terminal->cursor_x, tid);
    }

    return 0;
}
/**
 * @brief      Scroll the screen.
 *
 * @param[in]  tid   The terminal to scroll
 */
void scroll(uint8_t tid) {
    uint32_t i;
    /*Change video memory to scroll*/
    for (i = 0; i < 2 * (NUM_ROWS -1) * NUM_COLS; i++) {
        *(uint8_t *)(video_mem(tid) + i) = *(uint8_t *)(video_mem(tid) + i + 2 * NUM_COLS);
    }
    /*Fill with spaces*/
    for (i = 0; i < NUM_COLS; i++) {
        draw_char(' ', i, NUM_ROWS -1 , tid);
    }
}
/**
 * @brief      Push a character onto input buffer.
 *
 * @param[in]  character  The character to be pushed
 *
 * @return     0 on success, -1 on buffer overflow.
 */
int32_t buffer_push(uint8_t character, uint8_t tid) {
    /*Update current terminal*/
    terminal_t* current_terminal = &terminal_info[tid];
    if(current_terminal == NULL)
        return -1;

    if (current_terminal->buffer_top >= TERMINAL_INPUT_BUFFER_SIZE) {
        return -1; // buffer overflow
    } else {
        // push char onto input buffer
        current_terminal->input_buffer[current_terminal->buffer_top] = character;
        current_terminal->buffer_top++;
        return 0;
    }
}
/**
 * @brief      Pop a character from input buffer.
 *
 * @return     The character being poped. NUL (0x0) on buffer underflow.
 */
uint8_t buffer_pop(uint8_t tid) {
    /*Update current terminal*/
    terminal_t* current_terminal = &terminal_info[tid];
    if(current_terminal == NULL)
        return -1;
    if (current_terminal->buffer_top == 0) {
        return 0; // buffer underflow
    } else {
        current_terminal->buffer_top--;
        return current_terminal->input_buffer[current_terminal->buffer_top];
    }
}
/**
 * @brief      Draw a character at specified location.
 *
 * @param[in]  character  The character to be drawn
 * @param[in]  pos_x      X coordinate of the character
 * @param[in]  pos_y      Y coordinate of the character
 * @param[in]  tid        The terminal to draw
 *
 * @see        putc()
 */
void draw_char(uint8_t character, uint32_t pos_x, uint32_t pos_y, uint8_t tid) {
    // copied from putc()
    *(uint8_t *)(video_mem(tid) + ((NUM_COLS*pos_y + pos_x) << 1)) = character;
    *(uint8_t *)(video_mem(tid) + ((NUM_COLS*pos_y + pos_x) << 1) + 1) = ATTRIB;
}
/**
 * @brief      Move cursor to specified position.
 *
 *             Modified from http://wiki.osdev.org/Text_Mode_Cursor
 *
 * @param[in]  row   New y coordinate of cursor
 * @param[in]  col   New y coordinate of cursor
 * @param[in]  tid   The terminal to update
 */

void update_cursor(uint32_t row, uint32_t col, uint8_t tid) {
    if (tid == current_tid) {
        if ((row > NUM_ROWS) || (col > NUM_COLS)) {
            return;
        }

        uint16_t position = (row * column_num) + col;
     
        // cursor LOW port to vga INDEX register
        outb(LOW_PORT, BASE_PORT);
        outb((uint8_t)(position & MASK_8), BASE_PORT_N);
        // cursor HIGH port to vga INDEX register
        outb(HIGH_PORT, BASE_PORT);
        outb((uint8_t)((position >> 8) & MASK_8), BASE_PORT_N);        
    }
}
/**
 * @brief      Get a terminal buffer from tid.
 *
 * @param[in]  tid   tid of buffer to get
 *
 * @return     A pointer to tid's terminal buffer
 */
uint32_t* terminal_buffer(uint8_t tid) {
    return (uint32_t*)(VIDEO + (tid+1) * VIDEO_PAGE_SIZE);
}
/**
 * @brief      Get physical video memory from tid.
 *
 * @param[in]  tid   tid of video memory to get
 *
 * @return     A pointer to tid's video memory
 */
char* video_mem(uint8_t tid) {
    return (char*)(tid == current_tid ? VIDEO : (uint32_t)terminal_buffer(tid));
}

void print_gbk(uint16_t character, uint8_t tid) {
    /*update current terminal*/
    terminal_t* current_terminal = &terminal_info[tid];
    if(current_terminal == NULL) {
        return;
    }
    /*Handle char*/
    /*Check scroll*/
    if ((current_terminal->cursor_y == NUM_ROWS -1 ) && (current_terminal->cursor_x == NUM_COLS -2)) {
        draw_gbk(character, current_terminal->cursor_x, current_terminal->cursor_y, tid);
        scroll(tid);
        current_terminal->cursor_x = 0;
    } else {
        /*Newline*/
        if (current_terminal->cursor_x >= NUM_COLS - 1) {
            current_terminal->cursor_x = 0;
            current_terminal->cursor_y++;
            if (current_terminal->cursor_y >= NUM_ROWS) {
                scroll(tid);
                current_terminal->cursor_y = NUM_ROWS -1;
            }
        }
        /*Draw character*/
        draw_gbk(character, current_terminal->cursor_x, current_terminal->cursor_y, tid);
        current_terminal->cursor_x += 2;
    }
    /*Update cursor*/
    update_cursor(current_terminal->cursor_y, current_terminal->cursor_x, tid);
}

void draw_gbk(uint16_t character, uint32_t pos_x, uint32_t pos_y, uint8_t tid) {
    uint8_t char_to_draw = set_font(character);
    // modified from putc()
    *(uint8_t *)(video_mem(tid) + ((NUM_COLS*pos_y + pos_x) << 1)) = char_to_draw;
    *(uint8_t *)(video_mem(tid) + ((NUM_COLS*pos_y + pos_x) << 1) + 1) = ATTRIB;
    *(uint8_t *)(video_mem(tid) + ((NUM_COLS*pos_y + pos_x + 1) << 1)) = char_to_draw + 1;
    *(uint8_t *)(video_mem(tid) + ((NUM_COLS*pos_y + pos_x + 1) << 1) + 1) = ATTRIB;
}

void terminal_mouse(uint32_t x, uint32_t y)
{
    update_cursor(y,x,current_tid);
}
