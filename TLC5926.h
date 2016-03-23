#ifndef TLC5926_H
#define	TLC5926_H

#include <stdint.h>

#define TLC5926_US_DELAY   10

typedef struct Led_State {
    uint8_t ledState; // Red by default?
    uint8_t transitionCounter;
} LedState;

void TLC5926_init(void);
void TLC5926_SetLights(uint16_t bitmap);
LedState display_LED(uint_fast16_t distanceCounter, uint8_t currentState, 
        uint8_t transitionCounter);

static int thresh_red;
static int thresh_yellow;

#endif	/* TLC5926_H */
