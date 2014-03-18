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
#include "xc.h"
#include "spaceteam_wireless.h"
#include "nRF24L01.h"
#include "spaceteam_spi.h"
#include "spaceteam_display.h"
#include "spaceteam_rfid.h"
#include "spaceteam_io.h"
#include "spaceteam_msg.h"
#include "spaceteam_general.h"
#include <stddef.h>

#define FCY 8000000UL
#include <libpic30.h>

// Flag which denotes transmitting mode
volatile unsigned char PTX;

unsigned char player_addresses[MAX_NUM_PLAYERS][wl_module_ADDR_LEN] =
{
	{0x12, 0x34, 0x56, 0x78, 0x9A}, // Player 0
	{0x34, 0x56, 0x78, 0x9A, 0xBC}, // Player 1
	{0x56, 0x78, 0x9A, 0xBC, 0xDE}, // Player 2
	{0x78, 0x9A, 0xBC, 0xDE, 0xF0}, // Player 3
	{0x9A, 0xBC, 0xDE, 0xF0, 0x12}, // Player 4
	{0xBC, 0xDE, 0xF0, 0x12, 0x34}, // Player 5
	{0xDE, 0xF0, 0x12, 0x34, 0x45}, // Player 6
	{0xF0, 0x12, 0x34, 0x45, 0x67} 	// Player 7
};


// This function returns a pointer to a player's address
//	data struct based on the player number
unsigned char * get_player_address(unsigned char player_no)
{
	return &player_addresses[player_no][0];
}

//
// This function fully resets and initializes the wireless
//	module and sets it in PRX or PTX mode according to 
//	whether it is a master or a slave
//
void init_wireless()
{
    // Define CSN and CE as Output and set them to default
    wl_module_CE_lo;
    wl_module_CSN_hi;

    // Initialize spi module
    if (!is_spi_initialized())
    {
    	init_spi();
    }

    // Clear all pending interrupts for the wireless controller
    wl_module_write_register_byte(STATUS, 0x70);

    // Set up the chip address
    wl_module_set_address(get_player_address(BOARDNUM));

    // Set RF channel
    wl_module_write_register_byte(RF_CH, wl_module_CH);
	// Set data speed & Output Power configured in wl_module.h
	wl_module_write_register_byte(RF_SETUP, wl_module_RF_SETUP);

	// Clear the shared variable for monitoring retries
	PTX = 0;

    // 
    // Set up interrupts on the PIC 
    //
    IPC7bits.INT2IP = 7;	// Set interrupt priority to highest (7)
    IFS1bits.INT2IF = 0;	// Clear the interrupt flag, if it was set.
    INTCON2bits.INT2EP = 1; // Falling edge
    IEC1bits.INT2IE = 1;	// Enable interrupt 2

   	//
	// Either start the transmitter or receiver, based on master/slave 
	//
	#if (BOARDNUM == MASTER_BOARDNUM)
		// Setup retries
		wl_module_write_register_byte(SETUP_RETR, (SETUP_RETR_ARD_750 | SETUP_RETR_ARC_15));
		// Setup dynamic payload length 
		wl_module_write_register_byte(FEATURE, 0x06);
		// And enable it on pipe 0
		wl_module_write_register_byte(DYNPD, 0x01);

	#else
		// Set length of incoming payload
    	wl_module_write_register_byte(RX_PW_P0, SPACETEAM_PACKET_SIZE);
		// Setup dynamic payload length and ack payloads
		wl_module_write_register_byte(FEATURE, 0x06);
		// And enable it on pipe 0
		wl_module_write_register_byte(DYNPD, 0x01);
		// Want to start up the receiver
		RX_POWERUP;
		// And send the chip enable high to begin listening for packets
		wl_module_CE_hi;
	#endif

	// Give it a bit to power up
	__delay_ms(50);
}

// Set the TX and RX address for the module on data pipe 0
void wl_module_set_address(unsigned char * address)
{

	// Write the RX address to pipe 0
	wl_module_write_register(RX_ADDR_P0, address, wl_module_ADDR_LEN);

	// Only configure the TX address if we are a wireless master, otherwise
	//	it does bad things
	#if (BOARDNUM == MASTER_BOARDNUM)
		// And write the transmit address
		wl_module_write_register(TX_ADDR, address, wl_module_ADDR_LEN);
	#endif

}

//return the value of the status register
unsigned char wl_module_get_status(void)
{
	unsigned char status;

	// Set the chip select low
	wl_module_CSN_lo

	// Get the status byte
	status = spi_write(NOOP);

	// Pull the chip select back high
	wl_module_CSN_hi

	// And return the status
	return status;
}

// Send a command to the wireless module. The command may also require sending/
//	reading a number of bytes after the initial byte. If this is not the case, 
//	make the length 0
void wl_module_send_command(unsigned char command, unsigned char * datain, unsigned char * dataout, unsigned char data_len)
{
	// Send the chip select low
	wl_module_CSN_lo;
	// Write the command byte
	spi_write(command);

	// this function will read/write what we want. And it has error-checking
	//	for NULL pointers and such so we are OK. 
	spi_write_multiple(datain, dataout, data_len);

	// Pull the chip select back high
	wl_module_CSN_hi;
}

// Read a number of bytes from a wireless register
void wl_module_read_register(unsigned char reg, unsigned char * value, unsigned char len)
// Reads an array of bytes from the given start position in the wl-module registers.
{
	// Send the command to read the bytes
    wl_module_send_command( (R_REGISTER | (REGISTER_MASK & reg)), NULL, value, len );
}

// Read just one byte from a register
unsigned char wl_module_read_register_byte(unsigned char reg)
// Reads an array of bytes from the given start position in the wl-module registers.
{
	unsigned char reg_val;

	// Send the command to read the bytes
    wl_module_read_register(reg, &reg_val, 1 );

    return reg_val;
}

// Write a number of bytes to a wireless register
void wl_module_write_register(unsigned char reg, unsigned char * value, unsigned char len)
// Writes an array of bytes into inte the wl-module registers.
{
	// Send the command to write the bytes
    wl_module_send_command( (W_REGISTER | (REGISTER_MASK & reg)), value, NULL, len );
}

// Write just one byte to a register
void wl_module_write_register_byte(unsigned char reg, unsigned char value)
{
	unsigned char reg_val;

	reg_val = value;
	wl_module_write_register(reg, &reg_val, 1);
}

// Read the payload from the RX FIFO
void wl_module_get_payload(unsigned char * pload)
// Reads pload_len bytes from the RX fifo into the data array
{
    // Send the command to read the RX payload
    wl_module_send_command( R_RX_PAYLOAD, NULL, pload , SPACETEAM_PACKET_SIZE );

}

// Send the payload out
void wl_module_send_payload(unsigned char * pload)
// Sends a data package to the default address. Be sure to send the correct
// amount of bytes as configured as payload on the receiver.
{
    while (PTX) {}                  	// Wait until last packet is sent

    wl_module_CE_lo;					// Send the chip enable low

    PTX = 1;                        	// Indicate that we are trying to send a packet
    TX_POWERUP;                     	// Power up

    // Flush the TX FIFO
    wl_module_send_command(FLUSH_TX, NULL, NULL, 0);

    // SEnd the command to write the payload
    wl_module_send_command(W_TX_PAYLOAD, pload, NULL, SPACETEAM_PACKET_SIZE);

    // And start the transmit
    wl_module_start_transmit();

}

// Send a response payload to the master on pipe 0. Writes it to the TX 
//	ACK FIFO.
void wl_module_send_response(unsigned char * pload)
{
	wl_module_send_command(W_ACK_PAYLOAD, pload, NULL, SPACETEAM_PACKET_SIZE);
}

// This function toggles the CE line for 10us to begin a transmit
void wl_module_start_transmit(void)
{
	wl_module_CE_hi;
    __delay_us(10);						
    wl_module_CE_lo;	
}

// The wireless module interrupt handler, based of INT2
void _ISR _INT2Interrupt(void)
{
	unsigned char status;
	unsigned char rfid_cs_val;
	spaceteam_packet_t payload;

	// Need to set chip selects
	int_set_chip_selects();

    // Read wl_module status
    status = wl_module_get_status();


    if (status & (1<<TX_DS)){ // IRQ: Package has been sent
    	display_write_line(DISPLAY_LINE_1, "PACKET SENT");
	    wl_module_write_register_byte(STATUS, (1<<TX_DS)); //Clear Interrupt Bit
	    PTX=0;
    }

	if (status & (1<<MAX_RT)){ // IRQ: Package has not been sent, send again
		display_write_line(DISPLAY_LINE_1, "PACKET NOT SENT");
		wl_module_write_register_byte(STATUS, (1<<MAX_RT));	// Clear Interrupt Bit
		// We only want to retry the send if we know that it's to a module
		//	that exists. So only retry sending if the game state is not waiting
		if (get_game_state() != GAME_WAITING)
		{
			// Try to resend it, since we suppose it's going to someone who exists.
			wl_module_start_transmit();
		}
		// Otherwise we need to reset PTX
		else
		{
			PTX = 0;
		}
	}

	if (status & (1<<RX_DR)){
		display_write_line(DISPLAY_LINE_1, "GOT PACKET");
		wl_module_get_payload(&payload); // And get the data
		// Parse the message that we got

		wl_module_write_register_byte(STATUS, (1<<RX_DR)); //Clear Interrupt Bit

	}

    // reset INT2 flag
    IFS1bits.INT2IF = 0;

    // And need to reset chip selects
    int_reset_chip_selects();
}




