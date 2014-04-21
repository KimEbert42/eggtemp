#include "eggs.h"

#define LED_0 BIT0 
#define LED_1 BIT6
#define LED_OUT P1OUT
#define LED_DIR P1DIR

#define CAL_ADC_15T30 (*(unsigned int *)(0x10DA+0x08))
#define CAL_ADC_15T85 (*(unsigned int *)(0x10DA+0x0A))
#define CAL_ADC_25T30 (*(unsigned int *)(0x10DA+0x0E))
#define CAL_ADC_25T85 (*(unsigned int *)(0x10DA+0x10))

void chiptemp_setup()
{
    ADC10CTL0 = 0;
    ADC10CTL1 = INCH_10 + ADC10DIV_3;   // Channel 10 = Temp Sensor
    ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON;// + REF2_5V;
}

int chiptemp_read()
{
    long adc;
 
    // ENC = enable conversion, ADC10SC = start conversion
    ADC10CTL0 |= ENC + ADC10SC;
    while (!(ADC10CTL0 & ADC10IFG))
        /* wait until conversion is completed */ ;
 
    adc = ADC10MEM;
 
    // shut off conversion and lower flag to save power.
    // ADC10SC is reset automatically.
    while (ADC10CTL0 & ADC10BUSY)
        /* wait for non-busy per section 22.2.6.6 in User's Guide */ ;

    ADC10CTL0 &= ~ENC;
    ADC10CTL0 &= ~ADC10IFG;
 
    // VTEMP=0.00355(TEMPC)+0.986
    // VTEMP / 0.00355 - 0.986 / 0.00355 = TEMPC
    // C = TEMPC / 0.00355 - 277.746478873
    // A = 1023 * V / Vref
    // V = A * Vref / 1023
    // V = A * 1.5 / 1023
    // C = A * 1.5/ 1024 / 0.00355 - 277.746478873
    // C = A * 0.412632042254 - 277.746478873
    // C = (A * 27042 - 18,202,393) / 65536
    // C = A * 0.412632042254 - 277.746478873
    // F = (A * 0.412632042254  * 1.8 - 277.746478873 * 1.8) + 32
    // F = (A * 0.742737676057 - 499.943661971 + 32)
    // F = (A + 48,676 - 30,666,687.8873) / 65535
    // return degrees F
    //
    // Use calibration values
    adc = (
	 ((adc - (long)CAL_ADC_15T30) * (long)100 * (long)(85-30))
	/
	 ((long)(CAL_ADC_15T85 - CAL_ADC_15T30))
	+
	 ((long)3000)
	);
    adc = (long)9*adc/(long)5+(long)3200;
    return (int)(adc);
    // return (int)(((long)adc * 48676L - 30666688L) >> 16);
}

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
                if (tmp >= 10050) // include two decimal places
		{
			itoa(tmp, buf);
		//	morse_send_string("THE TEMP IS \0");
			morse_send_string(buf);
		//	morse_send_string(" F\0");
		}
		sleep(500);
	}

} 

