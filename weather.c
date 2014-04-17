#include "msp430g2452.h"

#define LED_0 BIT0 
#define LED_1 BIT6
#define LED_OUT P1OUT
#define LED_DIR P1DIR

volatile unsigned int timerSleep = 0;

/*
 * Morse code contains up to 5 dashes or dots per entity
 *
 * 2 ^ 3 = 8 how many bits to use
 * 5 bits = actual dashes or dots (1 or more may acutally be used
 * */

/*
 *  T = 1200 / W
 *
 *  T is in milliseconds
 *
 *  T = 1200 / W / 10 // centa seconds
 *
 */

#define MORSE_DOT_LENGTH 1200 / 5 / 10
#define MORSE_DASH_LENGTH 3 * MORSE_DOT_LENGTH
#define MORSE_LETTER_GAP_LENGTH 5 * MORSE_DOT_LENGTH
#define MORSE_WORD_GAP_LENGTH 7 * MORSE_DOT_LENGTH

#define MORSE_1 0x20
#define MORSE_2 0x40
#define MORSE_3 0x60
#define MORSE_4 0x80
#define MORSE_5 0xA0

#define MORSE_DASH 0x1
#define MORSE_DOT 0x0

const unsigned int morse_alpha[] = {
	MORSE_2 | 0x08, //A
	MORSE_4 | 0x10, //B
	MORSE_4 | 0x14, //C
	MORSE_3 | 0x10, //D
	MORSE_1 | 0x00, //E
	MORSE_4 | 0x04, //F
	MORSE_3 | 0x18, //G
	MORSE_4 | 0x00, //H
	MORSE_2 | 0x00, //I
	MORSE_4 | 0x0E, //J
	MORSE_3 | 0x14, //K
	MORSE_4 | 0x08, //L
	MORSE_2 | 0x18, //M
	MORSE_2 | 0x10, //N
	MORSE_3 | 0x1C, //O
	MORSE_4 | 0x0C, //P
	MORSE_4 | 0x1A, //Q
	MORSE_3 | 0x08, //R
	MORSE_3 | 0x00, //S
	MORSE_1 | 0x10, //T
	MORSE_3 | 0x04, //U
	MORSE_4 | 0x02, //V
	MORSE_3 | 0x0C, //W
	MORSE_4 | 0x12, //X
	MORSE_4 | 0x16, //Y
	MORSE_4 | 0x18  //Z
	};

const unsigned int morse_num[] = {
	MORSE_5 | 0x1F, //0
	MORSE_5 | 0x0F, //1
	MORSE_5 | 0x07, //2
	MORSE_5 | 0x03, //3
	MORSE_5 | 0x01, //4
	MORSE_5 | 0x00, //5
	MORSE_5 | 0x10, //6
	MORSE_5 | 0x18, //7
	MORSE_5 | 0x1C, //8
	MORSE_5 | 0x1E  //9
	};

void sleep(unsigned int centaseconds)
{
	timerSleep = centaseconds;
	__bis_SR_register((CPUOFF + GIE)); // LPM0 with interrupts enabled
}

void morse_write(int value)
{
	int i = 0;
	int max = (value >> 5 & 0x7);
	
	for (i = 0; i < max; i ++)
	{
		LED_OUT = LED_OUT | (LED_0);

		if ( (value >> (4 - i) & 0x1 ) == MORSE_DASH)
		{
			sleep(MORSE_DASH_LENGTH);
		}
		else
		{
			sleep(MORSE_DOT_LENGTH);
		}

		LED_OUT = LED_OUT & (~LED_0);

		sleep(MORSE_DOT_LENGTH);
	}

	sleep(MORSE_LETTER_GAP_LENGTH - MORSE_DOT_LENGTH);
}

void morse_send_string(char* string)
{
	int i = 0;
	while (string[i] != '\0')
	{
		if (string[i] >= 'A' && string[i] <= 'Z')
		{
			morse_write(morse_alpha[string[i] - 'A']);
		} else if (string[i] >= '0' && string[i] <= '9')
		{
			morse_write(morse_num[string[i] - '0']);
		} else if (string[i] == ' ')
		{
			sleep(MORSE_WORD_GAP_LENGTH - MORSE_LETTER_GAP_LENGTH);
		}
		i ++;
	}
}

void chiptemp_setup()
{
    ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON;
    ADC10CTL1 = INCH_10 + ADC10DIV_3;   // Channel 10 = Temp Sensor
}

int chiptemp_read()
{
    unsigned int adc;
 
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
    return (int)(((long)adc * 48676L - 30666688L) >> 16);
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

	//1 MHz = 1,000,000 cycles per second
	//1,000,000 / 100 = 10,000
	TACCR0 = 10000;// 100 interrupts per second
	TACTL = TASSEL_2 + MC_1; // Set the timer A to SMCLCK, Continuous
	// Clear the timer and enable timer interrupt

	__enable_interrupt();

	while (1)
	{
		LED_OUT |= (LED_1);
		itoa(chiptemp_read(), buf);
		LED_OUT &= ~(LED_1);
		morse_send_string("THE TEMP IS \0");
		morse_send_string(buf);
		morse_send_string(" F\0");
		sleep(500);
	}

} 



// Timer A0 interrupt service routine
// Occurs every 1/100th of a second
#pragma vector=TIMERA0_VECTOR
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
