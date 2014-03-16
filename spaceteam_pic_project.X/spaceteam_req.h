//
// This file contains all of the data and pointers necessary for 
//	displaying requests. It's in here so as not to clutter
//	up the display file
//
#include "spaceteam_game.h"


//
// These are the request verbs
//
const char * req_randomize 	= "Randomize";
const char * req_set		= "Set";
const char * req_engage		= "Engage";
const char * req_check  	= "Check";
const char * req_deactivate = "Deactivate";
const char * req_cycle	    = "Cycle";
const char * req_provide	= "Provide";
const char * req_deploy		= "Deploy";
const char * req_flip		= "FLIP";
const char * req_eject	 	= "Eject";
const char * req_vent	 	= "Vent";
const char * req_flood		= "Flood";
const char * req_crank	 	= "Crank";
const char * req_tune		= "Tune";

//
// These are the request nouns
//
const char * req_auth		= "Authentication";
const char * req_airbag		= "Airbag";
const char * req_thrust		= "Thruster";
const char * req_vaporizer	= "Vaporizer";
const char * req_pilot		= "Pilot";
const char * req_impeller	= "Impeller";
const char * req_combustor	= "Combustor";
const char * req_distiller 	= "Distiller";
const char * req_shields	= "Shields";
const char * req_network	= "Network";
const char * req_reflector	= "Reflector";
const char * req_sequencer 	= "Sequencer";
const char * req_perc		= "Percolator";
const char * req_ship		= "YOUR SHIP";

const (const char *) req_vergs[NO_REQ] =
{
	req_set, 		req_provide, 	req_cycle, 		req_deactivate, 
	req_engage, 	req_vent, 		req_randomize, 	req_check, 
	req_deploy, 	req_eject, 		req_crank, 		req_flip, 
	req_flood, 		req_align
};

const (const char *) req_names[NO_REQ] =
{
	req_thrust, 	req_auth,		req_vaporizer, 	req_network, 
	req_percolator,	req_combustor, 	req_sequencer,	req_impeller, 
	req_airbag, 	req_pilot, 		req_distiller, 	req_ship, 
	req_reflector, 	req_shields,
};

#define MAX_REQ_STR_LEN 32