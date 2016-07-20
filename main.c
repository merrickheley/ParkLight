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

typedef struct Global_State {
    bool echoHit;
    bool setYellow;
    bool setRed;
    uint8_t counter;
    uint8_t reading;
    uint8_t newReading;
} State;

volatile State state = { 0 }; 
LedState led_state = { 0 };

#define CLOCK_INTERNAL 0b10
#define CLOCK_EXTERNAL 0b00
#define SET_CLOCK(state) (OSCCONbits.SCS = state)

#define BAUD_RATE_FAST  4800
#define BAUD_RATE_SLOW  300

void init(void) 
{
    // Power Saving mode is off by default
    OSCCONbits.IRCF = 0b0111; // 500KHz internal oscillator
    SET_CLOCK(CLOCK_EXTERNAL);
    
    // Set the time-out period of the WDT
    WDTCONbits.WDTPS = 0b01010; // 1s typical time-out period

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
    state.echoHit = false;
    state.reading = state.counter;
    state.newReading = true;
}

void interrupt ISR(void) 
{
    // Edge detected on echo pin
    if (IOCAFbits.IOCAF2)
    {

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
}

#define FILTER_LEN 5
#define LIGHT_FLASHES 5
#define BUFSIZE 50

#define MAIN_STATE_PASSIVE          1 // reading is 500KHz internal
#define MAIN_STATE_CALIBRATION      2
#define MAIN_STATE_DISPLAY          3 // calibration/display are at 4MHz external

#define CALIB_STATE_RED             0
#define CALIB_STATE_YELLOW          1

#define POWER_SAVING_COUNTER_THRESH  2

#define HCSR04_TRIG_DELAY_MIN       200
#define HCSR04_TRIG_DELAY_SLOW      1000

#define DELAY_SCALING_FACTOR_READING 8

#define HCSR04_TRIG_DELAY_READING_MIN  (HCSR04_TRIG_DELAY_MIN/DELAY_SCALING_FACTOR_READING)
#define HCSR04_TRIG_DELAY_READING_SLOW (HCSR04_TRIG_DELAY_SLOW/DELAY_SCALING_FACTOR_READING)

void app_sleep(void)
{
    UART_write_text("E: PSave\r\n");
    // Enter sleep mode
    PIN_ENABLE_HCSR04 = 0;
    SLEEP(); // XC8 compiler version of the asm sleep command
    PIN_ENABLE_HCSR04 = 1;
}

void enter_passive(uint8_t *stateVar, uint8_t *cIndex, uint8_t *psReading) 
{
    *cIndex = 0;
    *psReading = 0;
    
    OSCCONbits.IRCF = 0b0111; // 500KHz Internal
    SET_CLOCK(CLOCK_INTERNAL);
    
    HCSR04_Trigger(true);
    __delay_ms(HCSR04_TRIG_DELAY_READING_MIN);
    *stateVar = MAIN_STATE_PASSIVE;
    UART_init(BAUD_RATE_SLOW, _XTAL_FREQ_READING, true, false);
    
    UART_write_text("E: Pas\r\n");
    
    // Disable LED's on TLC
    PIN_LED_OE = IO_HIGH;
    // Disable TLC via PIN_TLC_ENABLE
    PIN_ENABLE_TLC5926 = 0;
}

void enter_calibration(uint8_t *stateVar, uint8_t *cIndex, volatile State *state, 
        uint8_t *calibState)
{
    // Enable TLC
    PIN_ENABLE_TLC5926 = 1;
    // Re-enable LED's on TLC
    PIN_LED_OE = IO_LOW;

    *cIndex = 0;
    if (state->setRed == true)
        *calibState = CALIB_STATE_RED;
    else
        *calibState = CALIB_STATE_YELLOW;
    SET_CLOCK(CLOCK_EXTERNAL);
    
    UART_write_text("E: cal\r\n");
    
    *stateVar = MAIN_STATE_CALIBRATION;
    UART_init(BAUD_RATE_FAST, _XTAL_FREQ, true, false);
}

void enter_display(uint8_t *stateVar)
{
    // Enable TLC
    PIN_ENABLE_TLC5926 = 1;
    // Re-enable LED's on TLC
    PIN_LED_OE = IO_LOW;
    
    SET_CLOCK(CLOCK_EXTERNAL);
    
    UART_write_text( "E: disp\r\n");
    
    HCSR04_Trigger(false);
    __delay_ms(HCSR04_TRIG_DELAY_MIN);
    
    *stateVar = MAIN_STATE_DISPLAY;
    UART_init(BAUD_RATE_FAST, _XTAL_FREQ, true, false);
}

void main()
{
    uint8_t readings[FILTER_LEN] = {0};
    uint8_t cIndex = 0;
    char buf[BUFSIZE];
    uint8_t stateVar = MAIN_STATE_DISPLAY;
    uint8_t calibState = CALIB_STATE_RED;
    uint8_t curReading = 0;
    uint8_t psReading = 0;
    uint8_t temp = 0;
    init();

    TLC5926_SetLights(LIGHT_RED);
    HCSR04_Trigger(false);
    
    while(1) {

        // If there's been a new reading, add it to the circular buffer
        if (state.newReading == true) {
            // Bump the watchdog
            CLRWDT();
            
            // Process the reading
            readings[cIndex] = state.reading;
            curReading = readings[cIndex];
            sprintf(buf, "R: %d\r\n", curReading);
//            sprintf(buf, "FIL %d %d: %d %d %d %d %d\r\n", cIndex, curReading, 
//                    readings[0], readings[1], readings[2], 
//                    readings[3], readings[4]);
            UART_write_text(buf);
            circular_increment_counter(&cIndex, FILTER_LEN);
            // Clear new reading at end of while loop
            if (stateVar != MAIN_STATE_DISPLAY) 
            {
                state.newReading = false;
            }
        }
        
        // If a calibration button has been pressed reset the circular buffer
        // and enter the calibration state.
        if (state.setRed == true || state.setYellow == true)
        {
            enter_calibration(&stateVar, &cIndex, &state, &calibState);
            state.setRed = false;
            state.setYellow = false;
        }
        
        if (stateVar == MAIN_STATE_PASSIVE)
        {
            // Wait for the filter to fill for the first time
            if (cIndex == (FILTER_LEN-1) && psReading == 0)
                psReading = fastMedian5(readings);
            
            // If the reading has been set and the threshold has been met
            if (psReading > 0 && absdiff(state.reading, psReading) >= POWER_SAVING_COUNTER_THRESH)
                enter_display(&stateVar);
            else if (psReading > 0) 
            {
                cIndex = 0;
                psReading = 0;
                app_sleep();
            }
            else
            {
                UART_write_text("TRIG\r\n");
                HCSR04_Trigger(true);
                __delay_ms(HCSR04_TRIG_DELAY_READING_MIN);
            }
        }
        
        else if (stateVar == MAIN_STATE_CALIBRATION)
        {
            // Wait for the filter to fill
            if (cIndex == (FILTER_LEN-1))
            {
                #define CALIB_DISTANCE  5
                // If the red button was pressed.
                if (calibState == CALIB_STATE_RED) {
                    temp = fastMedian5(readings);
                    // Check if the distance is outside of the calib range from 
                    // yellow
                    if (absdiff(db.sdb.rangePointYellow, temp) > CALIB_DISTANCE) {
                        db.sdb.rangePointRed = temp;
                        sprintf(buf, "P RED: %d\r\n", db.sdb.rangePointRed);
                        UART_write_text(buf);
                        blink_light(LIGHT_GREEN, LIGHT_FLASHES);
                    }
                    else {
                        sprintf(buf, "PF RED: %d %d\r\n", db.sdb.rangePointYellow, temp);
                        UART_write_text(buf);
                        blink_light(LIGHT_RED, LIGHT_FLASHES);
                    }
                }
                // If the yellow button was pressed.
                if (calibState == CALIB_STATE_YELLOW) {
                    temp = fastMedian5(readings);
                    // Check if the distance is outside of the calib range from 
                    // red
                    if (absdiff(db.sdb.rangePointRed, temp) > CALIB_DISTANCE) {
                        db.sdb.rangePointYellow = temp;
                        sprintf(buf, "P YEL: %d\r\n", db.sdb.rangePointYellow);
                        UART_write_text(buf);
                        blink_light(LIGHT_GREEN, LIGHT_FLASHES);
                    }
                    else {
                        sprintf(buf, "PF YEL: %d %d\r\n", db.sdb.rangePointRed, temp);
                        UART_write_text(buf);
                        blink_light(LIGHT_RED, LIGHT_FLASHES);
                    }
                }
                db_save();
                enter_passive(&stateVar, &cIndex, &psReading);
            }
            // If filter isn't full, get more values
            else
            {
                HCSR04_Trigger(false);
                __delay_ms(HCSR04_TRIG_DELAY_MIN);
            }                   
        }
        // In the display state, drive the display as normal
        else if (stateVar == MAIN_STATE_DISPLAY)
        {           
            // Update the display
            if (state.newReading == true)
            {
                display_LED(&led_state, curReading, 
                      (uint8_t) db.sdb.rangePointYellow,
                      (uint8_t) db.sdb.rangePointRed);
                
                #define LED_TIMER_OFF_THRESH 25

                // If we've exceeded the turnoff counter then
                // save the current state, and transition to power saving.
                if (led_state.turnoffCounter > LED_TIMER_OFF_THRESH)
                {
                    led_state.offState = led_state.state;
                    led_state.offReading = curReading;
                    led_state.state = DISP_STATE_OFF;
                    enter_passive(&stateVar, &cIndex, &psReading);
                    continue;
                }
                state.newReading = false;
            }
            HCSR04_Trigger(false);
            __delay_ms(HCSR04_TRIG_DELAY_MIN);
        } 
    }
}
