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

static bool power_saving_mode = true;

/*
 * HCSR04_Trigger
 * 
 * Triggers the ultrasonic TRIGGER pin pulse.
 * Counts are then read by the external oscillator on TIMER1,
 * With the IOC_ISR registering the beginning and end of the ECHO result.
 * 
 * Input: 
 *      void
 * 
 * Output:  
 *      void
 * 
 */
void HCSR04_Trigger(void) {
    //Send at least a 10uS pulse on trigger line
    PIN_US_TRIGGER = 1; //high
    DELAY_15US(power_saving_mode);
    PIN_US_TRIGGER = 0; //low
}

void set_Power_Saving_Mode(bool mode) {
    power_saving_mode = mode;
}
