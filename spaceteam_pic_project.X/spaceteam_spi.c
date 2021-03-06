/*
 * This code is used for controlling the SPI port
 *
 * We are using SSP1
 */

 #include "xc.h"
 #include "spaceteam_spi.h"
 #include <stddef.h>

 static char init_done = 0;

 // This function initializes the SPI connection so that
 //   we are the master. 
 void init_spi(void)
 {

    SSP1CON1 = 0b0000000000100000; // Enable the serial port, SCK = Fosc/2
    SSP1CON2 = 0; // Don't need this, leave as default
    SSP1CON3 = 0; // Don't need this, leave as default
    SSP1STAT = 0b0000000001000000; // In order to get the display working, need the
                                    //  data to come out in the middle of the clock cycle

    // update the boolean stating that the module has been initialized
    init_done = 1;

 }

 // This function can be used to write a byte to the SPI bus
unsigned char _spi_write(unsigned char data, unsigned char turn_off_int)
 {
    unsigned char ret_val;

    if (turn_off_int != 0)
    {
        __builtin_disi(DISI_MAX_VAL);
    }

    // Do a dummy read to clear the BF flag
    //  if it's set
    ret_val = SSP1BUF;

    // write to the buffer
    SSP1BUF = data;

    // Wait until the transmit buffer is full
    while(SSP1STATbits.BF == 0){};

    ret_val = SSP1BUF;

    if (turn_off_int != 0)
    {
        __builtin_disi(0x0000);
    }

    return ret_val;

 }

 unsigned char spi_write(unsigned char data)
 {
    return _spi_write(data, 1);
 }

 // This function returns 1 if SPI has been initialized, else 0
 int is_spi_initialized(void)
 {
    return init_done;
 }

 // This function performs SPI writes on a bufffer of input data and
 // reads the results to a buffer of output data, unless the output data
 // pointer is NULL
 void spi_write_multiple(unsigned char * datain, unsigned char * dataout, unsigned char length)
 {
    int i;
    unsigned char temp_val;

    // Turn off interrupts
    __builtin_disi(DISI_MAX_VAL);

    // Do a SPI write for all of the data in the buffer
    for( i = 0; i < length; i++)
    {
        // Do the SPI write, if we have data to write
        if (datain != NULL)
        {
            temp_val = _spi_write(datain[i], 0);
        }
        // Or, we may be trying to do a read, in which case
        //  just send zeroes
        else
        {
            temp_val = _spi_write(0, 0);
        }
        
        // And write the return value to the output buffer if
        //  the pointer is non-NULL
        if (dataout != NULL)
        {
            dataout[i] = temp_val;
        }
    }

    // Turn interrupts back on
    __builtin_disi(0);
 }





