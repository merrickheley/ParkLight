#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <pic16f1828.h>

#define _XTAL_FREQ              4000000

#define ADC_LOW_BATTERY_ALARM   615

#define LED_ON                  1
#define LED_OFF                 0

#define IO_HIGH                 1
#define IO_LOW                  0

// Pins to enable peripherals
#define PIN_ENABLE_TLC5926      LATCbits.LATC5
#define PIN_ENABLE_HCSR04       LATCbits.LATC0

// Analogue battery check
#define PIN_BATTERY             RC1

// HCSR04 Pins
#define PIN_US_TRIGGER          LATCbits.LATC2
#define PIN_US_ECHO             RA2

// TLC5926 Pins
#define PIN_LED_SDI             LATCbits.LATC7
#define PIN_LED_CLK             LATCbits.LATC6
#define PIN_LED_LE              LATCbits.LATC3
#define PIN_LED_OE              LATCbits.LATC4

// LED Array colour bitmap values
#define LIGHT_RED               0x001F
#define LIGHT_YELLOW            0x7C00
#define LIGHT_GREEN             0x03E0
#define LIGHT_OFF               0x0000
#define LIGHT_CENTERS           0x2044

#define LIGHT_THRESH_OFFSET     2

// Button pins for EEPROM
#define BTN_SET_YELLOW          RB4
#define BTN_SET_RED             RB5

#endif	/* CONSTANTS_H */
