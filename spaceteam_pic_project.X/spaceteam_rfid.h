/* 
 * File:   spaceteam_rfid.h
 * Author: dpipemazo
 *
 * Created on February 13, 2014, 10:18 PM
 */

#ifndef SPACETEAM_RFID_H
#define	SPACETEAM_RFID_H

#include "xc.h"

#ifdef	__cplusplus
extern "C" {
#endif

// Commands 
#define RFID_IDLE 				0x00	
#define RFID_RECEIVE 			0x08
#define RFID_TRANSCIEVE 		0x0C
#define RFID_SOFTRESET 			0x0F

// Registers
#define RFID_COMMAND_REG   		0x01
#define RFID_IRQ_REG			0x04
#define RFID_FIFO_DATA_REG 		0x09
#define RFID_FIFO_LEVEL_REG 	0x0A
#define RFID_CONTROL_REG		0x0C
#define RFID_BITFRAMING_REG		0x0D
#define RFID_MODE_REG			0x11
#define RFID_TXCONTROL_REG		0x14
#define RFID_TXAUTO_REG			0x15
#define RFID_TXSEL_REG			0x16
#define RFID_RXSEL_REG			0x17
#define RFID_TMODE_REG			0x2A
#define RFID_TPRESCALER_REG		0x2B
#define RFID_TRELOADH_REG		0x2C
#define RFID_TRELOADL_REG		0x2D
#define RFID_VERSION_REG		0x37

// Mask for indicating that we are doing a read
#define RFID_READ_MASK 			0x80

// Other defines
#define RFID_CLEAR_FIFO			0x80

// Mask for waiting for a transmission to be done
#define IRQ_WAIT_MASK			0x30

// The type of RFID ID request which is being used
#define RFID_REQ_IDL			0x26	// Search all non-idle tags int he area
#define RFID_REQ_ALL			0x52	// search ALL tags in the area
#define RFID_IDREQ				0x93

// Timeout for the RFID request
#define RFID_TIMEOUT_CYCLES		2000

// Maximum size of the readback buffer for RFID
#define RFID_MAX_LEN 			16
#define RFID_ID_LEN				4

// Chip selects
#define RFID_CS 	LATBbits.LATB12

// Possible statuses for the RFID transactions
typedef enum _rfid_status_t
{
	RFID_TIMEOUT,
	RFID_SUCCESS,
	RFID_ERROR
} rfid_status_t;

// Function declarations
void init_rfid(void);
void rfid_write_reg(unsigned char addr, unsigned char data);
unsigned char rfid_read_reg(unsigned char addr);
rfid_status_t rfid_request_type(unsigned char *data);
rfid_status_t rfid_request_id(unsigned char *data);
rfid_status_t rfid_get_token(unsigned char *data);


#ifdef	__cplusplus
}
#endif

#endif	/* SPACETEAM_RFID_H */