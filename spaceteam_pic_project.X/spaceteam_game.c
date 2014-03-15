//
// This file contains the code for the actual spaceteam game
//

#include "xc.h"
#include "spaceteam_game.h"
#include "spaceteam_io.h"
#include "spaceteam_rfid.h"

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

// Active requests array
spaceteam_request_t active_requests[MAX_MUM_PLAYERS];
// Our current request

// Buffer which holds keypresses
unsigned char key_buf[MAX_KEYPRESSES];

// Table of possible RFID tokens
unsigned char rfid_tokens[NUM_RFID_TOKENS][RFID_BYTES_PER_TOKEN] =
	{
		0x00, 0x00, 0x00, 0x00, // Token 1
		0x00, 0x00, 0x00, 0x00, // Token 2
		0x00, 0x00, 0x00, 0x00, // Token 3
		0x00, 0x00, 0x00, 0x00, // Token 4
		0x00, 0x00, 0x00, 0x00, // Token 5
		0x00, 0x00, 0x00, 0x00, // Token 6
		0x00, 0x00, 0x00, 0x00, // Token 7
		0x00, 0x00, 0x00, 0x00, // Token 8
		0x00, 0x00, 0x00, 0x00, // Token 9
		0x00, 0x00, 0x00, 0x00, // Token 10
		0x00, 0x00, 0x00, 0x00, // Token 11
		0x00, 0x00, 0x00, 0x00, // Token 12
		0x00, 0x00, 0x00, 0x00, // Token 13
		0x00, 0x00, 0x00, 0x00, // Token 14
		0x00, 0x00, 0x00, 0x00, // Token 15
		0x00, 0x00, 0x00, 0x00, // Token 16
	};

// Table of LSEL values based off of request type. This is only applicable for
//	switch type requests
unsigned char lsel_vals[NO_REQ] = 
	{
		0xFF, // Keypad doesn't need a LSEL value
		0xFF, // RFID doesn't need a LSEL value
		11,   // PB1 = S1
		2, 	  // PB2 = S4
		13,   // PB3 = S9
		0, 	  // PB4 = S10
		15,   // Toggle1 = S2
		3, 	  // Toggle2 = S3
		9,    // Toggle3 = S8
		12,   // Toggle4 = S11
		0xFF  // Knob doesn't need a LSEL value
		14,   // Tilt = S6
		10	  // IR = U1
		1, 	  // Reed = S7
	}


// Initialize the game. 
void init_game(void)
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
	static spaceteam_request_t req;

	// Generate the random board number, request type and request value 
	//	for the request
	rand_val = lfsr_get_random();
	req.board = rand_val % num_boards;

	// Generate the request type
	rand_val = lfsr_get_random();
	req.type = rand_val % NUM_REQS;

	// Generate the request value
	rand_val = lfsr_get_random();
	// Need to get the value based off of the request type
	switch(req_type)
	{
		case KEYPAD_REQ:
			req.val = rand_val % NUM_KEYPAD_VALS;
			break;
		case KNOB_REQ:
			req.val = rand_val % NUM_KNOB_VALS;
			break;
		case RFID_REQ:
			req.val = rand_Val % (num_boards * 2);
			break;
		default:
			req.val = rand_val % NUM_SWITCH_VALS;
			break;
	}

	// Send a message issuing the request
	send_message(MSG_NEW_REQ, &req);

	// Reset the request time
	req_time = REQ_TIME_MAX;
	// Reset the timer counter
	TMR1 = 0;
	// And turn on the request timer interrupts
	TIMER_1_INT_ENABLE = 1;

}

// Register a new request which we receive
void register_request(rfid_type_t type, unsigned char board, unsigned val)
{
	int i;
	// Copy the request into our active requests array at the 
	//	first available index
	for (i = 0; i < MAX_NUM_PLAYERS; i++)
	{
		if(active_requests[i].type == NO_REQ)
		{
			active_requests[i].type 	= type;
			active_requests[i].board 	= board;
			active_requests[i].val 		= val;
			active_requests[i].debounce_count = IO_DEBOUNCE_COUNT;
			break;
		}
	}
}

// Deregister a request that we had gotten
void deregister_request(unsigned char board)
{
	int i;

	// Look through our array of active requests, and 
	//	take out this one since we no longer need it
	for (i = 0; i < MAX_NUM_PLAYERS; i++)
	{
		// We just need to look at the board who issued
		//	the request for a match, since each board
		//	can only have one pending request 
		if (active_requests[i].board == board)
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
	// Turn off interrupts for not
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
	// Decrease the request time by 1
	req_time -= 1;

	// If we timed out
	if (req_time == 0)
	{
		// Turn off timer interrupts until we generate a new request
		TIMER_1_INT_ENABLE = 0;

		// Send a message that our request failed
		send_message(MSG_REQ_FAILED);

		// And generate a new request
		generate_request();
	}
}

// Set up timer 3 as a 1KHz interrupt which will do all of our polling and will
//	multiplex the LEDs
void init_timer_3(void)
{
	// Set the prescaler value to the required val for 1KHz
	PR3 = TIMER_3_1KHz;
	// Clear the timer's count register
	TMR3 = 0;

	// Set interrupt priority lower than that of wireless (7) and the 
	//	game health timer (6)
	TIMER_1_PRIORITY = 5;
	// Turn on interrupts
	TIMER_1_INT_ENABLE = 1;

	// Turn on the timer and the prescaler
	T3CON = (TIMER_3_ON | TIMER_3_PRESCALE_8);
}

// This is the timer 3 interrupt. When we are in this interrupt handler, we need 
//	to check our active requests list and check the I/O for which there is a request. 
//	If the request has been fulfilled, send off a message saying so.
void _ISR _T1Interrupt(void)
{
	int i;
	unsigned val;

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
				send_message(MSG_REQ_COMPLETED, active_requests[i].board);

				// Reallocate the message
				active_requests[i].type = NO_REQ; 
			}
		}
	}

	// Multiplex the LEDs
	multiplex_leds();

	// Debounce a row of the keypad and put the result into the key buffer
	update_keybuf();

}

// This function is passed the index of a request in the active requests array. 
//	It will check to see if the request is completed by determining the 
//	type of request and then monitoring the I/O for the request if necessary. 
//	It will also perform debouncing 
int check_request_completed(req_no)
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

// This function will check to see if a keypad request has been completed
int check_keypad_completed(unsigned val)
{
	unsigned keybuf_val;
	int ret_val = 0;

	// Calculate the value of the key buffer
	keybuf_val = 1000*key_buf[3] + 100*key_buf[2] + 10*key_buf[1] + key_buf[0];

	// And return the comparison 
	if (keybuf_val == val)
	{
		ret_val = 1;
	}
	return ret_val;
}

// This function will check to see if an RFID request has been completed
int check_rfid_completed(unsigned val)
{
	unsigned char rfid_data[RFID_BYTES_PER_TOKEN];
	int i;
	int ret_val = 0;
	int successes = 0;

	// Get the current token, if there is one
	if (rfid_get_token(rfid_data) == RFID_SUCCESS)
	{
		// If we got a token, see if it matches the one we want
		for (i = 0; i < RFID_BYTES_PER_TOKEN; i++)
		{
			// If it matches, increment a success counter
			if (rfid_data[i] == rfid_tokens[val][i])
			{
				successes++;
			}
		}

		// If we successfully matched on each byte
		if (successes == RFID_BYTES_PER_TOKEN)
		{
			ret_val = 1;
		}
	}

	return ret_val;
}

// This function will check to see if a switch request has been completed
int check_switch_completed(int req_no)
{
	int ret_val = 0;

	// Set the ISEL so that we can read the desired switch
	set_isel(isel_vals[active_requests[req_no].type]);

	// If the switch is pressed, then decrement the debounce counter
	if(get_iomux() == active_requests[req_no].val)
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

	// Get the raw sample
	adc_val = get_knob_sample();
	// Map it into the range [1, MAX_ADC_RANGE]
	adc_val = (adc_val / ADC_RANGE_DIVIDER) + 1;

	if (adc_val == val)
	{
		ret_val == 1
	}

	return ret_val;
}

// During the game, the LEDs are used for two purposes: 
//	to show time remaining on a request and to 
//	show game health. Basiclaly, the multiplexer
//	increments one LED every call, and if the 
//  LED should be on, it changes the select line to turn it 
//	on. Else, it doesn't moce the LSEL.
void multiplex_LEDs(void)
{
	// If our LED should be on 
	if ( (curr_LED < req_time) || ( (curr_LED >= MIN_HEALTH_LED) && (curr_LED < (game_health + MIN_HEALTH_LED)) ) )
	{
		// The turn it on
		set_lsel(curr_LED);
	}

	// Increment by 1 and wrap around 15
	curr_LED = (curr_LED + 1) & 0xF;
}

// This function updates the key buffer by debouncing a row per call
//	and putting any keys pressed into the correct location in the 
//	key buffer
void update_key_buf(void)
{
	unsigned char key;
	static int idx = 0;

	// Check to see if we have a key
	key = scan_and_debounce_keypad();

	// If we have a key, put it into the key_buf at the current index
	//	and then increment the index, modding around the length of the
	//	buffer
	if (key != NO_KEY)
	{
		key_buf[idx] = key;
		idx = (idx + 1) % MAX_KEYPRESSES;
	}
}



