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
        if (bitmap & (1 << (15 - i)))
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

LedState display_LED(uint_fast16_t distanceCounter, uint8_t currentState, 
        uint8_t transitionCounter) {
    
    LedState resultState = { STATE_RED, 0 };
    resultState.ledState = currentState;
    resultState.transitionCounter = transitionCounter;
    
    if (resultState.ledState == STATE_GREEN) {
        TLC5926_SetLights(LIGHT_GREEN);

        if (distanceCounter < LIGHT_THRESH_GREEN) {
            resultState.transitionCounter = 0;
            resultState.ledState = STATE_YELLOW;
            TLC5926_SetLights(LIGHT_YELLOW);
        }
    } else if (resultState.ledState == STATE_YELLOW) {          
        if (distanceCounter < LIGHT_THRESH_YELLOW) {
            resultState.transitionCounter = 0;
            resultState.ledState = STATE_RED;
            TLC5926_SetLights(LIGHT_RED);
        }
        else if (distanceCounter > LIGHT_THRESH_GREEN + 20) {
            resultState.transitionCounter++;
            if (resultState.transitionCounter == 10) {
                resultState.transitionCounter = 0;
                resultState.ledState = STATE_GREEN;
                TLC5926_SetLights(LIGHT_GREEN);
            }                  
        } else {
            resultState.transitionCounter = 0;
        }
    } else if (resultState.ledState == STATE_RED) {          
        if (distanceCounter > LIGHT_THRESH_YELLOW + LIGHT_THRESH_OFFSET) {
            resultState.ledState = STATE_YELLOW;
            resultState.transitionCounter = 0;
            TLC5926_SetLights(LIGHT_YELLOW);
        } else {
            resultState.transitionCounter++;
            if(resultState.transitionCounter > 50) { // Approx five seconds of red
                TLC5926_SetLights(LIGHT_OFF);
                // Break the loop, go into a power saving mode
            }
        }
    }
    
    return resultState;
}
