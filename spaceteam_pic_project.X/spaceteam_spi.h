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
unsigned spi_write(unsigned char data);
int is_spi_initialized(void);
void spi_write_multiple(unsigned char * datain, unsigned char * dataout, unsigned int length);

#ifdef	__cplusplus
}
#endif

#endif	/* SPACETEAM_SPI_H */