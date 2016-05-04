/* 
 * File:   utils.h
 * Author: Merrick
 *
 * Created on May 2, 2016, 4:32 PM
 */

#ifndef UTILS_H
#define	UTILS_H

#include <stdint.h>

void circular_increment_counter(uint8_t *cnt, uint8_t max);
uint8_t fastMedian5(uint8_t *buf);
uint8_t absdiff(uint8_t a, uint8_t b);

#endif	/* UTILS_H */

