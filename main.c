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

#define BAUD_RATE_FAST  19200

void init(void) 
{
    OSCCONbits.SCS = 0b10;
    OSCCONbits.IRCF = 0b1101;
    
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
    // Enable only RC1
    ANSELCbits.ANSELC = 0x01;
    
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
    
    __delay_ms(100);
    
    UART_write_text("BOOT!\r\n");
}

void save_reading()
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
#define FILTER_LEN 5

// Application states
#define APP_STATE_DISPLAY           0
#define APP_STATE_STANDBY           1
#define APP_STATE_CALIB             2
#define APP_STATE_ENTER_DISPLAY     3
#define APP_STATE_ENTER_STANDBY     4
#define APP_STATE_ENTER_CALIB       5

// Calib types
#define APP_CALIB_NONE              0
#define APP_CALIB_YELLOW            1
#define APP_CALIB_RED               2

// Threshold for changing between DISP_STATE_*
#define DISP_THRESH_DIST            4

// Threshold for differences between calibration points
#define CALIB_DISTANCE              5

// Calibration Flashes
#define CALIB_FLASHES               5

// Values for Green, yellow and red lighting states
#define DISP_STATE_INIT             0
#define DISP_STATE_OFF              1
#define DISP_STATE_GREEN            2
#define DISP_STATE_YELLOW           3
#define DISP_STATE_RED              4

// Calculated using: 2.9V / 5V * 1024
#define BATTERY_LOW_ENTER           580
#define BATTERY_LOW_LEAVE           620

#define BATTERY_NORMAL              0
#define BATTERY_LOW                 1

#define MAX_DELAY_UNTIL_READING_COUNT 3

#define HCSR04_TRIG_DELAY_DISPLAY   200
#define HCSR04_TRIG_DELAY_STANDBY   0
#define HCSR04_TRIG_DELAY_CAL       0

// How different does the reading have to be from the standby reading to be valid
#define STANDBY_COUNTER_THRESH      2
// Number of valid readings before transitioning out of standby
#define STANDBY_STABLE_READINGS     3
// Number of stable readings to leave display state
#define DISPLAY_STABLE_READINGS     25

// The number of times the device is allowed to shift back and forth in display mode
// before power saving is enabled
#define SHIFTING_THRESH             12

#define MAX_NO_READING_THRESH       10

void blink_light(uint16_t lightColour, uint8_t flashes) {
    uint8_t i = 0;
    
    for (i = 0; i < flashes; i++) {
        TLC5926_SetLights(lightColour);
        CLRWDT();
        __delay_ms(200);
        CLRWDT();
        TLC5926_SetLights(LIGHT_OFF);
        __delay_ms(200);
        CLRWDT();
    }
}

// Delays in multiples of 10ms until a new reading has occurred
uint16_t delay_until_reading(uint16_t minimumTime) 
{
#define DELAY_TIME  10
#define MAX_COUNTS_UNTIL_ERR   50
    unsigned char tBuf[20] = {0};
    
    int counter = 0;
    for (counter = 0; 
         !((newTimeReading == true) && 
            (counter >= MAX_DELAY_UNTIL_READING_COUNT) &&
            (counter >= (minimumTime/DELAY_TIME))
           ) && (counter < MAX_COUNTS_UNTIL_ERR); 
         counter++) 
    {
        CLRWDT();
        __delay_ms(DELAY_TIME);
//        sprintf(tBuf, "C %d %d %d %d\r\n", counter, timeReading, timeCounterRunning, newTimeReading);
//        UART_write_text(tBuf);
    }
    
    return counter * DELAY_TIME;
}

// Update the lights depending on the state.
void setLights(uint8_t displayState)
{
    if (displayState == DISP_STATE_RED)
        TLC5926_SetLights(LIGHT_RED);
    else if (displayState == DISP_STATE_YELLOW)
        TLC5926_SetLights(LIGHT_YELLOW);
    else if (displayState == DISP_STATE_GREEN)
        TLC5926_SetLights(LIGHT_GREEN);
    else if (displayState == DISP_STATE_OFF)
        TLC5926_SetLights(LIGHT_OFF);
}

void main()
{
    /* Initialise main variables */
    char buf[BUFSIZE];
    
    /* Run init code*/
    init();
    
    // Handle readings within main loop
    bool lastReadingValid = false;
    uint8_t lastReading = 0;
    uint8_t standbyReading = 0;
    
    // Handle filtering readings for calibration
    uint8_t readings[FILTER_LEN] = {0};
    uint8_t cIndex = 0;
    uint8_t filteredReading = 0;
    
    // Application state
    uint8_t appState = APP_STATE_DISPLAY;
    uint8_t appCalibType = APP_CALIB_NONE;
   
    // Display state
    uint8_t displayState = DISP_STATE_INIT;
    uint8_t stableReadingCount = 0;
    
    // Minimum delay time for taking reading
    uint16_t readingDelayTime = HCSR04_TRIG_DELAY_DISPLAY;
    
    // Get a new analogue reading
    bool analogueReadingValid = false;
    bool batteryFlash = true;
    
    // Temporary code for testing
    db.sdb.rangePointYellow = 15;
    db.sdb.rangePointRed = 5;       
    
    /* Trigger the sensor for the first time */
    TLC5926_SetLights(LIGHT_OFF);
    HCSR04_Trigger();
    
    uint8_t noReadingCounter = 0;
    
    while(1) {
        
        // If there's been a new reading, add it to the circular buffer
        if (newTimeReading == true) {
            // Bump the watchdog
            CLRWDT();
            
            lastReadingValid = true;
            lastReading = timeReading;
            
            // Process the reading            
//            sprintf(buf, "R: %d\r\n", lastReading);
//            UART_write_text(buf);
            
            // Clear the new time reading
            newTimeReading = false;
            noReadingCounter = 0;
        } else {
            noReadingCounter++;
        }
        
        // If there have been no readings for 100 iterations, 
        // a problem has occured with the HCSR04
        if (noReadingCounter > MAX_NO_READING_THRESH) 
        {
            blink_light(LIGHT_CENTERS, 5);
            continue;
        }
        
        // If a calibration button has been pressed enter the ENTER_CALIB state.
        if ((appState != APP_STATE_ENTER_CALIB) && 
                (btnRedPressed == true || btnYellowPressed == true))
        {
            // Set the calib type to which button was pressed
            if (btnRedPressed == true)
                appCalibType = APP_CALIB_RED;
            else if (btnYellowPressed == true)
                appCalibType = APP_CALIB_YELLOW;
            
            btnRedPressed = false;
            btnYellowPressed = false;
            
            // Enter the calibration state
            appState = APP_STATE_ENTER_CALIB;
        }
        
        //////////////////////////////////
        // Handle entering the standby state
        //////////////////////////////////
        if (appState == APP_STATE_ENTER_STANDBY && lastReadingValid == true)
        {
            static bool standbyStarted = false;
            
            // If the standby isn't running, start standby
            if (standbyStarted == false)
            {
                // Reset the index
                cIndex = 0;
                // Disable LED's on TLC
                PIN_LED_OE = IO_HIGH;
                // Disable TLC via PIN_TLC_ENABLE
                PIN_ENABLE_TLC5926 = 0;
                // Reading delay time
                readingDelayTime = HCSR04_TRIG_DELAY_STANDBY;
                
                standbyStarted = true;
            } 
            else
            {
                // Update the circular buffer
                readings[cIndex] = lastReading;
                circular_increment_counter(&cIndex, FILTER_LEN);

                // If we've filled the filter, go to the calibration state
                if (cIndex == 0)
                {                    
                    standbyReading = fastMedian5(readings);
                    standbyStarted = false;

                    // If the reading isn't valid
                    if (filteredReading > MAX_COUNTER_VAL)
                    {
                        UART_write_text("STANDBY FAIL: VAL\r\n");
                        blink_light(LIGHT_RED, CALIB_FLASHES);
                        appState = APP_STATE_ENTER_DISPLAY;
                    }
                    // If the reading is valid, move to standby
                    else
                        appState = APP_STATE_STANDBY;
                }
            }

        }
        //////////////////////////////////
        // Handle the standby state
        //////////////////////////////////
        else if (appState == APP_STATE_STANDBY && lastReadingValid == true)
        {
            static uint8_t standbyReadingCounter = 0;
            bool resleep = true;
            
            // Check if absolute difference is greater than threshold and 
            // increment the transition counter if it is, reset it otherwise 
            // and sleep the application
            if (lastReading > 0 && lastReading <= MAX_COUNTER_VAL)
            {                   
                // Was the reading significantly different from the standby reading?
                if (absdiff(lastReading, standbyReading) >= STANDBY_COUNTER_THRESH)
                {
                    standbyReadingCounter++;
                    resleep = false;
                    
                    // If there have been enough valid readings, enter the display state
                    if (standbyReadingCounter >= STANDBY_STABLE_READINGS)
                    {
                        appState = APP_STATE_ENTER_DISPLAY;
                    }
                }
                // If it wasn't reset the counter.
                else
                {
                    standbyReadingCounter = 0;
                }
            }
            
            // If nothing brought us out of sleep, sleep.
            if (appState == APP_STATE_STANDBY && resleep == true)
            {
                
                // Enter sleep mode
                PIN_ENABLE_HCSR04 = 0;
                SLEEP();            
                PIN_ENABLE_HCSR04 = 1;
            }
        }
        //////////////////////////////////
        // Handle entering the display state
        //////////////////////////////////
        else if (appState == APP_STATE_ENTER_DISPLAY)
        {
            // Enable TLC
            PIN_ENABLE_TLC5926 = 1;
            // Re-enable LED's on TLC
            PIN_LED_OE = IO_LOW;
            // Set delay time
            readingDelayTime = HCSR04_TRIG_DELAY_DISPLAY;
            
            // Start the ADC for a low battery reading
            ADCON0bits.GO_nDONE = 1;
            analogueReadingValid = false;
            
            // Reset the battery flash variable
            batteryFlash = true;
            
            setLights(displayState);
            stableReadingCount = 0;
            appState = APP_STATE_DISPLAY;
        }
        //////////////////////////////////
        // Handle the display state
        //////////////////////////////////
        else if (appState == APP_STATE_DISPLAY && lastReadingValid == true)
        {
            uint8_t oldDisplayState = displayState;
            static uint8_t greenYellowTransitionCount = 0;
            static uint8_t yellowRedTransitionCount = 0;
            static uint8_t batteryState = BATTERY_NORMAL;
           
            // Get the ADC reading for low battery
            if (ADCON0bits.GO_nDONE == 0 && analogueReadingValid == false)
            {
                uint16_t analog = (ADRESHbits.ADRESH << 8) | ADRESLbits.ADRESL;
                PIR1bits.ADIF = 0;
                
                // If we're below the low battery enter threshold, then set the
                // battery state.
                if (batteryState == BATTERY_NORMAL && analog < BATTERY_LOW_ENTER)
                    batteryState = BATTERY_LOW;
                else if (batteryState == BATTERY_LOW && analog > BATTERY_LOW_LEAVE)
                    batteryState = BATTERY_NORMAL;
                
                sprintf(buf, "A: %u\r\n", analog);
                UART_write_text(buf);
                
                analogueReadingValid = true;
            }
            
            // Handle initial state of scale
            if (oldDisplayState == DISP_STATE_INIT) 
            {
                // Set the initial state to whatever the first reading is
                if (lastReading > db.sdb.rangePointYellow)
                    displayState = DISP_STATE_GREEN;
                else if (lastReading > db.sdb.rangePointRed)
                    displayState = DISP_STATE_YELLOW;
                else
                    displayState = DISP_STATE_RED;
            }
            // If the display state is green
            else if (oldDisplayState == DISP_STATE_GREEN) 
            {
                // If the counter is within the yellow threshold, transition
                if (lastReading < db.sdb.rangePointYellow) 
                {
                    displayState = DISP_STATE_YELLOW;
                    greenYellowTransitionCount++;
                    yellowRedTransitionCount = 0;
                }
            
            } 
            // If the state is yellow
            else if (oldDisplayState == DISP_STATE_YELLOW) 
            {
                // If the state is within the red threshold, transition
                if (lastReading < db.sdb.rangePointRed) 
                {
                    displayState = DISP_STATE_RED;
                    yellowRedTransitionCount++;
                }
                // If the state is outside the green threshold, transition
                else if (lastReading > (db.sdb.rangePointYellow + DISP_THRESH_DIST))
                {
                    displayState = DISP_STATE_GREEN;                
                    greenYellowTransitionCount++;
                }
            } 
            // If the state is red
            else if (oldDisplayState == DISP_STATE_RED) 
            {
                // If the state is within the yellow threshold, transition
                if (lastReading > (db.sdb.rangePointRed + DISP_THRESH_DIST))
                {
                    displayState = DISP_STATE_YELLOW;
                    yellowRedTransitionCount++;
                    greenYellowTransitionCount = 0;
                }
            }
            
            // If the led state hasn't been changed
            if (displayState == oldDisplayState)
            {
                stableReadingCount++;
                
                // If this has reached the threshold, move to powersaving
                if (stableReadingCount == DISPLAY_STABLE_READINGS)
                {
                    appState = APP_STATE_ENTER_STANDBY;
                    greenYellowTransitionCount = 0;
                    yellowRedTransitionCount = 0;
                }
                // else if the battery is low
                else if (batteryState == BATTERY_LOW)
                {
                    batteryFlash = !batteryFlash;
                    if (batteryFlash == true)
                        setLights(displayState);
                    else
                        setLights(DISP_STATE_OFF);
                }


            }
            else if (greenYellowTransitionCount > SHIFTING_THRESH || 
                    yellowRedTransitionCount > SHIFTING_THRESH)
            {
                appState = APP_STATE_ENTER_STANDBY;
                greenYellowTransitionCount = 0;
                yellowRedTransitionCount = 0;
            }
            else 
            {
                stableReadingCount = 0;
                setLights(displayState);
                batteryFlash = true;
            }
        }
        //////////////////////////////////
        // Handle entering the calibration state
        //////////////////////////////////
        else if (appState == APP_STATE_ENTER_CALIB && lastReadingValid == true)
        {
            static bool calibrationRunning = false;
            
            // If the calibration isn't running, start
            if (calibrationRunning == false)
            {
                // Reset the cIndex to 0
                cIndex = 0;
                // Enable TLC
                PIN_ENABLE_TLC5926 = 1;
                // Re-enable LED's on TLC
                PIN_LED_OE = IO_LOW;
                // Set delay time
                readingDelayTime = HCSR04_TRIG_DELAY_CAL;
                
                calibrationRunning = true;
            }
            // If the calibration is running
            else
            {
                // Update the circular buffer
                readings[cIndex] = lastReading;
                circular_increment_counter(&cIndex, FILTER_LEN);
                
                // If we've filled the filter, go to the calibration state
                if (cIndex == 0)
                {
                    filteredReading = fastMedian5(readings);
                    calibrationRunning = false;
                    // If the reading isn't valid
                    if (filteredReading > MAX_COUNTER_VAL)
                    {
                        UART_write_text("CAL FAIL: VAL\r\n");
                        blink_light(LIGHT_RED, CALIB_FLASHES);
                        appState = APP_STATE_ENTER_DISPLAY;
                    }
                    // If the reading is valid, calibrate it!
                    else
                        appState = APP_STATE_CALIB;
                }
            }
        }
        //////////////////////////////////
        // Handle the calibration state
        //////////////////////////////////
        else if (appState == APP_STATE_CALIB)
        {
            // If the red button was pressed.
            if (appCalibType == APP_CALIB_RED) {
                // Check if the distance is outside of the calib range from 
                // yellow
                if (absdiff(db.sdb.rangePointYellow, filteredReading) > CALIB_DISTANCE) {
                    db.sdb.rangePointRed = filteredReading;
                    sprintf(buf, "P RED: %d\r\n", db.sdb.rangePointRed);
                    UART_write_text(buf);
                    blink_light(LIGHT_GREEN, CALIB_FLASHES);
                }
                else {
                    sprintf(buf, "PF RED: %d %d\r\n", db.sdb.rangePointYellow, filteredReading);
                    UART_write_text(buf);
                    blink_light(LIGHT_RED, CALIB_FLASHES);
                }
            }
            // If the yellow button was pressed.
            if (appCalibType == APP_CALIB_YELLOW) {
                // Check if the distance is outside of the calib range from 
                // red
                if (absdiff(db.sdb.rangePointRed, filteredReading) > CALIB_DISTANCE) {
                    db.sdb.rangePointYellow = filteredReading;
                    sprintf(buf, "P YEL: %d\r\n", db.sdb.rangePointYellow);
                    UART_write_text(buf);
                    blink_light(LIGHT_GREEN, CALIB_FLASHES);
                }
                else {
                    sprintf(buf, "PF YEL: %d %d\r\n", db.sdb.rangePointRed, filteredReading);
                    UART_write_text(buf);
                    blink_light(LIGHT_RED, CALIB_FLASHES);
                }
            }
            db_save();
            appState = APP_STATE_ENTER_DISPLAY; 
        }
        
        // We're done with the reading for this iteration of the application,
        // so set the reading as invalid.
        lastReadingValid = false;

        HCSR04_Trigger();
        sprintf(buf, "D: %d\r\n", delay_until_reading(readingDelayTime));
//        UART_write_text(buf);
    }
}
