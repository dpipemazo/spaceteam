/* 
 * File:   spaceteam_display.h
 * Author: dpipemazo
 *
 * Created on February 13, 2014, 10:18 PM
 */

#ifndef SPACETEAM_DISPLAY_H
#define	SPACETEAM_DISPLAY_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "spaceteam_game.h"

// Map the chip registers to easier names
#define DISPLAY_RS LATB.LATB0
#define DISPLAY_RW LATB.LATB1
#define DISPLAY_E  LATB.LATB2

// Mask off the low 3 bits of port B which are 
//	the bits which control the display signals
#define DISPLAY_CONTROL_MASK 0xFFF8

// constant commands for writing display parameters
#define DISPLAY_FUNCTION_SET_DATA 	0b00111000
#define DISPLAY_OFF_DATA			0b00001000
#define DISPLAY_CLEAR_DATA			0b00000001
#define DISPLAY_ENTRY_MODE_DATA		0b00000110
#define DISPLAY_ON_DATA 			0b00001100
#define DISPLAY_ADDRESS_DATA		0b10000000

// Beginning of lines of the display
#define DISPLAY_LINE_1_START		0x00
#define DISPLAY_LINE_2_START		0x40

// The maximum string length
#define DISP_MAX_STR_LEN			32

// The number of characters per line
#define DISP_CHARS_PER_LINE			16


// Definitions of constants for setting the display control
//	signals
#define RS_LOW  0x0000
#define RS_HIGH 0x0001
#define RW_LOW  0x0000
#define RW_HIGH 0x0002
#define E_LOW   0x0000
#define E_HIGH  0x0004

// Scrolling timer values
#define TIMER_4_ON 				0x0004
#define TIMER_4_POSTSCALE_16	0x0078
#define TIMER_4_PRESCALE_16		0x0003
#define TIMER_4_122Hz			0xFF
#define TIMER_4_INT_ENABLE 		IEC1bits.T4IE
#define TIMER_4_PRIORITY		IPC6bits.T4IP
#define TIMER_4_INT_FLAG		IFS1bits.T4IF
#define TIMER_4_INT_SCROLL		60				// How many timer 4s to wait
												//	to scroll the display

// Scrolling defines
#define SCROLL_ON				1
#define SCROLL_OFF				0

// Display line constants
#define DISPLAY_LINE_1			0
#define DISPLAY_LINE_2			1

// New line contants
#define NO_NEW_LINE				0
#define NEW_LINE				1


// Function declarations
void init_display(void);
void display_set_control_sigs(unsigned data);
void display_write_command(unsigned char data);
void display_write_char(unsigned char data);
void display_reset(void);
void display_set_address(unsigned char address);
void display_write_line(unsigned char line, char * str);
void display_clear(void);
void display_write_hex(unsigned data, unsigned char line);
void hex_to_string(unsigned data, char * out_str);
void display_line_buf(unsigned char line);
void init_timer_4(void);
void display_set_buffer(char * buf, unsigned char len, unsigned char val);
unsigned char display_copy_string(char * str, char * buf);
void display_scroll_set(unsigned char line, unsigned char setting);
void display_write_request(spaceteam_req_t req, unsigned char board, unsigned val);
void display_clear_line(unsigned char line);
void display_key_buf(char * buf);

#ifdef	__cplusplus
}
#endif

#endif	/* SPACETEAM_DISPLAY_H */