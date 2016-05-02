/* 
 * File:   display.c
 * Author: Merrick
 *
 * Created on May 2, 2016, 4:31 PM
 */

#include "display.h"

#include <pic16f1828.h>
#include <xc.h>

#include "constants.h"
#include "TLC5926.h"

void display_LED(LedState *ledState, uint8_t reading, uint8_t yellow, uint8_t red) 
{       
    // If the state is green
    if (ledState->state == STATE_GREEN) {
        // If the counter is within the yellow threshold, transition
        if (reading < yellow) {
            ledState->state = STATE_YELLOW;
            TLC5926_SetLights(LIGHT_YELLOW);
        }
    // If the state is yellow
    } else if (ledState->state == STATE_YELLOW) {
        // If the state is within the red threshold, transition
        if (reading < red) {
            ledState->state = STATE_RED;
            TLC5926_SetLights(LIGHT_RED);
        }
        // If the state is outside the green threshold, transition
        else if (reading > (yellow + LIGHT_THRESH_OFFSET)) {
            ledState->state = STATE_GREEN;
            TLC5926_SetLights(LIGHT_GREEN);                
        }
    // If the state is red
    } else if (ledState->state == STATE_RED) {
        // If the state is within the yellow threshold, transition
        if (reading > (red + LIGHT_THRESH_OFFSET)) {
            ledState->state = STATE_YELLOW;
            TLC5926_SetLights(LIGHT_YELLOW);
            ledState->turnoffCounter = 0;
        } 
        // If there is no change in state
        else {
            // Increment the state counter
            ledState->turnoffCounter++;
            // After approx five seconds of red
            if (ledState->turnoffCounter > 5) { 
                TLC5926_SetLights(LIGHT_OFF);
            }
        }
    }
}

void blink_light(uint16_t lightColour, uint8_t flashes) {
    uint8_t i = 0;

    for (i = 0; i < flashes; i++) {
        TLC5926_SetLights(lightColour);
        __delay_ms(200);
        TLC5926_SetLights(LIGHT_OFF);
        __delay_ms(200);
    }
}