//
// This function controls the spaceteam display
//

#include "xc.h"
#include <stddef.h>
#include "spaceteam_spi.h"
#include "spaceteam_io.h"
#include "spaceteam_display.h"

#define FCY 8000000UL
#include <libpic30.h>

// This function initializes the display
void init_display(void)
{
	// If SPI has not yet been initialized, we need to do so
	if (!is_spi_initialized())
	{
		init_spi();
	}

	/// If the I/O has not yet been initialized, we need to do so
	if (!is_io_initialized())
	{
		init_io();
	}

	// Now, reset the display and we are done
	display_reset();

}

// This function sets the display control signals 
void display_set_control_sigs(unsigned data)
{
	unsigned new_portb_val;

	// Get the previous value of PORTB
	new_portb_val = PORTB;
	// Clear the display bits of the old value of portB, AND away all but the display bits 
	//	of the data input, and or the data input into the portB value
	new_portb_val = ((new_portb_val & DISPLAY_CONTROL_MASK) | (data & (~DISPLAY_CONTROL_MASK)));
	// Write the new value to portb;
	LATB = new_portb_val;
}

// This function writes a control command to the display
void display_write_command(unsigned short data)
{

	// Write the data to the shift register over the SPI connection. 
	//	NOTE: The SPI clock is running at 1/2 of the system clock, 
	//	so it will take 2*8 system clocks for the data to be valid, 
	//	which should work out
	spi_write(data);

	// Need to set up the control signals to begin the write
	display_set_control_sigs(RS_LOW | RW_LOW | E_LOW);

	// Wait a clock (need to wait 40 ns)
	// Nop();

	// Now pull E high in preparation for the write
	display_set_control_sigs(RS_LOW | RW_LOW | E_HIGH);

	// Wait a few clocks (need to wait a minimum of 230 ns)
	// Nop();
	// Nop();
	// Nop();

	// The data should be valid by this point, can insert a few more Nops if it is not
	//	so we can drop E and call it a day
	display_set_control_sigs(RS_LOW | RW_LOW | E_LOW);

	// Display writes take up to 37 us to complete
	__delay_us(37);

	return;
}

// This function writes a data byte to the display. It currently does nothing
//	about the display cursor position. Eventually we will get fancier functions
//	for this kind of thing
void display_write_char(unsigned short data)
{
	// Write the data to the shift register over the SPI connection. 
	//	NOTE: The SPI clock is running at 1/2 of the system clock, 
	//	so it will take 2*8 system clocks for the data to be valid, 
	//	which should work out
	spi_write(data);

	// Need to set up the control signals to begin the write
	display_set_control_sigs(RS_HIGH | RW_LOW | E_LOW);

	// Wait a clock (need to wait 40 ns)
	// Nop();

	// Now pull E high in preparation for the write
	display_set_control_sigs(RS_HIGH | RW_LOW | E_HIGH);

	// Wait a few clocks (need to wait a minimum of 230 ns)
	// Nop();
	// Nop();
	// Nop();

	// The data should be valid by this point, can insert a few more Nops if it is not
	//	so we can drop E and call it a day
	display_set_control_sigs(RS_HIGH | RW_LOW | E_LOW);

	// Display writes take up to 37 us to complete
	__delay_us(37);

	return;
}

// This function converts a hexadecimal value to a 4-character string, 
//	null-terminated string
void hex_to_string(unsigned data, char * out_str)
{
	unsigned pwr16 = 0x1000;
	unsigned digit;
	int i = 0;

	while(pwr16 != 0)
	{
		digit = data/pwr16;
		data = data % pwr16;
		pwr16 = pwr16 >> 4;

		if (digit < 10)
		{
			digit += '0';
		}
		else
		{
			digit += ('A' - 10);
		}

		out_str[i] = digit;
		i++;
	}

	out_str[i] = '\0';

	return;
}

// This function writes an unsigned number to the display as hexadecimal.
// It will display it beginning at the line and inxed indicated. 
// Line should be 0/1 and idx should be [0, 15]
void display_write_hex(unsigned data, unsigned short line, unsigned short idx)
{
	char disp_str[5];

	// Convert the hexadecimal to a string
	hex_to_string(data, disp_str);

	// Set the display position to the desired position
	display_set_address( (line * DISPLAY_LINE_2_START) + idx);

	// And display the new string
	display_write_string(disp_str);
}

// This function performs a display reset. Use it if the power-on-reset
//	for the display doesn't work
void display_reset(void)
{
	// Do a function set
	display_write_command(DISPLAY_FUNCTION_SET_DATA);

	// Wait for > 4.1ms
	__delay_ms(5);

	// Do another function set
	display_write_command(DISPLAY_FUNCTION_SET_DATA);

	// Wait for > 100 us
	__delay_ms(1);

	// Do another function set
	display_write_command(DISPLAY_FUNCTION_SET_DATA);

	//
	// Now fire off a bunch of commands in sequence to 
	//	configure the thing
	//
	display_write_command(DISPLAY_FUNCTION_SET_DATA);
	display_write_command(DISPLAY_ON_DATA);
	display_write_command(DISPLAY_ENTRY_MODE_DATA);
	display_write_command(DISPLAY_CLEAR_DATA);

	__delay_ms(2);


	// Should be all done, so just return
	return;
}

// This function writes a null-terminated string to the display in sequential order, 
//	up to 16 characters
void display_write_string(char *str_to_write)
{
	int idx = 0;

	// Make sure the pointer is mildly valid
	if (str_to_write != NULL)
	{
		// Loop over the characters in the string until a null-terminus is hit or
		//	more than 32 characters are written, since there are only 32 chars
		//	on the entire display.
		while( (str_to_write[idx] != 0) && (idx < 16) )
		{
			display_write_char(str_to_write[idx]);
			idx++;
		}
	}

	return;
}

// This function sets the display RAM address to the passed
//	address. Line 1 begins at 0x00, Line 2 begins at 0x40. 
//	Both lines go for 16 characters.
void display_set_address(unsigned short address)
{
	display_write_command(DISPLAY_ADDRESS_DATA | address);

	__delay_us(50);
}

// Write up to 16 characters on a line of the display. 
//	If line == 0 then the first line is written, else 
//	the second line
void display_write_line(unsigned short line, char * str)
{
	if (line == 0)
	{
		display_set_address(DISPLAY_LINE_1_START);
	}
	else
	{
		display_set_address(DISPLAY_LINE_2_START);
	}

	// Now, write the string, 
	display_write_string(str);
}

// Function to clear the display.
void display_clear(void)
{
	// Send the clear and wait a bit
	display_write_command(DISPLAY_CLEAR_DATA);
	__delay_ms(2);
}

