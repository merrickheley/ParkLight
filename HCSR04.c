/*
 * File:   HCSR04.c
 * Author: Merrick
 *
 * Created on 18 January 2016, 10:16 PM
 */

#include "HCSR04.h"

// Project includes
#include "constants.h"

// C libraries
#include <stdbool.h>
#include <stdint.h>

// PIC Includes
#include <htc.h>

/*
 * HCSR04_Trigger
 * 
 * Get the distance from the HCSR04 ultrasonic sensor. 
 * 
 * Input: 
 *      void
 * 
 * Output:  
 *      void
 * 
 * TODO: Change this so it outputs a number of counts?
 */
void HCSR04_Trigger(void) {
    uint_fast16_t counter = 0;
    uint_fast8_t risingEdge = false;
    
    //Send at least a 10uS pulse on trigger line
    PIN_US_TRIGGER = 1; //high
    __delay_us(15); //wait 15uS
    PIN_US_TRIGGER = 0; //low
    
    // Loop until we get a falling edge. 
    // TODO: Add some protection in here to break out if there's no falling edge?
    while (1) {
        // If the pin is high
        if (PIN_US_ECHO > 0) {
            // There has been a rising edge
            risingEdge = true;
            
            // Check if we can increment the counter, otherwise break.
            // TO DO: This should be compared to the max calibrated distance.
            // If it is further away, we don't have to wait any longer.
            if (counter < UINT_FAST16_MAX) {
               counter++;
            } else {
                break;
            }
            
        // If a falling edge occurs, break out.
        } else if (risingEdge == true && PIN_US_ECHO == 0) {
            break;
        }
        __delay_us(10);
    }
    
    if (counter > 50) {
        PIN_LED_0 = LED_ON;
    }
}
