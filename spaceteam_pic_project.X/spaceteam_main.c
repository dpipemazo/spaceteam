/*
 * File:   test_main.c
 * Author: dpipemazo
 *
 * Created on February 12, 2014, 4:19 PM
 */

#include "xc.h"
#include "spaceteam_io.h"
#include "spaceteam_display.h"
#include "spaceteam_rfid.h"

//
// Define the clock frequency
//
#define FCY 8000000UL
#include <libpic30.h> 


//
// Define the configuration stuff
//
    
// FBS
#pragma config BWRP = OFF               // Boot Segment Write Protect (Disabled)
#pragma config BSS = OFF                // Boot segment Protect (No boot flash segment)

// FGS
#pragma config GWRP = OFF               // General Segment Flash Write Protect (General segment may be written)
#pragma config GSS0 = OFF               // General Segment Code Protect (No Protection)

// FOSCSEL
#pragma config FNOSC = FRCPLL           // Oscillator Select (Fast RC Oscillator with Postscaler and PLL (FRCDIV+PLL))
#pragma config SOSCSRC = DIG            // SOSC Source Type (Digital Mode for use with external clock on SCLKI)
#pragma config LPRCSEL = HP             // LPRC Power and Accuracy (High Power/High Accuracy)
#pragma config IESO = ON                // Internal External Switch Over bit (Internal External Switchover mode enabled (Two-speed Start-up enabled))

// FOSC
#pragma config POSCMD = NONE            // Primary Oscillator Mode (Primary oscillator disabled)
#pragma config OSCIOFNC = ON            // CLKO Pin I/O Function (Port I/O enabled (CLKO disabled))
#pragma config POSCFREQ = HS            // Primary Oscillator Frequency Range (Primary Oscillator/External Clock frequency >8MHz)
#pragma config SOSCSEL = SOSCHP         // SOSC Power Selection Configuration bits (Secondary Oscillator configured for high-power operation)
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor Selection (Clock Switching and Fail-safe Clock Monitor Disabled)

// FWDT
#pragma config WDTPS = PS32768          // Watchdog Timer Postscale Select bits (1:32768)
#pragma config FWPSA = PR128            // WDT Prescaler bit (WDT prescaler ratio of 1:128)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable bits (WDT disabled in hardware; SWDTEN bit disabled)
#pragma config WINDIS = OFF             // Windowed Watchdog Timer Disable bit (Standard WDT selected (windowed WDT disabled))

// FPOR
#pragma config BOREN = BOR3             // Brown-out Reset Enable bits (Enabled in hardware; SBOREN bit disabled)
#pragma config PWRTEN = ON              // Power-up Timer Enable (PWRT enabled)
#pragma config I2C1SEL = PRI            // Alternate I2C1 Pin Mapping bit (Default SCL1/SDA1 Pins for I2C1)
#pragma config BORV = V18               // Brown-out Reset Voltage bits (Brown-out Reset at 1.8V)
#pragma config MCLRE = ON               // MCLR Pin Enable bit (RA5 input disabled; MCLR enabled)

// FICD
#pragma config ICS = PGx3               // ICD Pin Placement Select (EMUC/EMUD share PGC3/PGD3)

int main(void) {
    unsigned key;
    unsigned idx = 0;
    unsigned ad_val, ad_val_prev, id_val;
    unsigned char num_bits;
    unsigned char data[RFID_ID_LEN];
    int i;
    char loop_str[] = "loop = x\0";
    unsigned hex;
    rfid_status_t status;
    char rfid_id[10];

    // Initialize the data to make sure we're getting something
    for (i = 0; i < RFID_MAX_LEN; i++)
    {
      data[i] = 0xFF;
    }

    i = 0;


    // Init the IO
    init_io();

    // Initialize the display, and do a quick display test
    init_display();

    // Initialize the RFID
    init_rfid();

    // Loop doing some sort of test
    while(1)
    {

      //
      // LED TEST
      //
      // for (idx = 0; idx < 4; idx++)
      // {
      //   set_lsel(idx);
      //   __delay_ms(500);
      // }

      //
      // RFID Test
      //
      display_clear();

      // Write the data that comes back
      status = rfid_get_token(data);
      if (status == RFID_SUCCESS)
      {
          // Compile the first 16 bits of hex of the ID
          hex = data[0];
          hex = hex << 8;
          hex += data[1];
          hex_to_string(hex, rfid_id);
          // Compile the second 16 bits of hex of the ID
          hex = data[2];
          hex = hex << 8;
          hex += data[3];
          hex_to_string(hex, rfid_id + 4);

          display_write_line(0, rfid_id);
      }
      else if(status == RFID_ERROR)
      {
        display_write_line(0, "RFID ERROR");
      }
      else
      {
        display_write_line(0, "NO CARD FOUND");
      }

      __delay_ms(1000);


      //
      // A/D Test
      //
      // ad_val = get_knob_sample();
      // if(ad_val != ad_val_prev)
      // {
      //   display_clear();
      //   display_write_hex(ad_val);
      //   ad_val_prev = ad_val;
      // }

      // __delay_ms(500)


      //
      // Keypad test
      //

      // key = scan_and_debounce_keypad();

      // if (key != NO_KEY)
      // {

      //   // For lols, turn on the LED of the key
      //   set_lsel(key);

      //   if (idx == 16)
      //   {
      //     display_set_address(0x40);
      //   }
      //   else if (idx == 32)
      //   {
      //     display_set_address(0x00);
      //     idx = 0;
      //   }

      //   // How to handle the different keys
      //   switch(key)
      //   {
      //     case CLR_CODE:
      //       display_clear();
      //       idx = 0;
      //       break;

      //     case RUN_CODE:
      //       display_clear();
      //       display_write_string("RUN!");
      //       idx = 0;
      //       break;

      //     default:
      //       display_write_char('0' + key);
      //       idx++;
      //       break;
      //   }

      // }
      
    }

    return 0;
}
