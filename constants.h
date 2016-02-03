#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <pic16f1828.h>

#define _XTAL_FREQ 500000

#define LED_ON                  1
#define LED_OFF                 0

#define IO_HIGH                 1
#define IO_LOW                  0

// LED Pins
#define PIN_LED_0               RC6

// HCSR04 Pins
#define PIN_US_TRIGGER          RC0
#define PIN_US_ECHO             RA2

// TLC5926 Pins
#define PIN_LED_SDI             RC4
#define PIN_LED_CLK             RC3
#define PIN_LED_LE              RC6
#define PIN_LED_OE              RC7

// LED Array colour bitmap values
#define LIGHT_RED 0x2000
#define LIGHT_YELLOW 0x0800
#define LIGHT_GREEN 0x0200
#define LIGHT_OFF 0x0000

#define LIGHT_THRESH_GREEN  20
#define LIGHT_THRESH_YELLOW 5
#define LIGHT_THRESH_OFFSET 2

// Button pins for EEPROM
#define BTN_SET_YELLOW          RB4
#define BTN_SET_RED             RB5

// Values for Green, yellow and red lighting states
#define STATE_GREEN 0
#define STATE_YELLOW 1
#define STATE_RED 2

#endif	/* CONSTANTS_H */
