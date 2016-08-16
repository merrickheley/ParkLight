/* 
 * File:   display.h
 * Author: Merrick
 *
 * Created on May 2, 2016, 4:31 PM
 */

#ifndef DISPLAY_H
#define	DISPLAY_H

#include <stdint.h>
#include <stdbool.h>

// Values for Green, yellow and red lighting states
#define DISP_STATE_GREEN    0
#define DISP_STATE_YELLOW   1
#define DISP_STATE_RED      2
#define DISP_STATE_OFF      3

typedef struct Led_State {
    uint8_t state;          // Red by default
    uint8_t turnoffCounter; // How many consecutive instances of a colour
    uint8_t offReading;     // What reading was the last reading before the light turned off.
    uint8_t offState;       // What state was the light when turned off.
} LedState;

void display_LED(LedState *ledState, uint8_t reading, uint8_t yellow, uint8_t red, bool turnOff);
void blink_light(uint16_t lightColour, uint8_t flashes);

#endif	/* DISPLAY_H */

