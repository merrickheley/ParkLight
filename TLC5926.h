#ifndef TLC5926_H
#define	TLC5926_H

#include <stdint.h>

#define TLC5926_US_DELAY   10

void TLC5926_init(void);
void TLC5926_SetLights(uint16_t bitmap);

#endif	/* TLC5926_H */
