/* 
 * File:   display.h
 * Author: Merrick
 *
 * Created on May 2, 2016, 4:31 PM
 */

#ifndef DISPLAY_H
#define	DISPLAY_H

#include <stdint.h>

typedef struct Led_State {
    uint8_t state;          // Red by default
    uint8_t turnoffCounter; // How many consecutive instances of a colour
} LedState;

void display_LED(LedState *ledState, uint8_t reading, uint8_t yellow, uint8_t red);
void blink_light(uint16_t lightColour, uint8_t flashes);

#endif	/* DISPLAY_H */

