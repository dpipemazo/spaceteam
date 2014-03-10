/* 
 * File:   spaceteam_io.h
 * Author: dpipemazo
 *
 * Created on February 13, 2014, 11:05 PM
 */

#ifndef SPACETEAM_IO_H
#define	SPACETEAM_IO_H

#ifdef	__cplusplus
extern "C" {
#endif

//
// Defines for TRISA and TRISB I/O registers
//

// PORT A 
#define IOIN_INPUT 			0b00010000 	// IOin is RA4
#define WIRELESS_IRQ_INPUT	0b01000000	// IRQ is RA6

// PORT B
#define SPI_MISO_INPUT			0b0000010000000000
#define KNOB_INPUT				0b0100000000000000
#define TRISB_DEBUG				0b0000000001100000


//
// Defines for LATA and LATB registers
//

#define ISEL1_LAT 			LATAbits.LATA0
#define ISEL2_LAT			LATAbits.LATA1
#define ISEL3_LAT			LATAbits.LATA2
#define ISEL4_LAT			LATAbits.LATA3

#define WIRELESS_TXRX_LAT	LATAbits.LATA6
#define WIRELESS_CS_LAT		LATAbits.LATA7

#define DISPLAY_RS_LAT		LATBbits.LATB0
#define DISPLAY_RW_LAT		LATBbits.LATB1
#define DISPLAY_E_LAT		LATBbits.LATB2

#define LSEL1_LAT			LATBbits.LATB3
#define LSEL2_LAT			LATBbits.LATB4
#define LSEL3_LAT			LATBbits.LATB5
#define LSEL4_LAT			LATBbits.LATB6

#define KSEL1_LAT			LATBbits.LATB7
#define KSEL2_LAT			LATBbits.LATB8
#define KSEL3_LAT			LATBbits.LATB9

#define SPI_SCK_LAT			LATBbits.LATB11
#define SPI_MISO_LAT		LATBbits.LATB13

#define RFID_CS_LAT			LATBbits.LATB12

//
// Defines for port bits
//
#define IOIN_PORT			PORTAbits.RA4
#define SPI_MISO_PORT   	PORTBbits.RB10
#define KNOB_PORT			PORTBbits.RB14
#define WIRELESS_IRQ_PORT 	PORTBbits.RB15

// Masks
#define ISEL_MASK 0xF0
#define LSEL_MASK 0xFF87

// Shifts
#define LSEL_SHIFT 3

//
// Define the TRISA and TRISB configuration words
//
#define TRISAVAL ( IOIN_INPUT | WIRELESS_IRQ_INPUT )

// Update this when not debugging
#define TRISBVAL ( TRISB_DEBUG | SPI_MISO_INPUT | KNOB_INPUT )


// Kepyad constants
#define KEYPAD_MASK 	0x0380 // Bits 9, 8 and 7
#define COL_DRIVE_MASK 	0x0080 // Bit 7
#define DEBOUNCE_TIME   500   // Pretty arbitrary, but the amount of time to debounce for
#define AUTO_REP_TIME	5000
#define NUM_KEYS		12
#define NO_KEY			0xFFFF
#define RUN_CODE 		10
#define CLR_CODE		11

//
// Function Declarations
//

void init_io(void);
int is_io_initialized(void);
int set_isel(unsigned char val);
int set_lsel(unsigned val);
unsigned get_iomux(void);

void init_keypad(void);
unsigned char scan_and_debounce_keypad(void);
unsigned get_knob_sample(void);


#ifdef	__cplusplus
}
#endif

#endif	/* SPACETEAM_IO_H */

