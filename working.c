#pragma config FOSC = INTRC_IO  // Oscillator (INTRC with I/O function on OSC2/CLKOUT)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (Disabled)
#pragma config CP = OFF         // Code Protection bit (Code protection off)
#pragma config IOSCFS = 8MHz    // Internal Oscillator Frequency Select (8 MHz INTOSC Speed)
#pragma config CPSW = OFF       // Code Protection bit - Flash Data Memory (Code protection off)
#pragma config BOREN = OFF      //  (BOR Disabled)
#pragma config DRTEN = OFF      //  (DRT Disabled)


#include <xc.h>
#include <htc.h>
#define _XTAL_FREQ 8000000

void main()
{
  TRISC=0X00;
  PORTC=0X00;
  while(1)
  { 
    PORTC=0XFF;
    __delay_ms(1000);
    PORTC=0X00;
    __delay_ms(1000);
  }
}