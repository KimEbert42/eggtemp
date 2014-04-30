#include "eggs.h"

volatile unsigned int timerSleep = 0;

void sleep(unsigned int centaseconds)
{
        timerSleep = centaseconds;
#ifndef VLOCLK12Khz
#ifndef VLOCLK32Khz
        __bis_SR_register((CPUOFF + SCG0 + GIE)); // LPM1 with interrupts enabled
#endif
#endif

#ifdef VLOCLK32Khz
        __bis_SR_register((CPUOFF + SCG0 + SCG1 + GIE)); // LPM3 with interrupts enabled
#endif
#ifdef VLOCLK12Khz
        __bis_SR_register((CPUOFF + SCG0 + SCG1 + GIE)); // LPM3 with interrupts enabled
#endif
}

void setup_time()
{
        //1 MHz = 1,000,000 cycles per second
        //1,000,000 / 100 = 10,000
#ifndef VLOCLK12Khz
#ifndef VLOCLK32Khz
        TA0CCR0 = 10000;// 100 interrupts per second
        TA0CTL = TASSEL_2 + MC_1; // Set the timer A to SMCLCK, Continuous
        // Clear the timer and enable timer interrupt
#endif
#endif

#ifdef VLOCLK32Khz
	// Clock is 32768
	TA0CCR0 = 328; // 100 Interrupts per second
	TA0CTL = TASSEL_1 + MC_1;
#endif 

#ifdef VLOCLK12Khz
	// Clock is roughly 12 Khz per second
	TA0CCR0 = 120; // 100 interrupts per second
        TA0CTL = TASSEL_1 + MC_1; // Set the timer A to ALCK, Continuous
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

