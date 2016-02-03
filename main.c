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
    
    uint8_t state = STATE_GREEN;
    uint8_t transitionCounter = 0;
    
    while(1) {
        uint_fast16_t distanceCounts = HCSR04_Trigger();
        
        if (state == STATE_GREEN) {
            TLC5926_SetLights(LIGHT_GREEN);
            
            if (distanceCounts < LIGHT_THRESH_GREEN) {
                transitionCounter = 0;
                state = STATE_YELLOW;
                TLC5926_SetLights(LIGHT_YELLOW);
            }
            
        } else if (state == STATE_YELLOW) {          
            if (distanceCounts < LIGHT_THRESH_YELLOW) {
                transitionCounter = 0;
                state = STATE_RED;
                TLC5926_SetLights(LIGHT_RED);
            }
            else if (distanceCounts > LIGHT_THRESH_GREEN + 20) {
                transitionCounter++;
                if (transitionCounter == 10) {
                    transitionCounter = 0;
                    state = STATE_GREEN;
                    TLC5926_SetLights(LIGHT_GREEN);
                }                  
            } else {
                transitionCounter = 0;
            }
        } else if (state == STATE_RED) {          
            if (distanceCounts > LIGHT_THRESH_YELLOW + LIGHT_THRESH_OFFSET) {
                state = STATE_YELLOW;
                transitionCounter = 0;
                TLC5926_SetLights(LIGHT_YELLOW);
            } else {
                transitionCounter++;
                if(transitionCounter > 50) { // Approx five seconds of red
                    TLC5926_SetLights(LIGHT_OFF);
                    // Break the loop, go into a power saving mode
                }
            }
        }
        
        __delay_ms(100);
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

void interrupt ISR(void) 
{
//    static int oscillate = 0;
    
    // TIMER 0
    if (INTCONbits.TMR0IF && INTCONbits.TMR0IE) {
        INTCONbits.TMR0IF = 0;
    }
    
    // TIMER 1
    //if (PIR1bits.TMR1IF && PIE1bits.TMR1IE) {    
                        
        //RC6 = oscillate;
        //oscillate = !oscillate;
        
    //    PIR1bits.TMR1IF = 0;    
    //}  
    // TIMER 2
    //if (PIR1bits.TMR2IF && PIE1bits.TMR2IE) {    
        
    //    PIR1bits.TMR2IF = 0;    
    //}    
    // TIMER 3
    //if (PIR3bits.TMR4IF && PIE3bits.TMR4IE) {
    //    
    //    PIR3bits.TMR4IF = 0;
    //}
    // TIMER 4
    //if (PIR3bits.TMR6IF && PIE3bits.TMR6IE) {
    //    
    //    PIE3bits.TMR6IE = 0;
    //}
}  // End of isr()