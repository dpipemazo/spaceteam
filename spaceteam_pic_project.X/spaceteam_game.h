//
// This is the include file for the actual game
//

#ifndef SPACETEAM_GAME_H_
#define SPACETEAM_GAME_H_

// The maximum number of players
#define MAX_NUM_PLAYERS	8

// The types of requests which the game can make
typedef enum _req_type_t
{
	KEYPAD_REQ,
	RFID_REQ,
	PB1_REQ,
	PB2_REQ,
	PB3_REQ,
	PB4_REQ,
	TOGGLE1_REQ,
	TOGGLE2_REQ,
	TOGGLE3_REQ,
	TOGGLE4_REQ,
	KNOB_REQ,
	TILT_REQ,
	IR_REQ,
	REED_REQ,
	NO_REQ		// This should always be last
} req_type_t;

// Modulos for different request types
#define NUM_KEYPAD_VALS 10000
#define NUM_KNOB_VALS   10
#define NUM_SWITCH_VALS	2

typedef struct _spaceteam_request
{
	req_type_t		type;
	unsigned char 	board;
	unsigned 		val;
	unsigned char	debounce_count;

} spaceteam_request_t;

// Timer values
#define TIMER_1_ON 				0x8000
#define TIMER_1_PRESCALE_256 	0x0030
#define TIMER_1_INT_ENABLE 		IEC0bits.T1IE
#define TIMER_1_PRIORITY		IPC0bits.T1IP

#define TIMER_3_ON 				0x0001
#define TIMER_3_PRESCALE_8		0x00A0
#define TIMER_3_1KHz			1000
#define TIMER_3_INT_ENABLE 		IEC0bits.T3IE
#define TIMER_3_PRIORITY		IPC2bits.T3IP

// Request time value
#define REQ_TIME_MAX			8

// Different states that the game can be in
typedef enum _game_state_t
{
	GAME_WAITING,
	GAME_STARTED,
	GAME_OVER
} game_state_t;;

// Different messages within the game
typedef enum _game_msg_t
{
	MSG_NEW_REQ,
	MSG_REQ_COMPLETED,
	MSG_REQ_FAILED,
	MSG_POLL,
	MSG_HEALTH,
	MSG_NETWORKING,
	NUM_MSGS
} game_msg_t;

#endif /* SPACETEAM_GAME_H_ */