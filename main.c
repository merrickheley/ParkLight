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
} State;

static State state = { false, false }; 
static LedState led_state = { STATE_RED, 0 };

void init(void) 
{
    INTCONbits.GIE = 1; // Enable global interrupts
    INTCONbits.PEIE = 1; // Enable peripheral interrupts
    INTCONbits.T0IE = 1;

    // Enable each timer
    INTCONbits.TMR0IE = 1;
    //PIE1bits.TMR1IE = 1; 
    //PIE1bits.TMR2IE = 1;
    //PIE3bits.TMR4IE = 1;
    //PIE3bits.TMR6IE = 1;
    
    // Set overall timer & timer0 prescaler to 1:1
    OPTION_REG = 0b00000000;
    
    // bits 7-6 -> 01 use system clock FOSC, 00 use FOSC/4
    // bit 5-4 T1CKPS<1:0>: Timer1 Input Clock Prescale Select bits
    //      11 = 1:8 Prescale value
    //      10 = 1:4 Prescale value
    //      01 = 1:2 Prescale value
    //      00 = 1:1 Prescale value
    
    // Set timer 1 prescaler to 1:1 from FOSC
    T1CON = 0b01000001; 
    
    TRISB = 0b11110000; // Input for RB5
    TRISC = 0b00000000; // Outputs for RC2 and RC6
    
    // Disable analogue inputs. This should set all pins to digital.
    ANSELBbits.ANSB4 = 0;
    ANSELBbits.ANSB5 = 0;
    ANSELAbits.ANSELA = 0x00;
    ANSELCbits.ANSELC = 0x00;
    
    // Set outputs to low initially
    PORTC = 0x00; 

    TLC5926_init();
}

void main()
{
    init();
    
    // Drive it low to turn LED's on.
    PIN_LED_OE = IO_LOW;
    
    while(1) {
    }
}

void interrupt ISR(void) 
{
    static uint_fast16_t counter = 0;
	
    // IOC triggered
    if(INTCONbits.IOCIF && INTCONbits.IOCIE)
    {
		// if yellow button hit
		if(BTN_SET_YELLOW == IO_HIGH) {
            //TODO
        }
		// if red button hit
		else if(BTN_SET_RED == IO_HIGH) {
            //TODO
        }
        // if echo pin input changed
        else if(PIN_US_ECHO == IO_HIGH && state.echoHit == false) {
            // rising edge
            state.echoHit = true;
        } 
        else if(PIN_US_ECHO == IO_LOW && state.echoHit == true) {
            // falling edge
            state.finishedRead = true;
        }

		INTCONbits.IOCIF = 0;
	}
    
    // TIMER 0 - System clock
    if (INTCONbits.TMR0IF && INTCONbits.TMR0IE) {
        
		// Every so often trigger the Ultrasonic sensor
		HCSR04_Trigger();
		// And leave the IOC_ISR to handle triggering the counting/stop counting
		
		INTCONbits.TMR0IF = 0;
    }
    
    // TIMER 1 - External Oscillator
    if (PIR1bits.TMR1IF && PIE1bits.TMR1IE) {    
        
		// Increment counter if global echo bit is set
		if(state.echoHit == false) {
			counter++;
		} else if(state.finishedRead == true){
			// display using the count
            led_state = display_LED(counter, led_state.ledState, 
                                    led_state.transitionCounter);
            // Reset
            counter = 0;
            state.echoHit = false;
            state.finishedRead = false;
		}
		
        PIR1bits.TMR1IF = 0;    
    }
}

void db_test() {
    
    unsigned int a = 0;
    unsigned int i;
    db_init();

    for(i=0;i<5;i++)
    {
        a = ~a;
        db.serialised[i] = a;
    }
    
    db_write(i);
    db_save();
    __delay_ms(1000);
        
    if(db_read(0)) {
        for(i=0;i<5;i++)
        {
            PIN_LED_0 = db.serialised[i];
            __delay_ms(1000);
        }
    }
}

void db_test_read_only() {
    
    unsigned int i;
    db_init();
 
    if(db_read(0)) {
        for(i=0;i<5;i++)
        {
            PIN_LED_0 = db.serialised[i];
            __delay_ms(1000);
        }
    }
}