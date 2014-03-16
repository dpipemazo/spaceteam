//
// This file contains the code for the actual spaceteam game
//

#include "xc.h"
#include "spaceteam_game.h"
#include "spaceteam_io.h"
#include "spaceteam_rfid.h"
#include "spaceteam_msg.h"
#include "spaceteam_display.h"

// Random number generator value
unsigned lfsr;
// Number of players in the game
unsigned short num_players;
// State that the game is in
game_state_t game_state;
// Time left on a given request
unsigned char req_time;
// LED which is currently being multiplexed
unsigned char curr_LED;
// The game health
unsigned char game_health;

// Active requests array
spaceteam_request_t active_requests[MAX_NUM_PLAYERS];
// Our request
spaceteam_request_t my_req;

// Whether or not we should be doing RFID scans in the mainloop
unsigned char mainloop_scan_for_rfid;

// Buffer which holds keypresses
unsigned char key_buf[MAX_KEYPRESSES];

// Buffer which holds RFID values
unsigned char rfid_buf[RFID_ID_LEN];

// Table of possible RFID tokens
unsigned char rfid_tokens[NUM_RFID_TOKENS][RFID_ID_LEN] =
	{
		{0xF5, 0x02, 0x9D, 0xE2}, // Token 1
		{0x85, 0xB9, 0x3B, 0xE3}, // Token 2
		{0xF4, 0x83, 0x97, 0x5B}, // Token 3
		{0x44, 0x81, 0x97, 0x5B}, // Token 4
		{0xAA, 0xAA, 0xAA, 0xAA}, // Token 5
		{0xAA, 0xAA, 0xAA, 0xAA}, // Token 6
		{0xAA, 0xAA, 0xAA, 0xAA}, // Token 7
		{0xAA, 0xAA, 0xAA, 0xAA}, // Token 8
		{0xAA, 0xAA, 0xAA, 0xAA}, // Token 9
		{0xAA, 0xAA, 0xAA, 0xAA}, // Token 10
		{0xAA, 0xAA, 0xAA, 0xAA}, // Token 11
		{0xAA, 0xAA, 0xAA, 0xAA}, // Token 12
		{0xAA, 0xAA, 0xAA, 0xAA}, // Token 13
		{0xAA, 0xAA, 0xAA, 0xAA}, // Token 14
		{0xAA, 0xAA, 0xAA, 0xAA}, // Token 15
		{0xAA, 0xAA, 0xAA, 0xAA} // Token 16
	};

// Table of LSEL values based off of request type. This is only applicable for
//	switch type requests
unsigned char isel_vals[NO_REQ] = 
	{
		0xFF, // Keypad doesn't need a LSEL value
		0xFF, // RFID doesn't need a LSEL value
		0xFF, // Knob doesn't need a LSEL value
		11,   // PB1 = S1
		2, 	  // PB2 = S4
		13,   // PB3 = S9
		0, 	  // PB4 = S10
		15,   // Toggle1 = S2
		3, 	  // Toggle2 = S3
		9,    // Toggle3 = S8
		12,   // Toggle4 = S11
		14,   // Tilt = S6
		10,	  // IR = U1
		1 	  // Reed = S7
	};

// Initialize the game-related variables
void init_game_vars(void)
{
	int i;

	// Just set the game state to waiting
	game_state = GAME_WAITING;

	// Loop through the active requests array and initialize them all to NO_REQ
	for (i = 0; i < MAX_NUM_PLAYERS; i++)
	{
		active_requests[i].type = NO_REQ;
	}

	// Clear out the keypress buffer
	for (i = 0; i < MAX_KEYPRESSES; i++)
	{
		key_buf[i] = 0;
	}

	// We should not be doing mainloop RFID scans
	mainloop_scan_for_rfid = 0;
}

// Initialize the game. 
void init_game(void)
{
	int i;

	// Initialize the game variables
	init_game_vars();

	// Initialize the IO
	init_io();

	// Initialize the display
	init_display();
	display_scroll_set(DISPLAY_LINE_1, SCROLL_ON);
	display_scroll_set(DISPLAY_LINE_2, SCROLL_ON);

	// Write some welcome lines to the display
	display_write_line(DISPLAY_LINE_1, "Welcome to Spaceteam!");
	display_write_line(DISPLAY_LINE_2, "Waiting for other players...");

	// Initialize the RFID
	init_rfid();

	// Initialize the wireless
	//	commented out for now until we verify that the game works on one board
	// init_wireless();

	// Initialize the timers
	init_timer_1();
	init_timer_2();

}

// Begin the game
void begin_game(unsigned short num_boards)
{
	// Seed the LFSR with the current timer 1 count XOR'd with
	//	the current sample from the ADC knob
	lfsr = (TMR1 ^ get_knob_sample());

	// Set the number of boards equal to the number of players
	num_players = num_boards;

	// Start the game state
	game_state = GAME_STARTED;

	// Set the game health
	game_health = GAME_HEALTH_MAX;

	// Need to clear the second line of the display
	display_clear_line(DISPLAY_LINE_2);

	// And generate our new request
	generate_request();
}

// The LFSR which will act as a pseudo-random number generator. This will
//	generate a pseudo-random 16-bit number, which should be sufficient 
//	to get resolution of about 20 random values of it.
unsigned lfsr_get_random(void)
{
	// The value to OR into the LFSR
	unsigned bit;

	// Calculate the feedback bit
	bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1;
	// And update the LFSR
	lfsr =  (lfsr >> 1) | (bit << 15);

	return lfsr;
}

// Generate a random game request
void generate_request(void)
{
	unsigned rand_val;

	// Generate the random board number, request type and request value 
	//	for the request
	rand_val = lfsr_get_random();
	my_req.board = rand_val % MAX_NUM_PLAYERS;

	// Generate the request type
	rand_val = lfsr_get_random();
	my_req.type = rand_val % NO_REQ;

	// IR seems to be broken in hardware for now...
	if (my_req.type == IR_REQ)
	{
		my_req.type += 1;
	}

	// Generate the request value
	rand_val = lfsr_get_random();
	// Need to get the value based off of the request type
	switch(my_req.type)
	{
		case KEYPAD_REQ:
			my_req.val = rand_val % NUM_KEYPAD_VALS;
			break;
		case KNOB_REQ:
			my_req.val = rand_val % NUM_KNOB_VALS;
			break;
		case RFID_REQ:
			my_req.val = rand_val % NUM_RFID_REQS;
			break;
		default:
			// For switches, it's always a toggle
			//	from the current state
			my_req.val = 0;
			break;
	}

	// Send a message issuing the request
	send_message(MSG_NEW_REQ, my_req.type, my_req.board, my_req.val);

	// Reset the request time
	req_time = REQ_TIME_MAX;

	// Reset the timer counter
	TMR1 = 0;

	// And turn on the request timer interrupts
	TIMER_1_INT_ENABLE = 1;

}

// Register a new request which we receive
void register_request(spaceteam_req_t type, unsigned char board, unsigned val)
{
	int i;
	unsigned char switch_val;

	// Copy the request into our active requests array at the 
	//	first available index
	for (i = 0; i < MAX_NUM_PLAYERS; i++)
	{
		if(active_requests[i].type == NO_REQ)
		{
			active_requests[i].type 	= type;
			active_requests[i].board 	= board;
			active_requests[i].debounce_count = IO_DEBOUNCE_COUNT;

			// If this is a switch request, we need to figure out what the
			//	current state of the switch is so that we can set the 
			//	request value as a toggle
			if (type > KNOB_REQ)
			{
				switch_val = get_switch_val(isel_vals[type]);
				if (switch_val == 0)
				{
					active_requests[i].val = 1;
				}
				else
				{
					active_requests[i].val = 0;
				}
			}
			else
			{
				active_requests[i].val = val;
			}

			break;
		}
	}
}

// Deregister a request that we had gotten
void deregister_request(spaceteam_req_t type, unsigned char board, unsigned val)
{
	int i;

	// Look through our array of active requests, and 
	//	take out this one since we no longer need it
	for (i = 0; i < MAX_NUM_PLAYERS; i++)
	{
		// Match on all categories for good measure
		if ( (active_requests[i].board == board) && (active_requests[i].type == type) && (active_requests[i].val == val) )
		{
			// 'Free' up the slot by changing the type 
			//	to no request
			active_requests[i].type = NO_REQ;
		}
	}
}

// This function gets called if we get a message that our request was 
//	completed
void request_done(void)
{
	// Turn off timer interrupts
	TIMER_1_INT_ENABLE = 0;

	// And generate a new request
	generate_request();
}

// Set up timer 1 to count at 1/256 of the system clock and interrupt
//	when the count register is full
void init_timer_1(void)
{
	// Set the period value to the maximum value, 64K
	PR1 = 0xFFFF;

	// Clear the timer count
	TMR1 = 0;

	// Set interrupt priority lower than that of wireless which is 7
	TIMER_1_PRIORITY = 6;

	// Turn off the timer 1 interrupt flag
	TIMER_1_INT_FLAG = 0;

	// Turn off interrupts for now
	TIMER_1_INT_ENABLE = 0;

	// Turn the timer on, based off of the internal oscillator and 
	//	prescaled by 1/256. This generates a period of ~2s per timer
	//	interrupt, which gives about 16s for 
	T1CON = TIMER_1_ON | TIMER_1_PRESCALE_256;

}

// This is the timer 1 interrupt. If the game state is IDLE or OVER then
//	do nothing. If we are in the game, we want to decrease the request time
//	value by 1
void _ISR _T1Interrupt(void)
{

	// Need to set the chip selects
	int_set_chip_selects();

	if (game_state == GAME_STARTED)
	{
		// Decrease the request time by 1
		req_time -= 1;

		// If we timed out
		if (req_time == 0)
		{
			// Turn off timer interrupts until we generate a new request
			TIMER_1_INT_ENABLE = 0;

			// Send a message that our request failed
			send_message(MSG_REQ_FAILED, my_req.type, my_req.board, my_req.val);

			// Decrement the game health
			game_health -= 1;

			// If the game is over, then re-initialize everything
			if (game_health == 0)
			{
				// Restart the game
				init_game_vars();
				// And print a game-over statement
				display_clear();
				display_write_line(DISPLAY_LINE_1, "GAME OVER!");
			}
			// Otherwise, generate a new request
			else
			{
				generate_request();
			}
		}
	}

	// Need to clear the interrupt flag
	TIMER_1_INT_FLAG = 0;

	// Needs to reset chip selects
	int_reset_chip_selects();

}

// Set up timer 2 as a 1KHz interrupt which will do all of our polling and will
//	multiplex the LEDs
void init_timer_2(void)
{
	// Set the counter value to the required val for 1KHz
	PR2 = TIMER_2_1KHz;

	// Clear the timer's count register
	TMR2 = 0;

	// Set interrupt priority lower than that of wireless (7) and the 
	//	game health timer (6)
	TIMER_2_PRIORITY = 5;

	// Need to clear the interrupt flag
	TIMER_2_INT_FLAG = 0;

	// Turn on interrupts
	TIMER_2_INT_ENABLE = 1;

	// Turn on the timer and the prescaler
	T2CON = (TIMER_2_ON | TIMER_2_POSTSCALE_16 | TIMER_2_PRESCALE_16);
}

// This function checks to see if the begin button has been debounced
int is_begin_debounced(void)
{
	static char db_val;
	int ret_val = 0;

	// If the button is pressed (active low)
	if (get_switch_val(BEGIN_ISEL_VAL) == 0)
	{
		db_val -= 1;
	}
	else
	{
		db_val = IO_DEBOUNCE_COUNT;
	}

	// If db_val has reached zero, then we return 1 and reset it
	if (db_val == 0)
	{
		ret_val = 1;
		db_val = IO_DEBOUNCE_COUNT;
	}

	return ret_val;
}

// This is the timer 2 interrupt. When we are in this interrupt handler, we need 
//	to check our active requests list and check the I/O for which there is a request. 
//	If the request has been fulfilled, send off a message saying so.
void _ISR _T2Interrupt(void)
{
	int i;

	// Need to set the chip selects
	int_set_chip_selects();

	// If we are playing the game, we need to see if we have
	//	completed any of our pending requests
	if (game_state == GAME_STARTED)
	{
		//
		// Check all of our active requests
		//
		for (i = 0; i < MAX_NUM_PLAYERS; i++)
		{
			// See if there is a request pending in the slot
			if(active_requests[i].type != NO_REQ)
			{
				// Call the function which gets the current value for the request
				if (check_request_completed(i))
				{
					// Send a message saying that the resuest has been completed
					send_message(MSG_REQ_COMPLETED, active_requests[i].type, active_requests[i].board, active_requests[i].val);

					// Reallocate the message
					active_requests[i].type = NO_REQ; 

					// And generate a new request
					generate_request();
				}
			}
		}

		// Multiplex the LEDs
		multiplex_leds();
	}
	// Otherwise, we need to look for the begin button being pressed to 
	//	transition from the end of the game to something more usefun
	else
	{
		// If the begin button is debounced
		if(is_begin_debounced())
		{
			// Then start the game!
			begin_game(1);
		}
	}

	// Need to clear the interrupt Flag
	TIMER_2_INT_FLAG = 0;

	// And need to reset chip selects
	int_reset_chip_selects();

}

// This function is passed the index of a request in the active requests array. 
//	It will check to see if the request is completed by determining the 
//	type of request and then monitoring the I/O for the request if necessary. 
//	It will also perform debouncing 
int check_request_completed(int req_no)
{
	int ret_val = 0;

	// Need to do this check differently based on the 
	//	value of the request type
	switch(active_requests[req_no].type)
	{
		case KEYPAD_REQ:
			ret_val = check_keypad_completed(active_requests[req_no].val);
			break;
		case RFID_REQ:
			ret_val = check_rfid_completed(active_requests[req_no].val);
			break;
		case KNOB_REQ:
			ret_val = check_knob_completed(active_requests[req_no].val);
			break;
		default:
			ret_val = check_switch_completed(req_no);
			break;
	}

	return ret_val;
}

// This function updates the key buffer by debouncing a row per call
//	and putting any keys pressed into the correct location in the 
//	key buffer
int check_keypad_completed(unsigned val)
{
	unsigned char key;
	static int idx = 0;
	int i;
	int ret_val = 0;
	unsigned key_entry = 0xFFFF; // Not a valid combination
	unsigned key_multiplier = 1000;

	// Check to see if we have a key
	key = scan_and_debounce_keypad();

	// If we have a key, put it into the key_buf at the current index
	//	and then increment the index, modding around the length of the
	//	buffer
	if (key != NO_KEY)
	{
		// Take different actions based on the type of key
		switch(key)
		{
			// If we get a CLR, clear out the key buffer
			case CLR_CODE:
				for (i = 0; i < MAX_KEYPRESSES; i++)
				{
					key_buf[i] = 0;
				}
				idx = 0;
				break;
			// If we get a RUN code, compute the buffer's contents into an unsigned
			case RUN_CODE:
				key_entry = 0;
				for (i = 0; i < MAX_KEYPRESSES; i++)
				{
					key_entry += key_multiplier*key_buf[i];
					key_multiplier /= 10;
				}
				break;
			// Otherwise, just put the key in the key
			//	buffer.
			default:
				key_buf[idx] = key;
				idx = (idx + 1) % MAX_KEYPRESSES;
				break;
		}

		// Want to display the key buffer
		display_key_buf(key_buf);
	}

	// If we got it right, return the right value and clear out the 
	//	key buffer
	if (key_entry == val)
	{
		// Clear out the buffer
		for (i = 0; i < MAX_KEYPRESSES; i++)
		{
			key_buf[i] = 0;
		}

		// Clear the second line of the display
		display_clear_line(DISPLAY_LINE_2);

		// update the return value
		ret_val = 1;
	}

	return ret_val;
}

// This function will check to see if an RFID request has been completed
int check_rfid_completed(unsigned val)
{
	unsigned char rfid_data[RFID_ID_LEN];
	int i;
	int ret_val = 0;
	int successes = 0;

	// Set the flag that we should be doing RFID scans in the mainloop
	mainloop_scan_for_rfid = 1;

	// Check the rfid_buffer
	for (i = 0; i < RFID_ID_LEN; i++)
	{
		if (rfid_buf[i] == rfid_tokens[val][i])
		{
			successes++;
		}
	}

	// If we have RFID_ID_LEN successes, then we have the correct value
	if (successes == RFID_ID_LEN)
	{
		ret_val = 1;
		// And want to clear the internal RFID buffer
		for (i = 0; i < RFID_ID_LEN; i++)
		{
			rfid_buf[i] = 0;
		}
		// And also want to turn off scanning in the mainloop
		mainloop_scan_for_rfid = 0;
		// And finally, want to clear the second line of the display
		//	which should be showing an RFID token
		display_clear_line(DISPLAY_LINE_2);
	} 

	return ret_val;
}

// This function tells the mainloop whether or not it should be scanning for RFID values
int scan_for_rfid(void)
{	
	return mainloop_scan_for_rfid;
}

// This function takes an RFID value found by the mainloop and 
//	writes it to our internal RFID buffer
void set_game_rfid(unsigned char * data)
{
	int i;

	for (i = 0; i < RFID_ID_LEN; i++)
	{	
		rfid_buf[i] = data[i];
	}

	// Display the RFID token acquired
	display_rfid_token(data);
}

// This function will check to see if a switch request has been completed
int check_switch_completed(int req_no)
{
	int ret_val = 0;

	// If the switch is pressed, then decrement the debounce counter
	if(get_switch_val(isel_vals[active_requests[req_no].type]) == active_requests[req_no].val)
	{
		active_requests[req_no].debounce_count -= 1;
		// If the switch has been debounced
		if (active_requests[req_no].debounce_count == 0)
		{
			ret_val = 1;
		}
	}
	// Otherwise, reset the debounce counter
	else
	{
		active_requests[req_no].debounce_count = IO_DEBOUNCE_COUNT;
	}

	return ret_val;
}

// This function will check to see if an ADC request has been completed
int check_knob_completed(unsigned val)
{
	unsigned adc_val;
	int ret_val = 0;

	// Get the raw sample
	adc_val = get_knob_sample();
	// Map it into the range [1, MAX_ADC_RANGE]
	adc_val = adc_val / ADC_RANGE_DIVIDER;

	if (adc_val == val)
	{
		ret_val = 1;
	}

	return ret_val;
}

// During the game, the LEDs are used for two purposes: 
//	to show time remaining on a request and to 
//	show game health. Basiclaly, the multiplexer
//	increments one LED every call, and if the 
//  LED should be on, it changes the select line to turn it 
//	on. Else, it doesn't moce the LSEL.
void multiplex_leds(void)
{
	// If our LED should be on 
	if ( (curr_LED < req_time) || ( (curr_LED >= MIN_HEALTH_LED) && (curr_LED < (game_health + MIN_HEALTH_LED)) ) )
	{
		// Then turn it on
		set_lsel(curr_LED);
	}

	// Increment by 1 and wrap around 15
	curr_LED = (curr_LED + 1) & 0xF;
}





