//
// This is the include file for the actual game
//

#ifndef SPACETEAM_GAME_H_
#define SPACETEAM_GAME_H_

// The maximum number of players
#define MAX_NUM_PLAYERS	8

// The maximum number of keys which can be entered
#define MAX_KEYPRESSES  4

// The number of possible RFID tokens
#define NUM_RFID_TOKENS 		16
#define RFID_BYTES_PER_TOKEN 	4
#define NUM_RFID_REQS			1

// The maximum ADC range
#define MAX_ADC_RANGE 10
#define MAX_ADC_VAL 0x0400
#define ADC_RANGE_DIVIDER (MAX_ADC_VAL / MAX_ADC_RANGE)

// The types of requests which the game can make
typedef enum _spaceteam_req_t
{
	KEYPAD_REQ,
	RFID_REQ,
	KNOB_REQ,
	PB1_REQ, // S1
	PB2_REQ, // S4
	PB3_REQ, // S9
	PB4_REQ, // S10
	TOGGLE1_REQ, // S2
	TOGGLE2_REQ, // S3
	TOGGLE3_REQ, // S8
	TOGGLE4_REQ, // S11 
	TILT_REQ, // S6
	IR_REQ,   // U1
	REED_REQ,
	NO_REQ		// This should always be last
} spaceteam_req_t;

// Modulos for different request types
#define NUM_KEYPAD_VALS 10000
#define NUM_KNOB_VALS   10
#define NUM_SWITCH_VALS	2

typedef struct _spaceteam_request
{
	spaceteam_req_t		type;
	unsigned char 		board;
	unsigned 			val;
	unsigned char		debounce_count;

} spaceteam_request_t;

// Timer values
#define TIMER_1_ON 				0x8000
#define TIMER_1_PRESCALE_256 	0x0030
#define TIMER_1_INT_ENABLE 		IEC0bits.T1IE
#define TIMER_1_PRIORITY		IPC0bits.T1IP
#define TIMER_1_INT_FLAG		IFS0bits.T1IF

#define TIMER_2_ON 				0x0004
#define TIMER_2_POSTSCALE_16	0x0078
#define TIMER_2_PRESCALE_16		0x0003
#define TIMER_2_1KHz			32
#define TIMER_2_INT_ENABLE 		IEC0bits.T2IE
#define TIMER_2_PRIORITY		IPC1bits.T2IP
#define TIMER_2_INT_FLAG		IFS0bits.T2IF

// Request time value
#define REQ_TIME_MAX			8

// Number of timer interruprs (1KHz to wait before debouncing)
#define IO_DEBOUNCE_COUNT 		25

// Values for multiplexing the LEDs
#define MIN_HEALTH_LED			8
#define GAME_HEALTH_MAX			8

// LSEL value for the begin button
#define BEGIN_LSEL_VAL			8

// Different states that the game can be in
typedef enum _game_state_t
{
	GAME_WAITING,
	GAME_STARTED,
	GAME_OVER
} game_state_t;;

//
// Function declarations
//
void init_game(void);
void begin_game(unsigned short num_boards);
unsigned lfsr_get_random(void);
void generate_request(void);
void register_request(spaceteam_req_t type, unsigned char board, unsigned val);
void deregister_request(spaceteam_req_t type, unsigned char board, unsigned val);
void request_done(void);
void init_timer_1(void);
void _ISR _T1Interrupt(void);
void init_timer_2(void);
void _ISR _T2Interrupt(void);
int check_request_completed(int req_no);
int check_keypad_completed(unsigned val);
int check_rfid_completed(unsigned val);
int check_switch_completed(int req_no);
int check_knob_completed(unsigned val);
void multiplex_leds(void);
void update_key_buf(void);
int is_begin_debounced(void);

#endif /* SPACETEAM_GAME_H_ */