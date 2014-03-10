/* 
 * File:   spaceteam_spi.h
 * Author: dpipemazo
 *
 * Created on February 13, 2014, 10:18 PM
 */

#ifndef SPACETEAM_SPI_H
#define	SPACETEAM_SPI_H

#ifdef	__cplusplus
extern "C" {
#endif

// Function declarations
void init_spi(void);
unsigned spi_write(unsigned short data);
int is_spi_initialized(void);

#ifdef	__cplusplus
}
#endif

#endif	/* SPACETEAM_SPI_H */