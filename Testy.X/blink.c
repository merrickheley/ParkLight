/* 
 * File:   blink.c
 * Author: Jonathan Holland
 *
 * Created on 1 December 2015, 8:52 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <pic16f570.h>
#include <htc.h>

void init(void) {
    INTCON0bits.GIE = 1; //Enable INTs globally
    INTCON1bits.T0IE = 1; // Enable timer 0 interrupts
    INTCON1bits.RBIE = 1; // Enable port b interrupts
    INTCON1bits.CWIE = 0; // Disable comparators
    
    // TRIG is RC7 - Output
    TRISC = 0b00000000;
    // ECHO is RB3 - Input - RB Interrupt trigger
    TRISB = 0b11111111;

    ANSEL = 0;
    ADCON0 = 0;
    ADRES = 0;
    CM1CON0 = 0;      
    CM2CON0 = 0;
    VRCON = 0;
    OPACON = 0;
    
    OSCCAL = 0b01111110;
    OPTION = 0b00001000;
    PORTC = 0b00000000;
}


void main()
{
    init();
    while(1) {
        //RC4 = 1;
        PORTC = 0b11111111;
        if(INTCON0bits.T0IF == 1) {
            RC7 = 1;
            unsigned char dummy  = 0;
            unsigned char dummyTwo = 1;
            dummy  = 0;
            dummyTwo = 1;
            dummy  = 0;
            dummyTwo = 1;
            dummy  = 0;
            dummyTwo = 1;
            RC7 = 0;
            
            unsigned char readValue = RB3;
            INTCON0bits.T0IF = 0;
        }
    }
}