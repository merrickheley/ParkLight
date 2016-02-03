/*
 * File:   database.c
 * Author: Merrick
 *
 * Created on 27 January 2016, 8:00 PM
 */

#include "database.h"

#define CALC_CHECKSUM(tdb) chcksum(tdb.serialised + DATABASE_CHECKSUM_OFFSET, \
    DATABASE_LENGTH - DATABASE_CHECKSUM_OFFSET, DATABASE_SEED)

database db;

/*
 * checksum
 * 
 * Calculate a CRC checksum, where modulo is UINT16_MAX
 * 
 * Input:
 *      array   Array of data to build checksum
 *      len     Length of the array
 *      seed    Seed to use. See https://users.ece.cmu.edu/~koopman/crc/
 * 
 * Output:
 *      Checksum
 */
uint16_t chcksum(uint8_t *array, size_t len, uint16_t seed)
{
    uint_fast16_t i;
    uint16_t chk;
    
    chk = (uint16_t) seed;
    for (i = 0; i < len; i++)
        chk += *array++;
    
    return chk;
}

/*
 * db_write
 * 
 * Write the database to the location given. Only write if the data read differs
 * from the the data to be written.
 */
void db_write(uint16_t location)
{
    uint_fast16_t i;
    
    //TODO: Do we need to bump the watchdog while we're in here?
    for (i = 0; i < DATABASE_LENGTH; i++)
        if (eeprom_read_register((uint16_t) (location + i)) != db.serialised[i])
            eeprom_write_register((uint16_t) (location + i), db.serialised[i]);
}

/*
 * db_read
 * 
 * Read a database from the location into the structure.
 * 
 * Output:
 *      Boolean, true if database checksum matches calculated checksum.
 */
bool db_read(uint16_t location)
{
    uint_fast16_t i;
    
    //TODO: Do we need to bump the watchdog while we're in here?
    for (i = 0; i < DATABASE_LENGTH; i++)
        db.serialised[i] = eeprom_read_register((uint16_t) (location + i));
    
    return db.sdb.checksum == CALC_CHECKSUM(db);        
}

/*
 * db_save
 * 
 * Update the checksum and save the database to both locations.
 */
void db_save(void) {
    db.sdb.checksum = CALC_CHECKSUM(db);
    
    db_write(DATABASE_MEM_LOC_1);
    db_write(DATABASE_MEM_LOC_2);
}

/*
 * db_reset
 * 
 * Reset the database with defaults.
 */
void db_reset(void) {
    db.sdb.rangePoint1 = DEFAULT_RANGE_POINT_1;
    db.sdb.rangePoint2 = DEFAULT_RANGE_POINT_2;
    
    db_save();
}

/*
 * Read from one memory location. If it is valid, write to the other.
 * If both fail, reset the database and return false so the application can 
 * handle it.
 */
bool db_init(void) {
    if (db_read(DATABASE_MEM_LOC_1) == true)
        db_write(DATABASE_MEM_LOC_2);
    else if (db_read(DATABASE_MEM_LOC_2) == true)
        db_write(DATABASE_MEM_LOC_1);
    else
    {
        db_reset();
        return false;
    }
    
    return true;
}
