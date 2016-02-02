#ifndef EEPROM_H
#define	EEPROM_H

    // Explicit function declarations
    unsigned char readFromEEPROM(unsigned char address);
    void writeToEEPROM(unsigned char address, unsigned char data);

#endif	/* EEPROM_H */