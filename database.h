#ifndef DATABASE_H
#define	DATABASE_H

#include <stdbool.h>
#include <stdint.h>

#define DEFAULT_RANGE_POINT_1       50
#define DEFAULT_RANGE_POINT_2       100

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
        uint16_t rangePoint1;
        uint16_t rangePoint2;
    } sdb;
    uint8_t serialised[DATABASE_LENGTH];
} database;

extern database db;

bool db_init(void);
void db_reset(void);
void db_save(void);

#endif	/* DATABASE_H */
