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

#define FAILURE 1
#define SUCCESS 0

// Need to define the board number for the board being compiled
#define BOARDNUM 1

// The master board number. So if our board is this number, then
//	we are the master
#define MASTER_BOARDNUM 0


#ifdef	__cplusplus
}
#endif

#endif	/* SPACETEAM_GENERAL_H */

