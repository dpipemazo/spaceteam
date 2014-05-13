/*
 * File:   test_main.c
 * Author: dpipemazo
 *
 * Created on February 12, 2014, 4:19 PM
 */

#include "xc.h"
#include "spaceteam_game.h"
#include "spaceteam_display.h"
#include "spaceteam_io.h"
#include "spaceteam_rfid.h"
#include "spaceteam_general.h"
#include "spaceteam_msg.h"
#include "spaceteam_wireless.h"

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
    
    unsigned char rfid_data[RFID_ID_LEN];
    rfid_status_t status;
    unsigned char * players;
    unsigned char player_idx;
    game_state_t game_state;
    char master_payload[wl_module_PAYLOAD_LEN] = "Iron Man!";
    char slave_payload[wl_module_PAYLOAD_LEN] = "Justin Hammer!";


    // Want to initialize the game
    init_game();

    // Enter the waiting state for other players
    network_with_other_players();

    // Made it to while loop!
    display_write_line(1, "game over!");

    // Just loop for now
    while(1){};

    display_write_line(1, "game over!");

    return 0;
}
