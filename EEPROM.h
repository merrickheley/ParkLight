#ifndef EEPROM_H
#define	EEPROM_H

// Library Includes
#include "constants.h"

// C libraries
#include <stdbool.h>
#include <stdint.h>

// PIC Includes
#include <htc.h>

unsigned char eeprom_read_register(unsigned char address);
void eeprom_write_register(unsigned char address, unsigned char data);
    
#endif	/* EEPROM_H */