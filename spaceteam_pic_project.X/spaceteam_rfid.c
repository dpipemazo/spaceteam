// This file is for working with the RFID module.
// This code is largely based off of the code that can be 
//	found at: http://www.onemansanthology.com//arduino/rfid-arduino-micro-via-spi.pde
//	but hopefully simplified some. 

#include "xc.h"
#include "spaceteam_rfid.h"
#include "spaceteam_spi.h"

#define FCY 8000000UL
#include <libpic30.h>

// This function will initialize the RFID module
void init_rfid(void)
{
	unsigned short temp_val;

	// Make sure that SPI is initialized
	if (!is_spi_initialized())
	{
		init_spi();
	}

	// Set our CS and the wireless CS high
	RFID_CS = 1;

	__delay_us(1);


	// Perform a soft reset of the RFID MODULE
	rfid_write_reg(RFID_COMMAND_REG, RFID_SOFTRESET);

	// Give it a second to init. Not sure about this
	__delay_ms(1000);

	// Need to set up the timers aparently, this code is pretty much copied
	// from http://www.onemansanthology.com//arduino/rfid-arduino-micro-via-spi.pde
	rfid_write_reg(RFID_TMODE_REG, 0x8D);
	rfid_write_reg(RFID_TPRESCALER_REG, 0x3E);
	rfid_write_reg(RFID_TRELOADL_REG, 30);
	rfid_write_reg(RFID_TRELOADH_REG, 0);

	// Other TX stuff
	rfid_write_reg(RFID_TXAUTO_REG, 0x40);
	rfid_write_reg(RFID_MODE_REG, 0x3D);

	// Turn on the antenna, if it is not already on. 
	temp_val = rfid_read_reg(RFID_TXCONTROL_REG);
	if (!(temp_val & 0x03))
	{
		temp_val = (temp_val | 0x03);
		rfid_write_reg(RFID_TXCONTROL_REG, temp_val);

	}

}

// This function writes a command to the RC522 module
void rfid_write_reg(unsigned short addr, unsigned short data)
{
	// Need to set the chip select low
	RFID_CS = 0;

	// Write the address and data
	spi_write((addr << 1) & (~RFID_READ_MASK));
	spi_write(data);

	// Now pull the chip select back high
	RFID_CS = 1;

	// Give it a few clocks to propogate... the chip selects don't always update instantly.
	__delay_us(1);
}

// This function reads a register from the RC522 module
unsigned short rfid_read_reg(unsigned short addr)
{
	unsigned short ret_val;

	// Need to set the chip select low
	RFID_CS = 0;

	// Issue the read command
	spi_write((addr << 1) | RFID_READ_MASK);
	// The data returned on the next write will be the value desired
	ret_val = spi_write(0);

	// Pull the chip select back high
	RFID_CS = 1;

	return ret_val;
}

// This function sets a bit in the given register by 
//	reading the register and then OR-ing the register
//	value with the bitmask, thus setting all bits 
//	set in the bitmask
void rfid_set_bits(unsigned short reg, unsigned short mask)
{
	unsigned short temp_val;

	temp_val = rfid_read_reg(reg);
	rfid_write_reg(reg, (temp_val | mask));
}

// This function clears a bit in the given register
//	by reading in the register and then ANDing the 
//	register with the logical not of the bitmask, 
//	thus zeroing all bits set in the mask
void rfid_clear_bits(unsigned short reg, unsigned short mask)
{
	unsigned short temp_val;

	temp_val = rfid_read_reg(reg);
	rfid_write_reg(reg, (temp_val & (~mask)));
}

// This function performs the transcieve transaction with the RFID card. It takes 
//	a command to perform, a buffer of input data, a length of the input data buffer, 
//	and output data buffer of size MAX_LEN and an output pointer to the length of 
//	data in the buffer.
rfid_status_t rfid_transcieve(unsigned short * datain, unsigned short datain_len, unsigned short * dataout, unsigned short * dataout_len)
{
	unsigned short irq_reg = 0;
	unsigned short num_bytes = 0;
	unsigned short num_bits = 0;
	int i;
	rfid_status_t status = RFID_ERROR;

	// Want to clear the interrupt bits
	rfid_clear_bits(RFID_IRQ_REG, 0x80);

	// Want to flush the FIFO buffer
	rfid_write_reg(RFID_FIFO_LEVEL_REG, RFID_CLEAR_FIFO);

	// Return the RFID card to the IDLE state
	rfid_write_reg(RFID_COMMAND_REG, RFID_IDLE);

	//
	// Now, do the transcieving of information
	//

	// Write the input data to the FIFO
	for (i = 0; i < datain_len; i++)
	{
		rfid_write_reg(RFID_FIFO_DATA_REG, datain[i]);
	}

	// Tell the controller to do the transcieve
	rfid_write_reg(RFID_COMMAND_REG, RFID_TRANSCIEVE);

	// And start the transcieve by writing to the bitframing register
	rfid_set_bits(RFID_BITFRAMING_REG, 0x80);

	//
	// Now, wait until the transmission is done, or we have a timeout. 
	//	the timeout is a simple counter of reads on the SPI connection
	//
	i = RFID_TIMEOUT_CYCLES;
	while ( ((irq_reg & IRQ_WAIT_MASK) == 0) && (i > 0))
	{
		irq_reg = rfid_read_reg(RFID_IRQ_REG);
		i--;
	}

	// Want to clear the StartSend bit
	rfid_clear_bits(RFID_BITFRAMING_REG, 0x80);


	//
	// Read back the value, if we didn't have a timeout
	//
	if (i != 0)
	{
		status = RFID_SUCCESS;

		// Get the number of bytes in the FIFO
		num_bytes = rfid_read_reg(RFID_FIFO_LEVEL_REG);
		// Get the number of bits in the last word from the FIFO
		num_bits  = rfid_read_reg(RFID_CONTROL_REG);
		num_bits = (num_bits & 0x07);

		// The total number of bits is num_bytes*8 if there are 
		//	8 bits in the final word, else (num_bytes - 1)*8 + num_bits
		if (num_bits == 0)
		{
			num_bits = 8*num_bytes;
		}
		else
		{
			num_bits += 8*(num_bytes -1);
		}

		// Write the number of bits to the output parameter
		*dataout_len = num_bits;

		// Finally, read the data out of the fifo data register
		//	and write it into the output buffer
		for (i = 0; i < num_bytes; i++)
		{
			dataout[i] = rfid_read_reg(RFID_FIFO_DATA_REG);
		}
	}
	// If we did have a timeout, note it
	else
	{
		status = RFID_TIMEOUT;
		*dataout_len = 0;
	}

	return status;

}

// This function sends a request to an RFID card, and will 
//	return the card's MIFARE type in the data array. Data array
//	must be DATA_MAX_LEN bytes
rfid_status_t rfid_request_type(unsigned short *data)
{
	// Data array which will be used for I/O
	unsigned short data_len;
	rfid_status_t status;

	// Want to perform a write to the bit framing register, 
	//	to set up something
	rfid_write_reg(RFID_BITFRAMING_REG, 0x07);

	// Want to put our request type in the beginning of the data array
	data[0] = RFID_REQ_IDL;

	// And do the transcieve, with the input data array and 
	//	one byte of transmit data 
	status = rfid_transcieve(data, 1, data, &data_len);

	// Make sure we got 2 bytes back, since that's what we
	//	are looking for
	if ( (status == RFID_SUCCESS) && (data_len != 0x10) )
	{
		status = RFID_ERROR;
	}

	// Return the number of bits received
	return status;
}

// This function sends a request to an RFID card, and will 
//	return the card's serial identifier in the data array. Data array
//	must be DATA_MAX_LEN bytes
rfid_status_t rfid_request_id(unsigned short *data)
{
	// Data array which will be used for I/O
	unsigned short data_len;
	rfid_status_t status;
	int i;
	unsigned short xor_check = 0;

	// Want to perform a write to the bit framing register, 
	//	to set up transmit of all of the bits of the
	//	last byte that wil be sent.
	rfid_write_reg(RFID_BITFRAMING_REG, 0x00);

	// Want to put our request type in the beginning of the data array
	data[0] = RFID_IDREQ;
	data[1] = 0x20;	// No idea what this does

	// And do the transcieve, with the input data array and 
	//	one byte of transmit data 
	status = rfid_transcieve(data, 2, data, &data_len);

	// Make sure we got a valid ID back. We are expecting five bytes
	//	in the array. This first four are the ID, and the fifth
	//	is the XOR of the ID bytes. Perform this check
	if (status == RFID_SUCCESS)
	{
		// If we got the right amount of data
		if (data_len == 0x28)
		{
			for (i = 0; i < 4; i++)
			{
				xor_check ^= data[i];
			}
			if( xor_check != data[i] )
			{
				status = RFID_ERROR;
			}
		}
		// If we did not get the right amount of data
		else
		{
			status = RFID_ERROR;
		}
		
	}

	// Return the number of bits received
	return status;
}














