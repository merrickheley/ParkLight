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
#include "uart.h"

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

typedef struct Global_State {
    bool echoHit;
    bool setYellow;
    bool setRed;
    uint8_t counter;
    uint8_t reading;
    uint8_t newReading;
} State;

typedef struct Led_State {
    uint8_t state; // Red by default
    uint8_t turnoffCounter; // How many consecutive instances of a colour
} LedState;

volatile State state = { 0 }; 
LedState led_state = { 0 };

void display_LED(LedState *ledState, uint8_t reading);
void blink_light(uint16_t lightColour, uint8_t flashes);

void circular_increment_counter(uint8_t *cnt, uint8_t max)
{   
    (*cnt)++;
    if (*cnt == max)
        *cnt = 0;
}

void swap(uint8_t *a, uint8_t *b)
{
	uint8_t tmp = *b;
	*b = *a;
	*a = tmp;
}

uint8_t fastMedian5(uint8_t *buf)
{
    uint8_t arr[5] = {0};
    
    // Put the numbers in an array
    memcpy(arr, buf, 5);
    
    // Ensure a[0] < a[1], a[0] < a[3], a[3] < a[4]
    if (arr[0] > arr[1]) swap(&arr[0], &arr[1]);
    if (arr[3] > arr[4]) swap(&arr[3], &arr[4]);
    if (arr[0] > arr[3]) {
    	swap(&arr[0], &arr[3]);
    	swap(&arr[1], &arr[4]);
    }
    
    if (arr[2] > arr[1]) {
    	if (arr[1] < arr[3]) {
    		if (arr[2] < arr[3])
                return arr[2];
            else
                return arr[3];
        } else {
            if (arr[1] < arr[4])
                return arr[1];
            else
                return arr[4];
        }
    } else {
    	if (arr[2] > arr[3]) {
    		if (arr[2] < arr[4])
                return arr[2];
            else 
                return arr[4];
        } else {
            if (arr[1] < arr[3])
                return arr[1];
            else 
                return arr[3];
        }
    }
}

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
    
    // Enable RB5 and RB4 falling edge
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
    
    TLC5926_init();
    
    // Baud rates above 19200 didn't work
    UART_init(19200, true, false);
    
    // Drive it low to turn LED's on.
    PIN_LED_OE = IO_LOW;
}

#define FILTER_LEN 5
#define LIGHT_FLASHES 5
#define BUFSIZE 50

void main()
{
    uint8_t readings[FILTER_LEN] = {0};
    uint8_t cIndex = 0;
    char buf[BUFSIZE];
    init();

    TLC5926_SetLights(LIGHT_OFF);
    
    while(1) {
        
        // If there's been a new reading, add it to the circular buffer
        if (state.newReading == true) {
            readings[cIndex] = state.reading;
            sprintf(buf, "FIL %d %d: %d %d %d %d %d\r\n", cIndex, state.reading, 
                    readings[0], readings[1], readings[2], 
                    readings[3], readings[4]);
            circular_increment_counter(&cIndex, FILTER_LEN);
            UART_write_text(buf);
            state.newReading = false;
            //blink_light(LIGHT_GREEN, 1);
        }
        
        // If the red button has been pressed.
        // TODO: Should we clear the readings index when the button is pressed
        // and get the median of 5 new readings?
        if (state.setRed == true) {
            db.sdb.rangePointRed = fastMedian5(readings);
            db_save();
            sprintf(buf, "P RED: %d\r\n", db.sdb.rangePointRed);
            UART_write_text(buf);
            blink_light(LIGHT_RED, LIGHT_FLASHES);
            state.setRed = false;
        }
        
        // If the yellow button has been pressed.
        if (state.setYellow == true) {
            db.sdb.rangePointYellow = fastMedian5(readings);
            db_save();
            sprintf(buf, "P YEL: %d\r\n", db.sdb.rangePointYellow);
            UART_write_text(buf);
            blink_light(LIGHT_YELLOW, LIGHT_FLASHES);
            state.setYellow = false;
        }
        
        display_LED(&led_state, readings[cIndex]);
        HCSR04_Trigger();
        __delay_ms(1000);
    }
}

void save_reading(void)
{
    // Set edge tracker low and save counter
    state.echoHit = false;
    state.reading = state.counter;
    state.newReading = true;
}

void interrupt ISR(void) 
{
    // Edge detected on echo pin
    if (IOCAFbits.IOCAF2)
    {
        PIN_LED_0 = 1;

        // If echo pin input is rising edge
        if (PIN_US_ECHO == IO_HIGH && state.echoHit == false) {
            // Set edge tracker high and reset counter
            state.echoHit = true;
            state.counter = 0;
        }
        
        // If echo pin is falling edge
        if (PIN_US_ECHO == IO_LOW && state.echoHit == true) {
            save_reading();
        }
        
        // Clear all individual IOC bits to continue
        IOCAFbits.IOCAF2 = 0;
	}
    
    // Yellow button falling edge
    if (IOCBFbits.IOCBF4) {
        state.setYellow = true;
        IOCBFbits.IOCBF4 = 0;
    }
    
    // Red button falling edge
    if (IOCBFbits.IOCBF5) {
        state.setRed = true;
        IOCBFbits.IOCBF5 = 0;
    }
    
    // Timer 0
    if (INTCONbits.TMR0IF && INTCONbits.TMR0IE) {
        
		// Increment counter if global echo bit is set
		if (state.echoHit == true) {
			state.counter++;
            // Handle overflow
            if (state.counter > MAX_COUNTER_VAL)
                save_reading();
		}       
        
        INTCONbits.TMR0IF = 0;
    }
    
    // TIMER 1
    if (PIR1bits.TMR1IF && PIE1bits.TMR1IE) {    
        PIR1bits.TMR1IF = 0;    
    }
}

void display_LED(LedState *ledState, uint8_t reading) 
{       
    // If the state is green
    if (ledState->state == STATE_GREEN) {
        // If the counter is within the yellow threshold, transition
        if (reading < db.sdb.rangePointYellow) {
            ledState->state = STATE_YELLOW;
            TLC5926_SetLights(LIGHT_YELLOW);
        }
    // If the state is yellow
    } else if (ledState->state == STATE_YELLOW) {
        // If the state is within the red threshold, transition
        if (reading < db.sdb.rangePointRed) {
            ledState->state = STATE_RED;
            TLC5926_SetLights(LIGHT_RED);
        }
        // If the state is outside the green threshold, transition
        else if (reading > db.sdb.rangePointYellow + 5) {
            ledState->state = STATE_GREEN;
            TLC5926_SetLights(LIGHT_GREEN);                
        }
    // If the state is red
    } else if (ledState->state == STATE_RED) {
        // If the state is within the yellow threshold, transition
        if (reading > (db.sdb.rangePointRed + LIGHT_THRESH_OFFSET)) {
            ledState->state = STATE_YELLOW;
            TLC5926_SetLights(LIGHT_YELLOW);
            ledState->turnoffCounter = 0;
        } 
        // If there is no change in state
        else {
            // Increment the state counter
            ledState->turnoffCounter++;
            // After approx five seconds of red
            if (ledState->turnoffCounter > 5) { 
                TLC5926_SetLights(LIGHT_OFF);
            }
        }
    }
}

void blink_light(uint16_t lightColour, uint8_t flashes) {
    uint8_t i = 0;

    for (i = 0; i < flashes; i++) {
        TLC5926_SetLights(lightColour);
        __delay_ms(200);
        TLC5926_SetLights(LIGHT_OFF);
        __delay_ms(200);
    }
}
