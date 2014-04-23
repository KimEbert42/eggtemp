
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

void sleep(unsigned int centaseconds);

void setup_time();
