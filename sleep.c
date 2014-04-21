#include "eggs.h"

volatile unsigned int timerSleep = 0;

void sleep(unsigned int centaseconds)
{
#ifndef VLOCLK12Khz
        timerSleep = centaseconds;
        __bis_SR_register((CPUOFF + SCG1 + GIE)); // LPM0 with interrupts enabled
#else
	// Running at 12Khz we can't afford to wake up every centa second, so lower our resolution a little bit.
	centaseconds = centaseconds / 10;
        if (centaseconds == 0)
		centaseconds = 1;
        timerSleep = centaseconds;
        __bis_SR_register((CPUOFF + SCG0 + SCG1 + GIE)); // LPM3 with interrupts enabled
#endif
}

void setup_sleep()
{
        //1 MHz = 1,000,000 cycles per second
        //1,000,000 / 100 = 10,000
#ifndef VLOCLK12Khz
        TACCR0 = 10000;// 100 interrupts per second
        TACTL = TASSEL_2 + MC_1; // Set the timer A to SMCLCK, Continuous
        // Clear the timer and enable timer interrupt
#else
	// Clock is roughly 12 Khz per second
	TACCR0 = 1200; // We want to interrupt about 10 times a second instead of 100 times a second
        TACTL = TASSEL_1 + MC_1; // Set the timer A to ALCK, Continuous
#endif
}

// Timer A0 interrupt service routine
// Occurs every 1/100th of a second
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
        if (timerSleep != 0)
        {
                timerSleep --;
                if (timerSleep == 0)
                {
                        LPM4_EXIT;
                }
        }
}

