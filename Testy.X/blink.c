/* 
 * File:   blink.c
 * Author: Jonathan Holland
 *
 * Created on 1 December 2015, 8:52 PM
 */

#pragma config FOSC = INTRC_IO  // Oscillator (INTRC with I/O function on OSC2/CLKOUT)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (Disabled)
#pragma config CP = OFF         // Code Protection bit (Code protection off)
#pragma config IOSCFS = 8MHz    // Internal Oscillator Frequency Select (8 MHz INTOSC Speed)
#pragma config CPSW = OFF       // Code Protection bit - Flash Data Memory (Code protection off)
#pragma config BOREN = OFF      //  (BOR Disabled)
#pragma config DRTEN = OFF      //  (DRT Disabled)
 
#include <xc.h>
#include <htc.h>
#define _XTAL_FREQ 8000000
#include <stdio.h>
#include <stdlib.h>
#include <pic16f570.h>

#define TRUE 1
#define FALSE 0

void init(void) {
    INTCON0bits.GIE = 1; //Enable INTs globally
    INTCON1bits.T0IE = 1; // Enable timer 0 interrupts
    
    // TRIG is RC7 - Output
    TRISC = 0b00000000;

    TMR0 = 1;
    
    // Set max oscillation speed
    OSCCAL = 0b01111110;
    // Set PS 1:1
    OPTION = 0b00001000;
}

void main()
{
    init();
    RC4 = 1;
    
    while(1) {
    }
}

void interrupt ISR(void) {
    static int oscillate = 0;
    
    if(INTCON0bits.T0IF == 1) {
        RC4 = oscillate;
        
        oscillate = !oscillate;
        
        INTCON0bits.T0IF = 0;
    }
}
