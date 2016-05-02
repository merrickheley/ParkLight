/* 
 * File:   uart.c
 * Author: Merrick
 *
 * Created on May 2, 2016, 1:59 PM
 */

#include "uart.h"

#include "constants.h"

char UART_init(const long int baudrate, bool transmit, bool receive)
{
    unsigned int x;
    x = (_XTAL_FREQ - baudrate*64)/(baudrate*64);   //SPBRG for Low Baud Rate
    if(x>255)                                       //If High Baud Rage Required
    {
        x = (_XTAL_FREQ - baudrate*16)/(baudrate*16); //SPBRG for High Baud Rate
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

void UART_write_text(char *text)
{
    int i;
    for(i=0;text[i]!='\0';i++)
        UART_write(text[i]);
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

void UART_read_text(char *output, unsigned int length)
{
    unsigned int i;
    for(i = 0; i < length; i++)
        output[i] = UART_read();
}