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

#include <stdio.h>

#define LED_TIMER_OFF_THRESH 25
#define DIST_THRESH 2

uint8_t absdiff(uint8_t a, uint8_t b) 
{
    if (a > b)
        return a-b;
    else
        return b-a;
}

void display_LED(LedState *ledState, uint8_t reading, uint8_t yellow, uint8_t red) 
{   
    uint8_t oldLedState = ledState->state;
    
    // The LED's were off, and there has been significant enough change 
    // in the reading to trigger activation.
    if (oldLedState == STATE_OFF && 
            (absdiff(reading, ledState->offReading) > DIST_THRESH)) {
        ledState->state = ledState->offState;
    // If the state is green
    } else if (oldLedState == STATE_GREEN) {
        // If the counter is within the yellow threshold, transition
        if (reading < yellow)
            ledState->state = STATE_YELLOW;
    // If the state is yellow
    } else if (oldLedState == STATE_YELLOW) {
        // If the state is within the red threshold, transition
        if (reading < red)
            ledState->state = STATE_RED;
        // If the state is outside the green threshold, transition
        else if (reading > (yellow + DIST_THRESH))
            ledState->state = STATE_GREEN;
    // If the state is red
    } else if (oldLedState == STATE_RED) {
        // If the state is within the yellow threshold, transition
        if (reading > (red + DIST_THRESH))
            ledState->state = STATE_YELLOW;
    }        
    
    // If the led state hasn't been changed
    if (ledState->state == oldLedState && oldLedState != STATE_OFF) {
        // Turn the light off if we're over the threshold
        if (ledState->turnoffCounter >= LED_TIMER_OFF_THRESH) {
            ledState->offState = ledState->state;
            ledState->offReading = reading;
            ledState->state = STATE_OFF;
        }
        // Otherwise increment the counter
        else
            ledState->turnoffCounter++;
    } 
    
    // Only do something if the led state has changed
    if (ledState->state != oldLedState) {
        // If the state is off, set the lights off
        if (ledState->state == STATE_OFF)
            TLC5926_SetLights(LIGHT_OFF);
        // If the state has changed, reset the counter & update lights
        else {
            ledState->turnoffCounter = 0;
            if (ledState->state == STATE_RED)
                TLC5926_SetLights(LIGHT_RED);
            else if (ledState->state == STATE_YELLOW)
                TLC5926_SetLights(LIGHT_YELLOW);
            else if (ledState->state == STATE_GREEN)
                TLC5926_SetLights(LIGHT_GREEN);
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