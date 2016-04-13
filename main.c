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
    uint8_t counter;
} LedState;

volatile State state = { false, false, false, false }; 
volatile LedState led_state = { STATE_RED, 0 };

volatile uint8_t thresh_red;
volatile uint8_t thresh_yellow;

uint8_t display_LED(uint8_t counter, uint8_t state);
void display_All_Blink(void);

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
    
    // Enable RB5 and RB4 rising edge
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
    
    thresh_red = db.sdb.rangePointRed;
    thresh_yellow = db.sdb.rangePointYellow;
    
    TLC5926_init();
    
    // Drive it low to turn LED's on.
    PIN_LED_OE = IO_LOW;
}

void main()
{
    init();

    TLC5926_SetLights(LIGHT_RED);
    
    while(1) {
        led_state.ledState = display_LED(led_state.counter, led_state.ledState);
        HCSR04_Trigger();
        __delay_ms(250);
        if(state.setRed || state.setYellow) {
            display_All_Blink();
        }
    }
}

void interrupt ISR(void) 
{
    // IOC triggered
    if(IOCAFbits.IOCAF2)
    {
        PIN_LED_0 = 1;

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
    
    if(IOCBFbits.IOCBF4) {
        // Debounce these button presses
		// if yellow button hit
		if(BTN_SET_YELLOW == IO_HIGH) {
            state.setYellow = true;
        }
        IOCBFbits.IOCBF4 = 0;
    }
    
    if(IOCBFbits.IOCBF5) {
        // if red button hit
		if(BTN_SET_RED == IO_HIGH) {
            state.setRed = true;
        }
        IOCBFbits.IOCBF5 = 0;
    }
    
    if(INTCONbits.TMR0IF && INTCONbits.TMR0IE) {
        
		// Increment counter if global echo bit is set
		if(state.echoHit == true) {
			led_state.counter += 1;
		} else if(state.finishedRead == true){

            if(state.setRed) {
                db.sdb.rangePointRed = led_state.counter;
                db_save();
                thresh_red = led_state.counter;
                state.setRed = false;
            }
            
            if(state.setYellow) {
                db.sdb.rangePointYellow = led_state.counter;
                db_save();
                thresh_yellow = led_state.counter;
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

uint8_t display_LED(uint8_t counter, uint8_t state) 
{

    if (state == STATE_GREEN) {
        if (counter < thresh_yellow) {
            state = STATE_YELLOW;
            TLC5926_SetLights(LIGHT_YELLOW);
        }
    } else if (state == STATE_YELLOW) {          
        if (counter < thresh_red) {
            state = STATE_RED;
            TLC5926_SetLights(LIGHT_RED);
        }
        else if (counter > thresh_yellow + 5) {
            state = STATE_GREEN;
            TLC5926_SetLights(LIGHT_GREEN);                
        }
    } else if (state == STATE_RED) {          
        if (counter > (thresh_red + LIGHT_THRESH_OFFSET)) {
            state = STATE_YELLOW;
            TLC5926_SetLights(LIGHT_YELLOW);
        } 
        // Reimplement the turning off after red for a while functionality
        //else {

            //if(transition > 5) { // Approx five seconds of red
            //    TLC5926_SetLights(LIGHT_OFF);
                // Break the loop, go into a power saving mode
            //}
        //}
    }

    return state;
}

void display_All_Blink() {
    
    TLC5926_SetLights(0xFFFF);
    __delay_ms(200);
    TLC5926_SetLights(0x0000);
}
