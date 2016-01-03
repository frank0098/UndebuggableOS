#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "lib.h"
#include "types.h"
#include "debug.h"

/** @brief Total terminal input buffer size */
#define TERMINAL_INPUT_BUFFER_SIZE 128

#define MAX_TERMINAL_NUMBER 3

/**
 * @brief      Structure for terminal information
 */

typedef struct terminal
{
	uint32_t terminal_index;
	int32_t pid;
	uint8_t input_buffer[TERMINAL_INPUT_BUFFER_SIZE];
	int32_t cursor_x;
	int32_t cursor_y;
	uint32_t buffer_top;
	volatile int32_t enter_pressed;
	volatile int32_t reading_terminal;

}terminal_t;

/** @brief Terminal information structure */
terminal_t terminal_info[MAX_TERMINAL_NUMBER];

/** @brief Program counts */
uint32_t program_count;  

/** @brief Current terminal id */

uint8_t current_tid;

/**
 * @brief      Switch the terminal.
 *
 * @param[in]  tid   The terminal to switch to
 */
void switch_terminal(uint8_t tid);

/**
 * @brief      Initialize the terminal.
 */
void init_terminal();

/**
 * @brief      Reads the input buffer from terminal.
 *
 * @param[in]  fd      Ignored
 * @param      buf     Read into this buffer
 * @param[in]  nbytes  Number of characters to read
 *
 * @return     The number of bytes read, or -1 on failure
 */
int32_t read_terminal(int32_t fd, void* buf, int32_t nbytes);

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
int32_t write_terminal(int32_t fd, const void* buf, int32_t nbytes);

/**
 * @brief      Initialize the terminal for read and write.
 *
 * @param[in]  filename  Ignored
 *
 * @return     0
 */
int32_t open_terminal(const uint8_t* filename);

/**
 * @brief      Close the terminal.
 * 
 * Literally does nothing.
 *
 * @param[in]  fd    Ignored
 *
 * @return     0
 */
int32_t close_terminal(int32_t fd);


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
int32_t input_char_terminal(uint8_t character);

/**
 * @brief      Clears the terminal.
 * 
 * This function is called by keyboard interrupt handler. It should never wait.
 */
void clear_terminal();

/**
 * @brief      Get a terminal buffer from tid.
 *
 * @param[in]  tid   tid of buffer to get
 *
 * @return     A pointer to tid's terminal buffer
 */
uint32_t* terminal_buffer(uint8_t tid);

/**
 * @brief      Get physical video memory from tid.
 *
 * @param[in]  tid   tid of video memory to get
 *
 * @return     A pointer to tid's video memory
 */
char* video_mem(uint8_t tid);


void terminal_mouse(uint32_t x, uint32_t y);


#endif /* _TERMINAL_H */
