//
// This file is for handling the spaceteam IO ports
//

#include "spaceteam_general.h"
#include "spaceteam_io.h"
#include "xc.h"

#define FCY 8000000UL
#include <libpic30.h> // NOTE: MUST be included AFTER spaceteam_init.h

// variable for initialization
static int init_done = 0;

// variable for keypad column
static unsigned char curr_col; 
static unsigned debounce_counts[NUM_KEYS];

// The constant array that maps key codes from the
//  keypad function to their equivalent ASCII/command 
//  value
const unsigned char key_map[] = {
                                    RUN_CODE, 7, 4, 1, // Column 1, from bottom to top
                                           0, 8, 5, 2, // Column 2,
                                    CLR_CODE, 9, 6, 3  // COlumn 3
                                 };


//
// This function initializes the spaceteam program by 
//  setting up all necessary I/O
//
void init_io(void)
{
    ANSA = 0x0000; // All digital on port A
    ANSB = 0x4000; // Need analog input for the knob
    
    TRISA = TRISAVAL;
    TRISB = TRISBVAL;

    // Initialize the keypad too
    init_keypad();

    // Need to set up the A/D for the knob
    AD1CON2 = 0x0000;   // Can leave this as 0
    AD1CON3 = 0x0000; 
    AD1CON1 = 0x0000; 
    AD1CHS  = 0x0A0A;   // Set RB14 = AN10 as the input to the ADC  

    // Now, turn on the ADC module
    AD1CON1bits.ADON = 1;

    // Set the chip selects high
    LATAbits.LATA7 = 1;
    LATBbits.LATB12 = 1;

    init_done = 1;

    return;
}

// This function returns TRUE if IO has been initialized, false else. 
int is_io_initialized(void)
{
    return init_done;
}

// Use this function to set the select lines of the
//  IO mux to the passed value. Argument should
//  be range [0,15].
void set_isel(unsigned char val)
{
    unsigned char temp_val;
    
    // Read the previous value of port A
    temp_val = PORTA;
    // OR in the new value
    temp_val = ((temp_val & ISEL_MASK) | (val & (~ISEL_MASK)));
    // Write the new value back
    LATA = temp_val;

    // Success, so return 0
    return;
}


// Use this function to set the select lines of the
//  LED mux to the passed value. Argument should
//  be range [0,15].
void set_lsel(unsigned val)
{
    unsigned temp_val;
    
    // Read the previous value of port B
    temp_val = PORTB;
    // OR in the new value
    temp_val = ((temp_val & LSEL_MASK) | ((val << 3) & (~LSEL_MASK)));
    // Write the new value back
    LATB = temp_val;

    // Success, so return 0
    return;
}

// Use this function to get the value of IO mux
unsigned get_iomux(void)
{
	return IOIN_PORT;
}

void init_keypad(void)
{
    unsigned curr_portb;
    int i;

    // Set all of the column drivers high
    curr_portb = PORTB;
    curr_portb = (curr_portb | KEYPAD_MASK);
    LATB = curr_portb;

    //Initialize the current column to 0
    curr_col = 0;

    //Initialize all of the debounce counnts
    for (i = 0; i < NUM_KEYS; i++)
    {
        debounce_counts[i] = DEBOUNCE_TIME;
    }
}

// Use this function to scan and debounce the keypad
//  
//  Keypad is an input for the following values of isel:
//      4, 5, 6, 7
//
// This function will only return one keypress per column scanned
//  as it is currently written. If two keys are pressed at the same
//  exact time, one will be missed. In practice, this is probably
//  quite difficult.
unsigned char scan_and_debounce_keypad(void)
{
    unsigned curr_portb;
    int i, idx;

    // keycodes to return are 0 - 11, NO_KEY indicates
    //  that no key was returned
    unsigned ret_val = NO_KEY;

    // First, we need to drive the keypad's column low
    curr_portb = PORTB;
    curr_portb = curr_portb & (~(COL_DRIVE_MASK << curr_col));
    LATB = curr_portb;

    // Give it a few clocks to propogate
    __delay_us(1);

    // Now, we want to look at the various rows and see if they are low
    for (i = 0; i < 4; i++)
    {
        // Calculate the index
        idx = curr_col*4 + i;

        // Row inputs are 4, 5, 6 and 7
        set_isel(4 + i);

        // give it a few clocks to propogate 
        // __delay_us(1);

        // If the key has been pressed
        if(get_iomux() == 0)
        {

            // decrement the debounce counter
            debounce_counts[idx] -= 1;

            // If we have decremented it to 0, then we have a key to 
            //  return
            if (debounce_counts[idx] == 0)
            {
                ret_val = key_map[idx];
                debounce_counts[idx] = AUTO_REP_TIME;
            }
        }
        // Else the key has not been pressed, so reset the 
        //  debounce counter to the default value
        else
        {
            debounce_counts[idx] = DEBOUNCE_TIME;
        }
    }

    // Need to send the column driver line high again
    curr_portb = curr_portb | (COL_DRIVE_MASK << curr_col);
    LATB = curr_portb;

    // Wrap around the three rows
    curr_col = (curr_col + 1) % 3;

    return ret_val;
}

// Get a sample from the knob ADC
unsigned get_knob_sample(void)
{

    // Set the sample bit
    AD1CON1bits.SAMP = 1;

    // Give it a few microseconds to complete
    __delay_us(2);

    // Now clear the sample bit to begin conversion
    AD1CON1bits.SAMP = 0;

    // Wait until the conversion is done
    while(!AD1CON1bits.DONE){};

    return ADC1BUF0;

}
