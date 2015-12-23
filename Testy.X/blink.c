/* 
 * File:   blink.c
 * Author: Jonathan Holland
 *
 * Created on 23 December 2015, 7:00 PM
 */

 #pragma config FOSC = INTOSC    // Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
 #pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)    
 #pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)    
 #pragma config MCLRE = OFF      // MCLR Pin Function Select (MCLR/VPP pin function is digital input)    
 #pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)    
 #pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)    
 #pragma config BOREN = OFF      // Brown-out Reset Enable (Brown-out Reset disabled)    
 #pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)    
 #pragma config IESO = ON        // Internal/External Switchover (Internal/External Switchover mode is enabled)    
 #pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)    
 
 // CONFIG2    
 #pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)    
 #pragma config PLLEN = ON       // PLL Enable (4x PLL enabled)    
 #pragma config STVREN = OFF     // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)    
 #pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)    
 #pragma config LVP = OFF        // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)    
 
#include <xc.h>
#include <htc.h>
#define _XTAL_FREQ 8000000
#include <stdio.h>
#include <stdlib.h>
#include <pic16f1828.h>

#define TRUE 1
#define FALSE 0

void init(void) 
{
    INTCONbits.GIE = 1; // Enable global interrupts
    INTCONbits.PEIE = 1; // Enable peripheral interrupts
    INTCONbits.T0IE = 1;

    // Enable each timer
    INTCONbits.TMR0IE = 1;
    PIE1bits.TMR1IE = 1; 
    PIE1bits.TMR2IE = 1;
    PIE3bits.TMR4IE = 1;
    PIE3bits.TMR6IE = 1;
    
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
    TRISC = 0b00000000; // Outputs for RC2 and RC6
    TRISB = 0b11110000; // Input for RB5

    
}

void main()
{
    init();
    
    while(1) {
    }
}

void interrupt ISR(void) 
{
    static int oscillate = 0;
    
    // TIMER 0
    if (INTCONbits.TMR0IF && INTCONbits.TMR0IE) {
        
        INTCONbits.TMR0IF = 0;
    }
    
    // TIMER 1
    if (PIR1bits.TMR1IF && PIE1bits.TMR1IE) {    
                        
        RC6 = oscillate;
        oscillate = !oscillate;
        
        PIR1bits.TMR1IF = 0;    
    }  
    // TIMER 2
    if (PIR1bits.TMR2IF && PIE1bits.TMR2IE) {    
        
        PIR1bits.TMR2IF = 0;    
    }    
    // TIMER 3
    if (PIR3bits.TMR4IF && PIE3bits.TMR4IE) {
        
        PIR3bits.TMR4IF = 0;
    }
    // TIMER 4
    if (PIR3bits.TMR6IF && PIE3bits.TMR6IE) {
        
        PIE3bits.TMR6IE = 0;
    }
}  // End of isr()
