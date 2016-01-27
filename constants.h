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
#define PIN_US_TRIGGER          RC2
#define PIN_US_ECHO             RB5

// TLC5926 Pins
// TODO: Update these to be accurate
#define PIN_LED_SDI             RC4
#define PIN_LED_CLK             RC3
#define PIN_LED_LE              RC6
#define PIN_LED_OE              RC7

// Button pins for EEPROM
#define PIN_SET_YELLOW          RA4
#define PIN_SET_RED             RC5

#endif	/* CONSTANTS_H */
