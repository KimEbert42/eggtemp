#include "eggs.h"

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

#define LED_0 BIT0 
#define LED_1 BIT6
#define LED_OUT P1OUT
#define LED_DIR P1DIR

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

const unsigned char morse_alpha[] = {
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

const unsigned char morse_num[]  = {
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


void morse_write(char value) 
{
	int i = 0;
	int max = (value >> 5 & 0x7);
	
	for (i = 0; i < max; i ++)
	{
#ifndef BACKWARDLIGHT
		LED_OUT = LED_OUT | (LED_0);
#else
		LED_OUT = LED_OUT & (~LED_0);
#endif

		if ( (value >> (4 - i) & 0x1 ) == MORSE_DASH)
		{
			sleep(MORSE_DASH_LENGTH);
		}
		else
		{
			sleep(MORSE_DOT_LENGTH);
		}

#ifndef BACKWARDLIGHT
		LED_OUT = LED_OUT & (~LED_0);
#else
		LED_OUT = LED_OUT | (LED_0);
#endif

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
		i++;
	}
}

