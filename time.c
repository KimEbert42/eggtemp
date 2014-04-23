#include "eggs.h"

volatile unsigned int timerSleep = 0;

void sleep(unsigned int centaseconds)
{
        timerSleep = centaseconds;
#ifndef VLOCLK12Khz
        __bis_SR_register((CPUOFF + SCG0 + GIE)); // LPM1 with interrupts enabled
#else
        __bis_SR_register((CPUOFF + SCG0 + SCG1 + GIE)); // LPM3 with interrupts enabled
#endif
}

void setup_time()
{
        //1 MHz = 1,000,000 cycles per second
        //1,000,000 / 100 = 10,000
#ifndef VLOCLK12Khz
        TACCR0 = 10000;// 100 interrupts per second
        TACTL = TASSEL_2 + MC_1; // Set the timer A to SMCLCK, Continuous
        // Clear the timer and enable timer interrupt
#else
	// Clock is roughly 12 Khz per second
	TACCR0 = 120; // 100 interrupts per second
        TACTL = TASSEL_1 + MC_1; // Set the timer A to ALCK, Continuous
#endif
}

// Timer A0 interrupt service routine
// Occurs every 1/100th of a second
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
	time_event();
        if (timerSleep != 0)
        {
                timerSleep --;
                if (timerSleep == 0)
                {
                        LPM4_EXIT;
                }
        }
}
