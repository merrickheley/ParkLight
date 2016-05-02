/*
 * File:   TLC5926.c
 * Author: Merrick
 *
 * Created on 18 January 2016, 10:45 PM
 */

#include "TLC5926.h"

// Project includes
#include "constants.h"

// C libraries
#include <stdbool.h>
#include <stdint.h>

// PIC Includes
#include <htc.h>

/*
 * TLC5926_init
 * 
 * Initialise the LED driver
 * 
 * Inputs: 
 *      void
 * 
 * Output:
 *      void
 */
void TLC5926_init(void) {
    // Turn everything off
    PIN_LED_SDI = IO_LOW;
    PIN_LED_CLK = IO_LOW;
    PIN_LED_LE = IO_LOW;
    // This is NOT_OE, so drive it high to turn LED's off.
    PIN_LED_OE = IO_HIGH; 
}

/*
 * TLC5926_SetLights
 * 
 * Change the lights that are enabled. PIN_LE_OE must be LOW to turn leds on.
 * 
 * Inputs: 
 *      bitmap  bit 0 corresponds to LED0, bit 15 to LED15
 * 
 * Output:
 *      void
 * 
 * TODO: Check the bitmap code actually matches the documentation
 */
void TLC5926_SetLights(uint16_t bitmap) {
    uint_fast8_t i;
    
    // Send 16 pulses
    for (i = 0; i < 16; i++) {
        
        // Set the data line
        if (bitmap & (uint16_t) (1 << (15 - i)))
            PIN_LED_SDI = IO_HIGH;
        else
            PIN_LED_SDI = IO_LOW;
        
        // Trigger a rising edge
        __delay_us(TLC5926_US_DELAY);
        PIN_LED_CLK = IO_HIGH;
        
        // Drive the data line low
        // TODO: Is this even necessary?
        //__delay_us(TLC5926_US_DELAY);
        //PIN_LED_SDI = IO_LOW;
        
        // Trigger a falling edge
        __delay_us(TLC5926_US_DELAY);
        PIN_LED_CLK = IO_LOW;        
    }
    
    // Make sure everything is off
    PIN_LED_SDI = IO_LOW;
    PIN_LED_CLK = IO_LOW;
    
    // Save the data to the latch
    PIN_LED_LE = IO_HIGH;
    __delay_us(TLC5926_US_DELAY);
    PIN_LED_LE = IO_LOW;
}
