/*
 * File:   EEPROM.c
 * Author: Jonathan Holland
 *
 * Created on 27 January 2016, 9:41 PM
 */

#include "EEPROM.h"


// Library Includes
#include "constants.h"

// C libraries
#include <stdbool.h>
#include <stdint.h>

// PIC Includes
#include <htc.h>

/*
 * readFromEEPROM
 * 
 * Read data from the given address and return the data
 * 
 * Input: 
 *      unsigned char address
 *      - The location in memory to read from
 * 
 * Output:  
 *      unsigned char EEDATA
 *      - The data stored at the given memory address
 */
unsigned char readFromEEPROM(unsigned char address)
{
  EEADR = address; //Address to be read
  EECON1bits.EEPGD =  0;//Selecting EEPROM Data Memory
  EECON1bits.RD = 1; // Initialise read cycle
  return EEDATA; //Returning data
}

/*
 * writeToEEPROM
 * 
 * Write byte data to the specified address
 * 
 * Input: 
 *      unsigned char address
 *      - The memory location to write the data to
 *      unsigned char data
 *      - The data to write to the memory address
 * 
 * Output:  
 *      void
 * 
 */
void writeToEEPROM(unsigned char address, unsigned char data)
{
  unsigned char INTCON_SAVE; // To save INTCON register value
  EEADR = address; // Address to write
  EEDATA = data; // Data to write
  EECON1bits.EEPGD = 0; // Selecting EEPROM Data Memory
  EECON1bits.WREN = 1; // Enable writing of EEPROM
  INTCON_SAVE=INTCON; // Backup register
  INTCON=0; // Disable interrupt
  EECON2=0x55; // Required sequence for write to internal EEPROM
  EECON2=0xAA; // Required sequence for write to internal EEPROM
  EECON1bits.WR = 1; // Initialise write cycle
  INTCON = INTCON_SAVE;// Enables Interrupt
  EECON1bits.WREN = 0; // To disable write
  while(PIR2bits.EEIF == 0)// Check write finish
  {
    ((void)0); //do nothing
  }
  PIR2bits.EEIF = 0; //Clearing EEIF bit
}
