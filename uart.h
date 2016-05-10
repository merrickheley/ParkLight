/* 
 * File:   uart.h
 * Author: Merrick
 *
 * Created on May 2, 2016, 1:59 PM
 */

#ifndef UART_H
#define	UART_H

#include <stdbool.h>

char UART_init(const long int baudrate, bool transmit, bool receive);
void UART_write_text(char *text);
char UART_data_ready();
//void UART_read_text(char *output, unsigned int length);

#endif	/* UART_H */

