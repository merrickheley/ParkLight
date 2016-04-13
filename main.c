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

typedef volatile struct Global_State {
    bool echoHit;
    bool finishedRead;
    bool setYellow;
    bool setRed;
    bool triggerSensor;
} State;

typedef volatile struct Led_State {
    uint8_t ledState; // Red by default?
    uint8_t transitionCounter;
    uint8_t counter;
} LedState;

volatile State state = { false, false, false, false }; 
volatile LedState led_state = { STATE_RED, 0 , 0 };

uint8_t thresh_red;
uint8_t thresh_yellow;

void display_LED(uint8_t counter);

void init(void) 
{
    INTCONbits.GIE = 1; // Enable global interrupts
    INTCONbits.PEIE = 1; // Enable peripheral interrupts
    INTCONbits.T0IE = 1;
    
    // Enable IOC
    INTCONbits.IOCIE = 1;
    
    // Enable A2 rising edge
    IOCAPbits.IOCAP2 = 1;
    // Enable A2 falling edge
    IOCANbits.IOCAN2 = 1;

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
    
    thresh_red = db.sdb.rangePointRed;
    thresh_yellow = db.sdb.rangePointYellow;
    
    TLC5926_init();
    
    // Drive it low to turn LED's on.
    PIN_LED_OE = IO_LOW;
}

void main()
{
    init();
      
    uint8_t flipBitch = 0;
    TLC5926_SetLights(LIGHT_RED);
    
    while(1) {
        display_LED(led_state.counter);
        HCSR04_Trigger();
        __delay_ms(200);
    }
}

void interrupt ISR(void) 
{
    // IOC triggered
    if(IOCAFbits.IOCAF2)
    {
        PIN_LED_0 = 1;
        
        // Debounce these button presses
		// if yellow button hit
		if(BTN_SET_YELLOW == IO_HIGH) {
            state.setYellow = true;
        }
		// if red button hit
		else if(BTN_SET_RED == IO_HIGH) {
            state.setRed = true;
        }
        
        // if echo pin input changed
        if(PIN_US_ECHO == IO_HIGH && state.echoHit == false) {
            // rising edge
            state.echoHit = true;
            led_state.counter = 0;
        }
        
        if(PIN_US_ECHO == IO_LOW && state.echoHit == true) {
            // falling edge
            state.finishedRead = true;
            state.echoHit = false;
        }

		//INTCONbits.IOCIF = 0; // IOCIF listed as read only
        
        // Clear all individual IOC bits to continue
        IOCAFbits.IOCAF2 = 0;
	}
    
    if(INTCONbits.TMR0IF && INTCONbits.TMR0IE) {
        
		// Increment counter if global echo bit is set
		if(state.echoHit == true) {
			led_state.counter += 1;
            
		} else if(state.finishedRead == true){

            if(state.setRed) {
                db.sdb.rangePointRed = led_state.counter;
                db_save();
                state.setRed = false;
            }
            
            if(state.setYellow) {
                db.sdb.rangePointYellow = led_state.counter;
                db_save();
                state.setYellow = false;
            }
            
            // Reset
            state.finishedRead = false;
		}
        
        
        INTCONbits.TMR0IF = 0;
    }
    
    // TIMER 1
    if (PIR1bits.TMR1IF && PIE1bits.TMR1IE) {    
        
        PIR1bits.TMR1IF = 0;    
    }
}

void display_LED(uint8_t counter) {
    
    if (counter < 10)
        TLC5926_SetLights(LIGHT_RED);
    else if (counter < 20)
        TLC5926_SetLights(LIGHT_YELLOW);
    else
        TLC5926_SetLights(LIGHT_GREEN);
    
//    LedState resultState = { STATE_RED, 0 };
//    resultState.ledState = led_state.ledState;
//    resultState.transitionCounter = led_state.transitionCounter;
//    
//    if (resultState.ledState == STATE_GREEN) {
//        TLC5926_SetLights(LIGHT_GREEN);
//
//        if (counter < thresh_yellow) {
//            resultState.transitionCounter = 0;
//            resultState.ledState = STATE_YELLOW;
//            TLC5926_SetLights(LIGHT_YELLOW);
//        }
//    } else if (resultState.ledState == STATE_YELLOW) {          
//        if (counter < thresh_red) {
//            resultState.transitionCounter = 0;
//            resultState.ledState = STATE_RED;
//            TLC5926_SetLights(LIGHT_RED);
//        }
//        else if (counter > thresh_yellow + 1) {
//            resultState.transitionCounter++;
//            if (resultState.transitionCounter == 10) {
//                resultState.transitionCounter = 0;
//                resultState.ledState = STATE_GREEN;
//                TLC5926_SetLights(LIGHT_GREEN);
//            }                  
//        } else {
//            resultState.transitionCounter = 0;
//        }
//    } else if (resultState.ledState == STATE_RED) {          
//        if (counter > thresh_red + LIGHT_THRESH_OFFSET) {
//            resultState.ledState = STATE_YELLOW;
//            resultState.transitionCounter = 0;
//            TLC5926_SetLights(LIGHT_YELLOW);
//        } else {
//            resultState.transitionCounter++;
//            if(resultState.transitionCounter > 50) { // Approx five seconds of red
//                TLC5926_SetLights(LIGHT_OFF);
//                // Break the loop, go into a power saving mode
//            }
//        }
//    }
//   
//    led_state = resultState;
}
