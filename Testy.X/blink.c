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

unsigned char overflowNum = 0; //Overflow counter

void init(void) {
    PSA=0;      //Timer Clock Source is from Prescaler
    T0CS=0;     //Prescaler gets clock from FCPU (5MHz)
    GIE=1;      //Enable INTs globally

    //Set RB1 as output because we have LED on it
    TRISB&=0B11111101;
    TRISC = 0x00; // Output - LED 2
       
    //RC4 = 0xFF;
    //PORTCbits.RC4 = 1;
}


void main()
{
    init();
    while(1);   //Sit Idle Timer will do every thing!
}

//Main Interrupt Service Routine (ISR)
void interrupt ISR()
{
}