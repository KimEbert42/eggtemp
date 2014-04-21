#include "eggs.h"

volatile unsigned int timerSleep = 0;

void sleep(unsigned int centaseconds)
{
        timerSleep = centaseconds;
        __bis_SR_register((CPUOFF + GIE)); // LPM0 with interrupts enabled
}

void setup_sleep()
{
        //1 MHz = 1,000,000 cycles per second
        //1,000,000 / 100 = 10,000
        TACCR0 = 10000;// 100 interrupts per second
        TACTL = TASSEL_2 + MC_1; // Set the timer A to SMCLCK, Continuous
        // Clear the timer and enable timer interrupt
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

