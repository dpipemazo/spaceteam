//
// This file implements the messaging between boards in spaceteam
//

#include "xc.h"
#include "spaceteam_game.h"
#include "spaceteam_display.h"
#include "spaceteam_msg.h"
#include "spaceteam_general.h"
#include "spaceteam_wireless.h"

#define FCY 8000000UL
#include <libpic30.h> 

// memory used for sending a packet
spaceteam_packet_t packet_buf;

// This function sends a spaceteam message packet to another board.
//	If will eventually do this using the wireless, but for now, just 
//	keep everything local
void send_message(spaceteam_msg_t msg, spaceteam_req_t req, unsigned char sender, unsigned char recipient, unsigned val )
{
	// First, see if the board that we are sending this to is our own. If so, 
	//	just process it now and don't bother routing it through the master,
	//	though that theoretically should work
	if (recipient == THIS_PLAYER)
	{
		parse_message(msg, req, sender, recipient, val);
	}
	else
	{
		// Move the message info into the packet buffer
		packet_buf.type = msg;
		packet_buf.sender = sender;
		packet_buf.recipient = recipient;
		packet_buf.request = req;
		packet_buf.val = val;

		// if we are the wireless master, send it. Otherwise, we want to write it to our 
		//	ACK FIFO
		#if (THIS_PLAYER == MASTER_PLAYER)
			// And then send the packet
			wl_module_send_payload((unsigned char *)&packet_buf, recipient);
		// If we are not the master
		#else
			// Then send it as a response
			wl_module_send_ack((unsigned char *)&packet_buf);
		#endif
	}
}

// This function parses messages that are meant for our board
void parse_message(spaceteam_msg_t msg, spaceteam_req_t req, unsigned char sender, unsigned char recipient, unsigned val)
{
	unsigned char * players;
	int i = 0;

	// #if (THIS_PLAYER == MASTER_PLAYER)
	// 	char forward_msg = 0;
	// #endif

	// // We first need to see if we should simply be forwarding this message along, as 
	// //	can happen if we are the wireless master
	// #if (THIS_PLAYER == MASTER_PLAYER)
	// 	if (recipient != THIS_PLAYER)
	// 	{
	// 		send_message(msg, req, sender, recipient, val);
	// 	}
	// 	// Otherwise, the message is meant for us
	// 	else
	// #endif
	{
		// Parse the message based on message type
		switch(msg)
		{
			// If it is a new request, then regster it
			case MSG_NEW_REQ:
				register_request(req, sender, val);
				break;
			// If it is a failed request, then deregister it
			case MSG_REQ_FAILED:
				deregister_request(req, sender, val);
				break;
			// If our request has been completed
			case MSG_REQ_COMPLETED:
				// Just generate a new request
				// generate_request();
				break;
			// If it's a standard polling message from the master
			case MSG_POLL:
				// Don't think you have to do anything
				break;
			// If it's a game health message, then decrement the 
			//	game health accordingly, and if we are a wireless
			//	master then we need to forward this to all boards
			case MSG_HEALTH:
				dec_game_health();
				break;
			// If it's a networking request, then register the player
			case MSG_NETWORKING:
				register_player(sender);
				break;
			// We need to begin the game!
			case MSG_BEGIN:
				begin_game();
				break;
			default:
				break;
		}

		// If we need to forward the message out to all of the other boards, 
		//	the figure out which players are present and send the message to them
		// #if (THIS_PLAYER == MASTER_PLAYER)
		// 	if (forward_msg)
		// 	{
		// 		// Figure out the players
		// 		players = get_active_players();
		// 		// Send message to all of them who are not the sender
		// 		for (i = 0; i < NUM_PLAYERS; i++)
		// 		{
		// 			if ( (i != MASTER_PLAYER) && (i != sender) && (players[i] == PLAYER_PLAYING) )
		// 			{
		// 				send_message(msg, req, sender, i, val);
		// 			}
		// 		}
		// 	}
		// #endif
	}

}


