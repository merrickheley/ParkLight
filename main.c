/* 
 * File:   main.c
 * Author: Jonathan Holland
 * Author: Merrick Heley
 *
 * Created on 23 December 2015, 7:00 PM
 */

// Project configuration files and definitions
#include "config.h"
#include "pinout.h"

// C libraries
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// PIC includes
#include <xc.h>
#include <htc.h>
#include <pic16f1828.h>

#define _XTAL_FREQ 500000

#define TRUE 1
#define FALSE 0

void HCSRTrigger();

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
    
    // Set outputs to low initially
    PORTC = 0x00; 
}

void main()
{
    init();
    
    while(1) {
        HCSRTrigger();
        __delay_ms(1000);
        PIN_LED_0 = LED_OFF;
        __delay_ms(1000);
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

void HCSRTrigger() {
    uint_fast16_t counter = 0;
    uint_fast8_t risingEdge = FALSE;
    
    //Send at least a 10uS pulse on trigger line
    PIN_US_TRIGGER = 1; //high
    __delay_us(15); //wait 15uS
    PIN_US_TRIGGER = 0; //low
    
    while (1) {
        // If the pin is high
        if (PIN_US_ECHO > 0) {
            // There has been a rising edge
            risingEdge = TRUE;
            
            // Check if we can increment the counter, otherwise break.
            // TO DO: This should be compared to the max calibrated distance.
            // If it is further away, we don't have to wait any longer.
            if (counter < UINT_FAST16_MAX) {
               counter++;
            } else {
                break;
            }
            
        // If a falling edge occurs, break out.
        } else if (risingEdge == TRUE && PIN_US_ECHO == 0) {
            break;
        }
        __delay_us(10);
    }
    
    if (counter > 50) {
        PIN_LED_0 = LED_ON;
    }
}