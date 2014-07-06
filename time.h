
/*
 * Time makes use of the A Timer, using SMCLK or ACLK.
 *
 * You need to define time_event() which gets called every 1/100 of a second.
 *
 * This allows you to create events that happen periodically.
 *
 * void inline time_event()
 *
 * */

#ifndef __SLEEP_H
#define __SLEEP_H

#define CYCLES_PER_MS (8) // Depends on CPU speed - 8Mhz
#define sleepMicro(x) __delay_cycles((x * CYCLES_PER_MS))
void sleep(unsigned int centaseconds);

void setup_time();


#endif
