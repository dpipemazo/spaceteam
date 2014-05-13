/*
        Modifications by Elia Ritterbusch
        http://eliaselectronics.com

        Copyright (c) 2011 by Ernst Buchmann

	Code based on the work of Stefan Engelke and Brennan Ball

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy,
    modify, merge, publish, distribute, sublicense, and/or sell copies
    of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.


*/

#ifndef _WL_MODULE_H_
#define _WL_MODULE_H_

#include "xc.h"
#include "spi.h"
#include "nrf24l01.h"
#include "spaceteam_general.h"
#include "spaceteam_msg.h"

// WL-Module settings
#define wl_module_CH			2
#define wl_module_PAYLOAD_LEN	(sizeof(spaceteam_packet_t))
#define wl_module_RF_DR_HIGH	0		//0 = 1Mbps, 1 = 2Mpbs
#define wl_module_RF_SETUP		(RF_SETUP_RF_PWR_0 | RF_SETUP_RF_DR_250)
#define wl_module_CONFIG		( (1<<EN_CRC) | (1<<CRCO) )
#define wl_module_TX_NR_0		0
#define wl_module_TX_NR_1		1
#define wl_module_TX_NR_2		2
#define wl_module_TX_NR_3		3
#define wl_module_TX_NR_4		4
#define wl_module_TX_NR_5		5

#define wl_module_ADDR_LEN      5

// Pin definitions for chip select and chip enabled of the wl-module
#define wl_module_CE    LATBbits.LATB15 // RA6
#define wl_module_CSN   LATAbits.LATA7 // RA7

// Definitions for selecting and enabling wl_module module
#define wl_module_CSN_hi     wl_module_CSN = 1;
#define wl_module_CSN_lo     wl_module_CSN = 0;
#define wl_module_CE_hi      wl_module_CE = 1;
#define wl_module_CE_lo      wl_module_CE = 0;

// Defines for setting the wl_module `s for transmitting or receiving mode
#define TX_POWERUP wl_module_write_register_byte(CONFIG, wl_module_CONFIG | ( (1<<PWR_UP) | (0<<PRIM_RX) ) )
#define RX_POWERUP wl_module_write_register_byte(CONFIG, wl_module_CONFIG | ( (1<<PWR_UP) | (1<<PRIM_RX) ) )

// Function declarations
void init_wireless(void);
void wl_module_set_address(unsigned char * address);
unsigned char wl_module_get_status(void);
void wl_module_send_command(unsigned char command, unsigned char * datain, unsigned char * dataout, unsigned char data_len);
void wl_module_read_register(unsigned char reg, unsigned char * value, unsigned char len);
unsigned char wl_module_read_register_byte(unsigned char reg);
void wl_module_write_register(unsigned char reg, unsigned char * value, unsigned char len);
void wl_module_write_register_byte(unsigned char reg, unsigned char value);
void wl_module_get_payload(unsigned char * pload);
void wl_module_send_payload(unsigned char * pload, spaceteam_player_t player);
void wl_module_start_transmit(void);
void wl_module_send_ack(unsigned char * pload);

#endif /* _WL_MODULE_H_ */
