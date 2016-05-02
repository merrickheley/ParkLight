/* 
 * File:   utils.c
 * Author: Merrick
 *
 * Created on May 2, 2016, 4:32 PM
 */

#include "utils.h"

#include <string.h>

void circular_increment_counter(uint8_t *cnt, uint8_t max)
{   
    (*cnt)++;
    if (*cnt == max)
        *cnt = 0;
}

void swap(uint8_t *a, uint8_t *b)
{
	uint8_t tmp = *b;
	*b = *a;
	*a = tmp;
}

uint8_t fastMedian5(uint8_t *buf)
{
    uint8_t arr[5] = {0};
    
    // Put the numbers in an array
    memcpy(arr, buf, (size_t) 5);
    
    // Ensure a[0] < a[1], a[0] < a[3], a[3] < a[4]
    if (arr[0] > arr[1]) swap(&arr[0], &arr[1]);
    if (arr[3] > arr[4]) swap(&arr[3], &arr[4]);
    if (arr[0] > arr[3]) {
    	swap(&arr[0], &arr[3]);
    	swap(&arr[1], &arr[4]);
    }
    
    if (arr[2] > arr[1]) {
    	if (arr[1] < arr[3]) {
    		if (arr[2] < arr[3])
                return arr[2];
            else
                return arr[3];
        } else {
            if (arr[1] < arr[4])
                return arr[1];
            else
                return arr[4];
        }
    } else {
    	if (arr[2] > arr[3]) {
    		if (arr[2] < arr[4])
                return arr[2];
            else 
                return arr[4];
        } else {
            if (arr[1] < arr[3])
                return arr[1];
            else 
                return arr[3];
        }
    }
}