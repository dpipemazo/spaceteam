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


// Definitions of constants for setting the display control
//	signals
#define RS_LOW  0x0000
#define RS_HIGH 0x0001
#define RW_LOW  0x0000
#define RW_HIGH 0x0002
#define E_LOW   0x0000
#define E_HIGH  0x0004


// Function declarations
void init_display(void);
void display_set_control_sigs(unsigned data);
void display_write_command(unsigned char data);
void display_write_char(unsigned char data);
void display_reset(void);
void display_write_string(char *str_to_write);
void display_set_address(unsigned char address);
void display_write_line(unsigned char line, char * str);
void display_clear(void);
void display_write_hex(unsigned data, unsigned char line, unsigned char idx);
void hex_to_string(unsigned data, char * out_str);
void display_write_debug(char * data, unsigned char line, unsigned char len);

#ifdef	__cplusplus
}
#endif

#endif	/* SPACETEAM_DISPLAY_H */