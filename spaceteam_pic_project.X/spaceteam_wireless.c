// This file contains the functionality used to implement the
//	wireless communication of the spacteam board
#include "spi.h"
#include "xc.h"


void init_wireless(void)
{
	// Set the chip select high
	WIRLESS_CS = 1;
	// Set txrx low
	WIRELESS_CE = 0;

	// Set up the interrupt to go off of data coming in

	// TODO

}

// This function writes to a register on the 
void rfid_write_reg(unsigned short reg, unsigned short value)
{

}