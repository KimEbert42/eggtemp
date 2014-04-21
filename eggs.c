#include "eggs.h"

#define LED_0 BIT0 
#define LED_1 BIT6
#define LED_OUT P1OUT
#define LED_DIR P1DIR

char buf[10];

/* strlen: return length of s */
int strlen(char s[])
{
    int i = 0;
    while (s[i] != '\0')
        ++i;
    return i;
}

/* reverse:  reverse string s in place */
void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

/* itoa:  convert n to characters in s */
void itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

void main(void)
{
	WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer

	chiptemp_setup();

	//Set clock to 1Mhz
	BCSCTL1 = CALBC1_1MHZ; // Set range
	DCOCTL = CALDCO_1MHZ;  // Set DCO step and modulation

	LED_DIR |= (LED_0 | LED_1); // Set P1.0 and P1.6
	LED_OUT &= ~(LED_0 | LED_1); // Set the LEDs off

	CCTL0 = CCIE;

	setup_sleep();

	__enable_interrupt();

	LED_OUT |= (LED_1);
	sleep(100);
	LED_OUT &= ~(LED_1);
	while (1)
	{
                int tmp = 0;
		LED_OUT |= (LED_1);
                tmp = chiptemp_read();
		LED_OUT &= ~(LED_1);

		// If temp is above 100.5
                if (tmp <= 10050) // include two decimal places
		{
			itoa(tmp, buf);
		//	morse_send_string("THE TEMP IS \0");
			morse_send_string(buf);
		//	morse_send_string(" F\0");
		}
		sleep(500);
	}

} 

