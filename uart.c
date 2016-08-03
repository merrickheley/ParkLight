/* 
 * File:   uart.c
 * Author: Merrick
 *
 * Created on May 2, 2016, 1:59 PM
 */



#include "uart.h"
#include "constants.h"

#include <xc.h>
#include <htc.h>
#include <pic16f1828.h>

char UART_init(const long int baudrate, const long int clock, bool transmit, bool receive)
{
    unsigned int x;
    x = (clock - baudrate*64)/(baudrate*64);   //SPBRG for Low Baud Rate
    if(x>255)                                       //If High Baud Rage Required
    {
        x = (clock - baudrate*16)/(baudrate*16); //SPBRG for High Baud Rate
        BRGH = 1;                                     //Setting High Baud Rate
    }
    if(x<256)
    {
        SPBRG = x;                                    // Writing SPBRG Register
        SYNC = 0;                                     // Setting Asynchronous Mode, ie UART
        SPEN = 1;                                     // Enables Serial Port    
        if (transmit == true)
            TXEN = 1;                                 // Enables Transmission
        
        if (receive == true)
            CREN = 1;                                 // Enables Continuous Reception
        
        return 1;                                     // Returns 1 to indicate Successful Completion
    }
    return 0;                                       // Returns 0 to indicate UART initialization failed
}

void UART_write(char data)
{
    while(!TRMT);
    TXREG = data;
}

char UART_tx_empty()
{
    return TRMT;
}

void UART_write_text(const char *text)
{
    int i;
    for(i=0;text[i]!='\0';i++) 
    {
        CLRWDT();
        UART_write(text[i]);
    }
}

char UART_data_ready()
{
    return RCIF;
}

char UART_read()
{
    while(!RCIF);
    return RCREG;
}

//void UART_read_text(char *output, unsigned int length)
//{
//    unsigned int i;
//    for(i = 0; i < length; i++)
//        output[i] = UART_read();
//}