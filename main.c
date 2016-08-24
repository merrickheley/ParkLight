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
#include "display.h"
#include "uart.h"
#include "utils.h"

// C libraries
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// PIC includes
#include <xc.h>
#include <htc.h>
#include <pic16f1828.h>

#define MAX_COUNTER_VAL 100

volatile bool btnYellowPressed = false;
volatile bool btnRedPressed = false;

volatile bool timeCounterRunning = false;
volatile uint8_t timeCounter = 0;
volatile uint8_t timeReading = 0;
volatile bool newTimeReading = false;

LedState led_state = { 0 };

#define BAUD_RATE_FAST  19200

void init(void) 
{
    OSCCONbits.SCS = 0b00;
    
    // Set the time-out period of the WDT
    WDTCON = 0b00010011; // 512ms typical time-out period

    INTCONbits.GIE = 1; // Enable global interrupts
    INTCONbits.PEIE = 1; // Enable peripheral interrupts
    INTCONbits.T0IE = 1; // Enable timer interrupts
    
    // Enable IOC
    INTCONbits.IOCIE = 1;
    
    // Enable RA2 rising edge
    IOCAPbits.IOCAP2 = 1;
    // Enable RA2 falling edge
    IOCANbits.IOCAN2 = 1;
    
    // Enable RB5 and RB4 falling edge
    IOCBNbits.IOCBN4 = 1;
    IOCBNbits.IOCBN5 = 1;

    // Enable each timer
    INTCONbits.TMR0IE = 1;
    
    // Weak pull-up enabled on ECHO pin
    WPUAbits.WPUA2 = 1;
    
    // Set timer0 prescaler to 1:1
    OPTION_REG = 0b00000000;
    
    TRISA = 0b11111111; // Inputs
    TRISB = 0b11110000; // Inputs
    TRISC = 0b00000010; // Outputs, except for RC1 as an input.

    // Disable analogue inputs. This should set all pins to digital.
    ANSELBbits.ANSB4 = 0;
    ANSELBbits.ANSB5 = 0;
    ANSELAbits.ANSELA = 0x00;
    ANSELCbits.ANSELC = 0x00;
    
    // Enable RC1 as analogue input
    ANSELCbits.ANSC1 = 1;
    
    // Configure the ADC for battery
    ADCON1bits.ADCS = 0b000;            // F_osc/64. Slow as possible.
    ADCON1bits.ADNREF = 0;              // V_ref- is connected to Vss
    ADCON1bits.ADPREF = 0;              // V_ref+ is connected to Vdd
    ADCON1bits.ADFM = 1;                // Right justify A/D result 
    ADCON0bits.CHS = 0b00101;           // Enable AN5
    ADCON0bits.ADON = 1;                // Turn on the ADC
    
    // Set outputs to low initially
    PORTC = 0x00; 
    
    // Initialise the database so that it is populated
    db_init(); 
    
    // Enable RC5 to TLC
    PIN_ENABLE_TLC5926 = 1;
    
    UART_init(BAUD_RATE_FAST, _XTAL_FREQ, true, false);
    TLC5926_init();
    
    // Drive it low to turn LED's on.
    PIN_LED_OE = IO_LOW;
    
    // Enable RC0 to HCSR
    PIN_ENABLE_HCSR04 = 1;
    
    UART_write_text("BOOT!\r\n");
}

void save_reading(void)
{
    // Set edge tracker low and save counter
    timeCounterRunning = false;
    timeReading = timeCounter;
    newTimeReading = true;
}

void interrupt ISR(void) 
{
    // Edge detected on echo pin
    if (IOCAFbits.IOCAF2)
    {

        // If echo pin input is rising edge
        if (PIN_US_ECHO == IO_HIGH && timeCounterRunning == false) {
            // Set edge tracker high and reset counter
            timeCounterRunning = true;
            timeCounter = 0;
        }
        
        // If echo pin is falling edge
        if (PIN_US_ECHO == IO_LOW && timeCounterRunning == true) {
            save_reading();
        }
        
        // Clear all individual IOC bits to continue
        IOCAFbits.IOCAF2 = 0;
	}
    
    // Yellow button falling edge
    if (IOCBFbits.IOCBF4) {
        btnYellowPressed = true;
        IOCBFbits.IOCBF4 = 0;
    }
    
    // Red button falling edge
    if (IOCBFbits.IOCBF5) {
        btnRedPressed = true;
        IOCBFbits.IOCBF5 = 0;
    }
    
    // Timer 0
    if (INTCONbits.TMR0IF && INTCONbits.TMR0IE) {
        
            // Increment counter if global echo bit is set
            if (timeCounterRunning == true) {
                timeCounter++;
                // Handle overflow
                if (timeCounter > MAX_COUNTER_VAL)
                    save_reading();
            }       

        INTCONbits.TMR0IF = 0;
    }
}

#define BUFSIZE 50

// Calculated using: 2.9V / 5V * 1024
#define BATTERY_LOW_ENTER           590
#define BATTERY_LOW_LEAVE           610

#define BATTERY_NORMAL              0
#define BATTERY_LOW                 1

#define MAX_DELAY_UNTIL_READING_COUNT 3

#define HCSR04_TRIG_DELAY_MIN       200

// Delays in multiples of 10ms until a new reading has occurred
uint16_t delay_until_reading(uint16_t minimumTime) 
{
#define DELAY_TIME  10
    
    int counter = 0;
    for (counter = 0; 
         !((newTimeReading == true) && 
            (counter >= MAX_DELAY_UNTIL_READING_COUNT) &&
            (counter >= (minimumTime/DELAY_TIME))); 
         counter++) 
    {
        CLRWDT();
        __delay_ms(DELAY_TIME);
    }
    
    return counter * DELAY_TIME;
}

void main()
{
    /* Initialise main variables */
    char buf[BUFSIZE];
    
    /* Run init code*/
    init();
    
    /* Set the lights to red then trigger the sensor for the first time */
    TLC5926_SetLights(LIGHT_RED);
    HCSR04_Trigger();
    
    while(1) {
        
        // If there's been a new reading, add it to the circular buffer
        if (newTimeReading == true) {
            // Bump the watchdog
            CLRWDT();
            
            // Process the reading            
            sprintf(buf, "R: %d\r\n", timeReading);
            UART_write_text(buf);
            
            // Clear the new time reading
            newTimeReading = false;
        }

        HCSR04_Trigger();
        sprintf(buf, "D: %d\r\n", delay_until_reading(1000));
        UART_write_text(buf);
    }
}
