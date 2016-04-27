/* 
 * File:   main.c
 * Author: Jonathan Holland
 * Author: Merrick Heley
 *
 * Created on 23 December 2015, 7:00 PM
 */

// Project configuration files and definitions
#include "config.h"
#include "constants.h"
#include "HCSR04.h"
#include "TLC5926.h"
#include "database.h"

// C libraries
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// PIC includes
#include <xc.h>
#include <htc.h>
#include <pic16f1828.h>

typedef struct Global_State {
    bool echoHit;
    bool setYellow;
    bool setRed;
    uint8_t counter;
    uint8_t lastCounter;
} State;

typedef struct Led_State {
    uint8_t state; // Red by default
    uint8_t turnoffCounter; // How many consecutive instances of a colour
} LedState;

volatile State state = { 0 }; 
LedState led_state = { 0 };

void display_LED(LedState *ledState);
void blink_light(uint16_t lightColour);

void init(void) 
{
    INTCONbits.GIE = 1; // Enable global interrupts
    INTCONbits.PEIE = 1; // Enable peripheral interrupts
    INTCONbits.T0IE = 1;
    
    // Enable IOC
    INTCONbits.IOCIE = 1;
    
    // Enable RA2 rising edge
    IOCAPbits.IOCAP2 = 1;
    // Enable RA2 falling edge
    IOCANbits.IOCAN2 = 1;
    
    // Enable RB5 and RB4 falling edge
    IOCBNbits.IOCBN4 = 1;
    IOCBNbits.IOCBN5 = 1;

    // Enable each timer
    INTCONbits.TMR0IE = 1;
    PIE1bits.TMR1IE = 1; 
    
    //PIE1bits.TMR2IE = 1;
    //PIE3bits.TMR4IE = 1;
    //PIE3bits.TMR6IE = 1;
    
    // Weak pull-up enabled on ECHO pin
    WPUAbits.WPUA2 = 1;
    
    // Set timer0 prescaler to 1:256
    OPTION_REG = 0b00000000;
    
    // bits 7-6 -> 01 use system clock FOSC, 00 use FOSC/4
    // bit 5-4 T1CKPS<1:0>: Timer1 Input Clock Prescale Select bits
    //      11 = 1:8 Prescale value
    //      10 = 1:4 Prescale value
    //      01 = 1:2 Prescale value
    //      00 = 1:1 Prescale value
    
    // Set timer 1 prescaler to 1:1 from FOSC (which should be 
    // the value of the external oscillator)
    T1CON = 0b01000001; 
    
    TRISA = 0b11111111; // Inputs
    TRISB = 0b11110000; // Inputs
    TRISC = 0b00000000; // Outputs

    // Disable analogue inputs. This should set all pins to digital.
    ANSELBbits.ANSB4 = 0;
    ANSELBbits.ANSB5 = 0;
    ANSELAbits.ANSELA = 0x00;
    ANSELCbits.ANSELC = 0x00;
    
    // Set outputs to low initially
    PORTC = 0x00; 
    
    // Initialise the database so that it is populated
    db_init(); 
    
    TLC5926_init();
    
    // Drive it low to turn LED's on.
    PIN_LED_OE = IO_LOW;
}

void main()
{
    uint8_t i = 0;
    init();

    TLC5926_SetLights(LIGHT_OFF);
    
    while(1) {        
        if (state.setRed) {
            //db.sdb.rangePointRed = state.lastCounter;
            //db_save();
            blink_light(LIGHT_RED);
            state.setRed = false;
        }

        if (state.setYellow) {
            //db.sdb.rangePointYellow = state.lastCounter;
            //db_save();
            blink_light(LIGHT_YELLOW);
            state.setYellow = false;
        }
        
        //display_LED(&led_state);
        //HCSR04_Trigger();

        __delay_ms(250);
    }
}

void interrupt ISR(void) 
{
    // Edge detected on echo pin
    if (IOCAFbits.IOCAF2)
    {
        PIN_LED_0 = 1;

        // If echo pin input is rising edge
        if (PIN_US_ECHO == IO_HIGH && state.echoHit == false) {
            // Set edge tracker high and reset counter
            state.echoHit = true;
            state.counter = 0;
        }
        
        // If echo pin is falling edge
        if (PIN_US_ECHO == IO_LOW && state.echoHit == true) {
            // Set edge tracker low and save counter
            state.echoHit = false;
            state.lastCounter = state.counter;
        }
        
        // Clear all individual IOC bits to continue
        IOCAFbits.IOCAF2 = 0;
	}
    
    // Yellow button falling edge
    if (IOCBFbits.IOCBF4) {
        state.setYellow = true;
        IOCBFbits.IOCBF4 = 0;
    }
    
    // Red button falling edge
    if (IOCBFbits.IOCBF5) {
        state.setRed = true;
        IOCBFbits.IOCBF5 = 0;
    }
    
    // Timer 0
    if (INTCONbits.TMR0IF && INTCONbits.TMR0IE) {
        
		// Increment counter if global echo bit is set
		if (state.echoHit == true) {
			state.counter += 1;
		}       
        
        INTCONbits.TMR0IF = 0;
    }
    
    // TIMER 1
    if (PIR1bits.TMR1IF && PIE1bits.TMR1IE) {    
        PIR1bits.TMR1IF = 0;    
    }
}

void display_LED(LedState *ledState) 
{   
    uint8_t counter = state.lastCounter;
    
    // If the state is green
    if (ledState->state == STATE_GREEN) {
        // If the counter is within the yellow threshold, transition
        if (counter < db.sdb.rangePointYellow) {
            ledState->state = STATE_YELLOW;
            TLC5926_SetLights(LIGHT_YELLOW);
        }
    // If the state is yellow
    } else if (ledState->state == STATE_YELLOW) {
        // If the state is within the red threshold, transition
        if (counter < db.sdb.rangePointRed) {
            ledState->state = STATE_RED;
            TLC5926_SetLights(LIGHT_RED);
        }
        // If the state is outside the green threshold, transition
        else if (counter > db.sdb.rangePointYellow + 5) {
            ledState->state = STATE_GREEN;
            TLC5926_SetLights(LIGHT_GREEN);                
        }
    // If the state is red
    } else if (ledState->state == STATE_RED) {
        // If the state is within the yellow threshold, transition
        if (counter > (db.sdb.rangePointRed + LIGHT_THRESH_OFFSET)) {
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

void blink_light(uint16_t lightColour) {
    uint8_t i = 0;
    
    for (i = 0; i < 5; i++) {
        TLC5926_SetLights(lightColour);
        __delay_ms(200);
        TLC5926_SetLights(LIGHT_OFF);
        __delay_ms(200);
    }
}
