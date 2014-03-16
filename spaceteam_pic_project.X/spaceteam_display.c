//
// This function controls the spaceteam display
//

#include "xc.h"
#include <stddef.h>
#include "spaceteam_spi.h"
#include "spaceteam_io.h"
#include "spaceteam_game.h"
#include "spaceteam_display.h"
#include "spaceteam_req.h"

#define FCY 8000000UL
#include <libpic30.h>

//
// The two display line buffers and their variables
//
char line_1_buf[DISP_MAX_STR_LEN + 1];
char line_2_buf[DISP_MAX_STR_LEN + 1];
unsigned char line_1_scroll_idx;
unsigned char line_2_scroll_idx;
unsigned char line_1_scroll_on;
unsigned char line_2_scroll_on;
unsigned char line_1_new;
unsigned char line_2_new;
unsigned char line_1_len;
unsigned char line_2_len;

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

	// Clear both of the buffers
	display_set_buffer(line_1_buf, DISP_MAX_STR_LEN + 1, 0);
	display_set_buffer(line_2_buf, DISP_MAX_STR_LEN + 1, 0);

	// Turn off scrolling for both lines
	line_1_scroll_on = SCROLL_OFF;
	line_2_scroll_on = SCROLL_OFF;

	// reset the scroll indices for the two buffers
	line_1_scroll_idx = 0;
	line_2_scroll_idx = 0;

	// Reset the "new line" variables
	line_1_new = NO_NEW_LINE;
	line_2_new = NO_NEW_LINE;

	// Reset the length variables
	line_1_len = 0;
	line_2_len = 0;

	// Set up the timer 4 interrupt for scrolling
	init_timer_4();

	// Now, reset the display and we are done
	display_reset();

}

// This function initializes the timer 4 interrupt which
//	will be used for scrolling
void init_timer_4(void)
{
	// Set the counter value to the maximum, which gets it 
	//	down to about 122 Hz. 
	PR4 = TIMER_4_122Hz;

	// Clear the timer's count register
	TMR4 = 0;

	// Set interrupt priority lower than that of wireless (7) and the 
	//	game health timer (6) and the IO timer (5). So lowest priority
	TIMER_4_PRIORITY = 4;

	// Need to clear the interrupt flag
	TIMER_4_INT_FLAG = 0;

	// Turn on interrupts
	TIMER_4_INT_ENABLE = 1;

	// Turn on the timer and the prescaler
	T4CON = (TIMER_4_ON | TIMER_4_POSTSCALE_16 | TIMER_4_PRESCALE_16);
}

// This is the timer 4 interrupt. This is responsible for scrolling the 
//	display and actually writing new strings
void _ISR _T4Interrupt(void)
{
	static unsigned char int_counter = 0;

	// If we have a new line to write , or if it's time to scroll 
	if ( (line_1_new == NEW_LINE) || ( (int_counter == (TIMER_4_INT_SCROLL - 1) ) && (line_1_scroll_on == SCROLL_ON) ) )
	{
		// Rewrite the line
		display_line_buf(DISPLAY_LINE_1);
		// Note that we don't have a new line
		line_1_new = NO_NEW_LINE;
	}

	if ( (line_2_new == NEW_LINE) || ( (int_counter == (TIMER_4_INT_SCROLL - 1) ) && (line_2_scroll_on == SCROLL_ON) ) )
	{
		// Rewrite the line
		display_line_buf(DISPLAY_LINE_2);
		// Note that we don't have a new line
		line_2_new = NO_NEW_LINE;
	}

	// Need to increment the interrupt counter. We need
	//	to only scroll the display when int_counter = T4_INT_SCROLL
	int_counter = (int_counter + 1) % TIMER_4_INT_SCROLL;

	// Need to clear the interrupt Flag
	TIMER_4_INT_FLAG = 0;
}

// This function rewrites lines. It is meant to be called from the display interrupt, and 
//	will scroll a line if necessary
void display_line_buf(unsigned char line)
{
	unsigned char * buf;
	unsigned char * scrollidx;
	unsigned char scrollon;
	unsigned char len;
	unsigned char begin_address;
	int i;
	unsigned char idx = 0;

	if (line == DISPLAY_LINE_1)
	{
		// Grab our variables
		buf = line_1_buf;
		scrollidx = &line_1_scroll_idx;
		scrollon  = line_1_scroll_on;
		begin_address = DISPLAY_LINE_1_START;
		len = line_1_len;

	}
	else
	{
		// Grab our variables
		buf = line_2_buf;
		scrollidx = &line_2_scroll_idx;
		scrollon  = line_2_scroll_on;
		begin_address = DISPLAY_LINE_2_START;
		len = line_2_len;
	}

	// Need to set the display address to the beginning of the
	//	line which we will be writing
	display_set_address(begin_address);

	// If we need to scroll
	if ((scrollon == SCROLL_ON) && (len > DISP_CHARS_PER_LINE))
	{	
		// Set our index equal to the scroll index to begin with
		idx = *scrollidx;

		// Increment the scroll index
		*scrollidx = (idx + 1) % (len + 1);
	}
	// Since we don't need to scroll, make sure the
	//	mod in the loop below doesn't do anything.
	else
	{
		len = DISP_CHARS_PER_LINE;
	}

	// We need to write the display's length of characters
	for (i = 0; i < DISP_CHARS_PER_LINE; i++)
	{
		// Write the character
		display_write_char(buf[idx]);

		// Recalculate our index, modding around the length of 
		//	the string + 1 (to account for the space)
		idx = (idx + 1) % (len + 1);
	}

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
void display_write_command(unsigned char data)
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
void display_write_char(unsigned char data)
{
	// Write the data to the shift register over the SPI connection. 
	//	NOTE: The SPI clock is running at 1/2 of the system clock, 
	//	so it will take 2*8 system clocks for the data to be valid, 
	//	which should work out

	// Write the data passed if the data is non-NULL, else
	//	write a space
	if (data != 0)
	{
		spi_write(data);
	}
	else
	{
		spi_write(' ');
	}

	// Need to set up the control signals to begin the write
	display_set_control_sigs(RS_HIGH | RW_LOW | E_LOW);

	// Now pull E high in preparation for the write
	display_set_control_sigs(RS_HIGH | RW_LOW | E_HIGH);

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
void display_write_hex(unsigned data, unsigned char line)
{
	char disp_str[5];

	// Convert the hexadecimal to a string
	hex_to_string(data, disp_str);

	// And display the new string
	display_write_line(line, disp_str);
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

// This function sets the display RAM address to the passed
//	address. Line 1 begins at 0x00, Line 2 begins at 0x40. 
//	Both lines go for 16 characters.
void display_set_address(unsigned char address)
{
	display_write_command(DISPLAY_ADDRESS_DATA | address);

	__delay_us(50);
}

// This function just copies the line passed into the appropriate 
//	line buffer which will then be displayed and scrolled
//	by the interrupt function
void display_write_line(unsigned char line, char * str)
{
	char * buffer;
	char * new_line_ptr;
	char * len_ptr;
	unsigned char * scroll_idx;


	// Figure out which buffer to use
	if (line == DISPLAY_LINE_1)
	{
		buffer = line_1_buf;
		new_line_ptr = &line_1_new;
		len_ptr = &line_1_len;
		scroll_idx = &line_1_scroll_idx;
	}
	else
	{
		buffer = line_2_buf;
		new_line_ptr = &line_2_new;
		len_ptr = &line_2_len;
		scroll_idx = &line_2_scroll_idx;
	}

	//Reset the scroll index
	*scroll_idx = 0;

	// First, clear the display buffer
	display_set_buffer(buffer, DISP_MAX_STR_LEN, 0);
	// Next, copy the string into the buffer
	*len_ptr = display_copy_string(str, buffer);
	// Finally, note that we have a new line to the 
	//	display interrupt
	*new_line_ptr = NEW_LINE;

}

// Function to clear the display.
void display_clear(void)
{
	// Send the clear and wait a bit
	display_write_command(DISPLAY_CLEAR_DATA);
	__delay_ms(2);
}

// This function converts a decimal value to its ASCII string equivalent
//	buf should be 5 chars long
void dec_to_string(unsigned val, char * buf)
{
	unsigned divider = 1000;
	unsigned char digit;
	unsigned char count = 0;
	unsigned rem;

	rem = val;

	while( divider != 0 )
	{
		digit 	= rem / divider;
		rem 	= rem % divider;
		divider = divider / 10;
		buf[count] = digit + '0';
		count++;
	}

	// Null-terminate the string
	buf[count] = 0;
}

// This function takes a request type, board and value and prints out
//	the appropriate string on the display. 
//
// 
void display_write_request(spaceteam_req_t req, unsigned char board, unsigned val)
{
	char * req_verb;
	char * req_name;
	char request[DISP_MAX_STR_LEN];
	unsigned char len;
	char val_str[5];

	req_verb = req_verbs[req];
	req_name = req_names[req];

	// Convert the value to a string
	dec_to_string(val, val_str);

	// First, clear the request buffer
	display_set_buffer(request, DISP_MAX_STR_LEN, 0);

	// Now, copy the verb into the buffer first
	len = display_copy_string(req_verb, request);
	// Put a space into the display buffer
	request[len] = ' ';
	len += 1;
	// copy the noun into the buffer
	len += display_copy_string(req_name, &request[len]);

	//
	// If the display string requires a preposition
	//	all reqs which require this are less than or equal to the
	//	knob.
	//
	if ( req <= KNOB_REQ )
	{
		request[len] = ' ';
		len += 1;
		len += display_copy_string(req_preps[req], &request[len]);
		request[len] = ' ';
		len += 1;
		len += display_copy_string(val_str, &request[len]);
	}

	// When all done, write the request to the display
	display_write_line(DISPLAY_LINE_1, request);

}

// This function sets a buffer of the passed length to the passed value
void display_set_buffer(char * buf, unsigned char len, unsigned char val)
{
	int i;
	for (i = 0; i < len; i++)
	{
		buf[i] = val;
	}
}

// This function clears a line of the display
void display_clear_line(unsigned char line)
{
	if (line == DISPLAY_LINE_1)
	{
		display_set_buffer(line_1_buf, DISP_MAX_STR_LEN, 0);
		line_1_new = NEW_LINE;
	}
	else
	{
		display_set_buffer(line_2_buf, DISP_MAX_STR_LEN, 0);
		line_2_new = NEW_LINE;
	}
}

// This function copies a string from one pointer to another, and returns
//	the length of the string copied. It will only copy the first
//  DISP_MAX_STR_LEN bytes, if it is longer than that
unsigned char display_copy_string(char * str, char * buf)
{
	unsigned char len = 0;

	while( (str[len] != 0) && (len < DISP_MAX_STR_LEN) )
	{
		buf[len] = str[len];
		len++;
	}

	// Null=terminate the buffer for good measure
	buf[len] = 0;

	return len;
}

// This function takes a line and a scroll setting and applies is
void display_scroll_set(unsigned char line, unsigned char setting)
{

	if (line == DISPLAY_LINE_1)
	{
		line_1_scroll_on = setting;
		line_1_scroll_idx = 0;
	}
	else
	{
		line_2_scroll_on = setting;
		line_2_scroll_idx = 0;
	}
}

// This function takes a key buffer and displays it on the second line 
//	of the display
void display_key_buf(char * buf)
{
	// We have our own buffer, converted to ASCII
	char line_buf[DISP_MAX_STR_LEN];
	char ascii_keys[MAX_KEYPRESSES + 1];
	int i;
	unsigned char len = 0;

	// First, clear the line buffer
	display_set_buffer(line_buf, DISP_MAX_STR_LEN, 0);

	// Copy and convert to ASCII
	for (i = 0; i < MAX_KEYPRESSES; i++)
	{
		ascii_keys[i] = buf[i] + '0';
	}

	// Null-terminate the string
	ascii_keys[i] = 0;

	// Now, put the verb into the line
	len += display_copy_string(req_names[KEYPAD_REQ], line_buf);
	// Now, copy the equals sign into the line
	len += display_copy_string(" = ", &line_buf[len]);
	// Finally, copy the key buffer into the line
	len += display_copy_string(ascii_keys, &line_buf[len]);

	// And now we can write the string
	display_write_line(DISPLAY_LINE_2, line_buf);

}








