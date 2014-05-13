/* 
 * File:   spaceteam_general.h
 * Author: dpipemazo
 *
 * Created on February 13, 2014, 11:02 PM
 */

#ifndef SPACETEAM_GENERAL_H
#define	SPACETEAM_GENERAL_H


#ifdef	__cplusplus
extern "C" {
#endif

//
// Return Codes
//

typedef enum _spaceteam_player_t
{
	PLAYER_0,
	PLAYER_1,
	PLAYER_2,
	PLAYER_3,
	PLAYER_4,
	NUM_PLAYERS
} spaceteam_player_t;

#define MASTER_PLAYER 0
#define THIS_PLAYER	0
#define NUM_PLAYERS 5

#define FAILURE 1
#define SUCCESS 0

#ifdef	__cplusplus
}
#endif

#endif	/* SPACETEAM_GENERAL_H */

