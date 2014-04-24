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
		n = -n;		  /* make n positive */
	i = 0;
	do {	   /* generate digits in reverse order */
		s[i++] = n % 10 + '0';   /* get next digit */
	} while ((n /= 10) > 0);	 /* delete it */
	if (sign < 0)
		s[i++] = '-';
	s[i] = '\0';
	reverse(s);
}

volatile unsigned int events; // 16 events
volatile unsigned long event_counter;

#define EVENT_WATCHDOG 0x0001
#define EVENT_CHECKTEMP 0x0002
#define EVENT_RECORDTEMP 0x0004
#define EVENT_BLINKLED 0x0008

void inline time_event()
{
	if ((event_counter % 1000L) == 0) // Every 10 seconds // for testing 1 second
	{
		events |= EVENT_CHECKTEMP;

		if ((event_counter % 6000L) == 0) // Every 60 seconds
		{
			events |= EVENT_BLINKLED;
		}
		if ((event_counter % 90000L) == 0) // Every 15 minutes (15 * 60 * 100 = 900 * 100 = 90,000
		{
			events |= EVENT_RECORDTEMP + EVENT_WATCHDOG;
			event_counter = 0;// Reset Event counter
		}
	}
	event_counter ++;
}

/**************************
 *
 * Memory for history is stored at 0xe000 + 4096 bytes
 * Each block is divided into 512 byte segments
 * 0xe000 - 0xe1ff
 * 0xe200 - 0xe3ff
 * 0xe400 - 0xe5ff
 * 0xe600 - 0xe7ff
 * ...
 *
 * Temp will be store as an int with a value less then 0xffff giving us
 * 1024 data points, or about 10 days of data points. (Data point every 15 minutes)
 *
 * Max ints is 2048
 * Int cannot == 0xffff, so we make it 0xfffe
 * Ints per segment = 256
 * */
#define INFOE ((int *)(0xe000))
#define INTS_PER_SEGMENT 256
#define INTS_TOTAL 2048

volatile int next_memory;

void save_temp(int temp)
{
	int after_next_memory = 0;
	if (temp == 0xffff)
	{
		temp = 0xfffe;
	}
	
	flash_write_int(INFOE + next_memory, temp);

	next_memory ++;

	if (next_memory >= INTS_TOTAL)
		next_memory = 0;

	after_next_memory = next_memory + 1;
	if (after_next_memory >= INTS_TOTAL)
		after_next_memory = 0;

	if (*(INFOE + after_next_memory) != 0xffff)
	{
		flash_erase(INFOE + after_next_memory);
	}
}

void main(void)
{
	WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer

	// Setup the Clocks
	//Set clock to 1Mhz
	BCSCTL1 = CALBC1_1MHZ; // Set range
	DCOCTL = CALDCO_1MHZ;  // Set DCO step and modulation
#ifdef VLOCLK12Khz
	//Set clock to Internal Very Low Power Low Frequency Oscillator ~12Khz
	BCSCTL3 |= LFXT1S_2;
#endif

	// Setup the LEDS
	LED_DIR |= (LED_0 | LED_1); // Set P1.0 and P1.6
	LED_OUT &= ~(LED_0 | LED_1); // Set the LEDs off

	CCTL0 = CCIE;

	// Setup Events
	
	events = 0;
	event_counter = 0;

	// Setup memory
	{
		int i;

		for (i = 0; i < INTS_TOTAL; i ++)
		{
			if (*(INFOE + i) == 0xffff)
				break;
		}
		if (i > INTS_TOTAL)
		{
			i = 0;
			flash_erase(INFOE);
		}
		next_memory = i;
	}

	// Setup Timer

	setup_time();

	// Setup the Watchdog

	__enable_interrupt();

	while (1) // Event loop
	{
		if (events != 0)
		{
			if ((events & EVENT_WATCHDOG) != 0)
			{
				// Watchdog event always goes first.
				events &= ~EVENT_WATCHDOG;
			}
			if ((events & EVENT_RECORDTEMP) != 0)
			{
				int tmp = 0;
				tmp = get_temp_f(3);

				save_temp(tmp);

				events &= ~EVENT_RECORDTEMP;
			}
			if ((events & EVENT_BLINKLED) != 0)
			{
				LED_OUT |= (LED_1);
				sleep(5);
				LED_OUT &= ~(LED_1);
				events &= ~EVENT_BLINKLED;
			}
			if ((events & EVENT_CHECKTEMP) != 0)
			{
				int tmp = 0;
				tmp = get_temp_f(3);

				// If temp is above 100.5
				if (tmp >= 10050) // include two decimal places
				{
					itoa(tmp, buf);
				//	morse_send_string("THE TEMP IS \0");
					morse_send_string(buf);
				//	morse_send_string(" F\0");
				}
				events &= ~EVENT_CHECKTEMP;
			}

		}
		if (events == 0)
		{
			sleep(100); // Sleep 1 second
		}
	}

} 

