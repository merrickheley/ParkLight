/* 
 * File:   blink.c
 * Author: Jonathan Holland
 *
 * Created on 16 November 2015, 6:52 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <p24FJ32GA002.h>

unsigned char counter = 0;
unsigned char triggerBit = 0;

void init(void) {
    AD1PCFG = 0xFFFF; // Pins to digital 
    
    // Set up Timer 1 for internal use
    T1CON = 0x00; // Stops Timer 1
    TMR1 = 0x00; // Clear timer
    PR1 = 0xFFFF;
    IPC0bits.T1IP = 0x01; // Timer 1 level 1 priority
    IFS0bits.T1IF = 0; // Clear Timer 1 status flag
    IEC0bits.T1IE = 1; // Enable Timer 1 Interrupts
    
    // Use internal clock, 1:1 prescale, start timer
    //T1CON = 0b101000000000000;
    // Can also use T1CONbits.TON = 1; to turn on timer 1
    T1CONbits.TCKPS = 0b00;
    T1CONbits.TON = 1;

    TRISBbits.TRISB15 = 0; // TRIS B8 is output
    TRISBbits.TRISB14 = 1; //
    TRISBbits.TRISB7 = 0; // TRIS B7 is output
    TRISBbits.TRISB6 = 0; // TRIS B6 is output
}

/*
 * Clock fCPU = 8MHz,
 *       fOSC = 8/4 MHz = 2MHz,
 *       1/2MHz = .5us,
 *       2MHz / 2^16 = 30.5Hz interrupt resolution
 * Therefore, approximately 30 overflows to 1 second
 */
void __attribute__((interrupt, auto_psv)) _T1Interrupt(void)
{
    ++counter; //increment the counter variable by 1
    
    //if(counter%30 == 0) {
    //    counter = 0;
    //    LATBbits.LATB7 = (LATBbits.LATB7 == 1) ? 0 : 1;  // output RB7
    //    LATBbits.LATB6 = (LATBbits.LATB6 == 1) ? 0 : 1; // output RB6
    //}
    
    if(counter % 30 == 0) {    
        // Send a pulse to the ultrasonic transceiver
        LATBbits.LATB15 = 1;
        LATBbits.LATB7 = (LATBbits.LATB7 == 1) ? 0 : 1;  // output RB7
    }
    else if(counter % 41 == 0) {
        LATBbits.LATB15 = 0;
        counter = 0;
    }
    
    // Read back the ECHO pin data
    unsigned char echo = PORTBbits.RB14;
    if(echo != 0) {
        LATBbits.LATB6 = (LATBbits.LATB6 == 1) ? 0 : 1; // output RB6
    }

    IFS0bits.T1IF = 0; // Reset interrupt flag
}

int main(void) {
    init();
    while(1) {
    }
    return (0);
}

