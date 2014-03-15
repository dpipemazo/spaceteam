//
// This file implements the messaging between boards in spaceteam
//

#include "xc.h"
#include "spaceteam_game.h"
#include "spaceteam_display.h"
#include "spaceteam_msg.h"

// This function sends a spaceteam message packet to another board.
//	If will eventually do this using the wireless, but for now, just 
//	keep everything local
void send_message(spaceteam_msg_t msg, spaceteam_req_t req, unsigned char board, unsigned val )
{
	char req_str[16] = "rXXXXbXXXXvXXXX";

	switch(msg)
	{
		// If we have a new request message, just register it on this board
		case MSG_NEW_REQ:
			// Send the request right back to this board
			register_request(req, board, val);

			// Compile the request to display
			hex_to_string(req, &req_str[1]);
			hex_to_string(board, &req_str[6]);
			hex_to_string(val, req_str);

			// And display it
			display_write_line(0, &req_str[11]);
			break;
		case MSG_REQ_COMPLETED:
			display_write_line(1, "Req Completed!");
			break;
		case MSG_REQ_FAILED:
			deregister_request(req, board, val);
			display_write_line(1, "Req Failed!");
			break;
		default:
			break;
	}

}