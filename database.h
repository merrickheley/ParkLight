#ifndef DATABASE_H
#define	DATABASE_H

#include "EEPROM.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define DEFAULT_RANGE_POINT_1       5
#define DEFAULT_RANGE_POINT_2       20

#define DATABASE_MAX_SIZE   256
#define DATABASE_LENGTH     6
#define DATABASE_SEED       0xED2F

#define DATABASE_MEM_LOC_1  0
#define DATABASE_MEM_LOC_2  (DATABASE_MAX_SIZE/2)

#define DATABASE_CHECKSUM_OFFSET   2

typedef union
{
    struct
    {
        uint16_t checksum;
        uint16_t rangePointRed;
        uint16_t rangePointYellow;
    } sdb;
    uint8_t serialised[DATABASE_LENGTH];
} database;

extern database db;

bool db_init(void);
void db_reset(void);
void db_save(void);
uint16_t chcksum(uint8_t *array, size_t len, uint16_t seed);
bool db_read(uint16_t location);
void db_write(uint16_t location);

#endif	/* DATABASE_H */
