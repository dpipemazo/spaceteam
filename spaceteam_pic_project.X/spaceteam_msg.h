//
// This is the include file for the messagine functionality of spaceteam
//

#ifndef SPACETEAM_MSG_H_
#define SPACETEAM_MSG_H_

#include "spaceteam_game.h"

// Different messages within the game
typedef enum _spaceteam_msg_t
{
	MSG_NEW_REQ,
	MSG_REQ_COMPLETED,
	MSG_REQ_FAILED,
	MSG_POLL,
	MSG_HEALTH,
	MSG_NETWORKING,
	MSG_BEGIN,
	NUM_MSGS
} spaceteam_msg_t;

// The message packet to be sent
typedef struct _spaceteam_packet_t
{
	spaceteam_msg_t type;			// The type of message this is
	unsigned char sender;			// which board originated the message
	unsigned char recipient;		// which board is the final destination of the message
	spaceteam_req_t request;		// If necessary, the game request type
	unsigned 		val;			// And the game request value
} spaceteam_packet_t;

// The size of a spaceteam packet
#define SPACETEAM_PACKET_SIZE (sizeof(spaceteam_packet_t))

//
// Function declarations
//
void send_message(spaceteam_msg_t msg, spaceteam_req_t req, unsigned char sender, unsigned char recipient, unsigned val );
void parse_message(spaceteam_msg_t msg, spaceteam_req_t req, unsigned char sender, unsigned char recipient, unsigned val);

#endif /* SPACETEAM_MSG_H_ */