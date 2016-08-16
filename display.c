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
#include "utils.h"

#include <stdio.h>

#define DIST_THRESH 2

void display_LED(LedState *ledState, uint8_t reading, 
        uint8_t yellow, uint8_t red, bool turnOff) 
{   
    uint8_t oldLedState = ledState->state;
    static bool wasOff = false;
    
    // The LED's were off, and there has been significant enough change 
    // in the reading to trigger activation.
    if (oldLedState == DISP_STATE_OFF && 
            (absdiff(reading, ledState->offReading) > DIST_THRESH)) {
        ledState->state = ledState->offState;
    // If the state is green
    } else if (oldLedState == DISP_STATE_GREEN) {
        // If the counter is within the yellow threshold, transition
        if (reading < yellow)
            ledState->state = DISP_STATE_YELLOW;
    // If the state is yellow
    } else if (oldLedState == DISP_STATE_YELLOW) {
        // If the state is within the red threshold, transition
        if (reading < red)
            ledState->state = DISP_STATE_RED;
        // If the state is outside the green threshold, transition
        else if (reading > (yellow + DIST_THRESH))
            ledState->state = DISP_STATE_GREEN;
    // If the state is red
    } else if (oldLedState == DISP_STATE_RED) {
        // If the state is within the yellow threshold, transition
        if (reading > (red + DIST_THRESH))
            ledState->state = DISP_STATE_YELLOW;
    }        
    
    // If the led state hasn't been changed
    if (ledState->state == oldLedState)
        ledState->turnoffCounter++;
    
    // If turnoff is true, turn the lights off
    if (turnOff == true && wasOff == false)
        TLC5926_SetLights(LIGHT_OFF);
    // If the led state has changed, or the lights were previous off but are
    // now on, drive the lights.
    else if (ledState->state != oldLedState || wasOff != turnOff) {
        // Only reset the timeout counter if oldLedState has changed.
        if (ledState-> state != oldLedState)
            ledState->turnoffCounter = 0;
        
        // Set the lights depending on the state.
        if (ledState->state == DISP_STATE_RED)
            TLC5926_SetLights(LIGHT_RED);
        else if (ledState->state == DISP_STATE_YELLOW)
            TLC5926_SetLights(LIGHT_YELLOW);
        else if (ledState->state == DISP_STATE_GREEN)
            TLC5926_SetLights(LIGHT_GREEN);
    }
    
    wasOff = turnOff;
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