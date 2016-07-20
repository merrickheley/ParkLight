#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <pic16f1828.h>

#define _XTAL_FREQ              4000000
#define _XTAL_FREQ_READING      500000

#define LED_ON                  1
#define LED_OFF                 0

#define IO_HIGH                 1
#define IO_LOW                  0

// Pins to enable peripherals
#define PIN_ENABLE_TLC5926      RC5
#define PIN_ENABLE_HCSR04       RC0

// HCSR04 Pins
#define PIN_US_TRIGGER          RC2
#define PIN_US_ECHO             RA2

// TLC5926 Pins
#define PIN_LED_SDI             RC7
#define PIN_LED_CLK             RC6
#define PIN_LED_LE              RC3
#define PIN_LED_OE              RC4

// LED Array colour bitmap values
#define LIGHT_RED               0x001F
#define LIGHT_YELLOW            0x7C00
#define LIGHT_GREEN             0x03E0
#define LIGHT_OFF               0x0000

#define LIGHT_THRESH_OFFSET     2

// Button pins for EEPROM
#define BTN_SET_YELLOW          RB4
#define BTN_SET_RED             RB5

// Values for Green, yellow and red lighting states
#define DISP_STATE_GREEN             0
#define DISP_STATE_YELLOW            1
#define DISP_STATE_RED               2

#endif	/* CONSTANTS_H */
