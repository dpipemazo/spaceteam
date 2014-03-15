//
// This file contains the code for the actual spaceteam game
//

#include "xc.h"
#include "spaceteam_game.h"
#include "spaceteam_io.h"

// Random number generator value
unsigned lfsr;
// Number of players in the game
unsigned short num_players;
// State that the game is in
game_state_t game_state;
// Game health
unsigned req_time;

// Active requests array
spaceteam_requests_t active_requests[MAX_MUM_PLAYERS];

// Lookup table of functions which 



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
void generate_request(spaceteam_request_t * req)
{
	unsigned rand_val;

	// Generate the random board number, request type and request value 
	//	for the request
	rand_val = lfsr_get_random();
	req->board = rand_val % num_boards;

	// Generate the request type
	rand_val = lfsr_get_random();
	req->type = rand_val % NUM_REQS;

	// Generate the request value
	rand_val = lfsr_get_random();
	// Need to get the value based off of the request type
	switch(req_type)
	{
		case KEYPAD_REQ:
			req->val = rand_val % NUM_KEYPAD_VALS;
			break;
		case KNOB_REQ:
			req->val = rand_val % NUM_KNOB_VALS;
			break;
		case RFID_REQ:
			req->val = rand_Val % (num_boards * 2);
			break;
		default:
			req->val = rand_val % NUM_SWITCH_VALS;
			break;
	}

	// Reset the request time
	req_time = REQ_TIME_MAX;
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
	// Turn on interrupts
	TIMER_1_INT_ENABLE = 1;

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

	// 
	if (req_time == 0)
	{
		send_message(MSG_REQ_FAILED);
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

	// Work our way through the active requests
	for (i = 0; i < MAX_NUM_PLAYERS; i++)
	{
		// See if there is a request pending in the slot
		if(active_requests[i].type != NO_REQ)
		{
			// Call the function which gets the current value for the request

		}
	}

}

