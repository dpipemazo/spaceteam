//
// This file implements the messaging between boards in spaceteam
//

#include "xc.h"
#include "spaceteam_game.h"
#include "spaceteam_display.h"
#include "spaceteam_msg.h"

#define FCY 8000000UL
#include <libpic30.h> 

// This function sends a spaceteam message packet to another board.
//	If will eventually do this using the wireless, but for now, just 
//	keep everything local
void send_message(spaceteam_msg_t msg, spaceteam_req_t req, unsigned char board, unsigned val )
{
	char req_str[16];

	switch(msg)
	{
		// If we have a new request message, just register it on this board
		case MSG_NEW_REQ:
			// Send the request right back to this board
			register_request(req, board, val);
			display_write_request(req, board, val);
			break;
		case MSG_REQ_COMPLETED:
			deregister_request(req, board, val);
			// display_write_line(DISPLAY_LINE_2, "Req Completed!");
			break;
		case MSG_REQ_FAILED:
			deregister_request(req, board, val);
			// display_write_line(DISPLAY_LINE_2, "Req Failed!");
			break;
		default:
			break;
	}

}