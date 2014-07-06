#include "onewire.h"

/*

The MIT License

Copyright (c) 2014 Kim Ebert (kim@kd7ike.info)
Copyright (c) 2010-2012 David Siroky (siroky@dasir.cz)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

typedef struct {
	volatile uint8_t *port_out;
	const volatile uint8_t *port_in;
	volatile uint8_t *port_ren;
	volatile uint8_t *port_dir;
	int pin;
} onewire_t;

inline void onewire_line_low(onewire_t *ow)
{
	*(ow->port_dir) |= ow->pin;
	*(ow->port_out) &= ~ow->pin;
	*(ow->port_ren) &= ~ow->pin;
}

inline void onewire_line_high(onewire_t *ow)
{
	*(ow->port_dir) |= ow->pin;
	*(ow->port_out) |= ow->pin;
	*(ow->port_ren) &= ~ow->pin;
}

inline void onewire_line_release(onewire_t *ow)
{
	*(ow->port_dir) &= ~ow->pin; // input
	*(ow->port_ren) |= ow->pin;
	*(ow->port_out) |= ow->pin; // internal resistor pullup
}

/// @return: 0 if ok
int onewire_reset(onewire_t *ow)
{
	onewire_line_low(ow);
	sleepMicro(550); // 480us minimum
	onewire_line_release(ow);
	sleepMicro(70); // slave waits 15-60us
	if (*(ow->port_in) & ow->pin) return 1; // line should be pulled down by slave
	sleepMicro(300); // slave TX presence pulse 60-240us
	if (!(*(ow->port_in) & ow->pin)) return 2; // line should be "released" by slave
	return 0;
}

void onewire_write_bit(onewire_t *ow, int bit)
{
	sleepMicro(2); // recovery, min 1us
	onewire_line_low(ow);
	if (bit)
		sleepMicro(5); // max 15us
	else
		sleepMicro(55); // min 60us
	onewire_line_release(ow);
	// rest of the write slot
	if (bit)
		sleepMicro(55);
	else
		sleepMicro(5);
}

int onewire_read_bit(onewire_t *ow)
{
	int bit;
	sleepMicro(1); // recovery, min 1us
	onewire_line_low(ow);
	sleepMicro(5); // hold min 1us
	onewire_line_release(ow);
	sleepMicro(10); // 15us window
	bit = *(ow->port_in) & ow->pin;
	sleepMicro(50); // rest of the read slot
	return bit;
}

void onewire_write_byte(onewire_t *ow, uint8_t byte)
{
	int i;
	for(i = 0; i < 8; i++)
	{
		onewire_write_bit(ow, byte & 1);
		byte >>= 1;
	}
}

uint8_t onewire_read_byte(onewire_t *ow)
{
	int i;
	uint8_t byte = 0;
	for(i = 0; i < 8; i++)
	{
		byte >>= 1;
		if (onewire_read_bit(ow)) byte |= 0x80;
	}
	return byte;
}

int get_ext_temp_f(int oversample_bits)
{
	int result = 0;
	uint8_t one = 0;
	uint8_t two = 0;

	onewire_t ow;

	ow.port_out = &P1OUT;
	ow.port_in = &P1IN;
	ow.port_ren = &P1REN;
	ow.port_dir = &P1DIR;
	ow.pin = BIT7;

	onewire_line_high(&ow);
	sleepMicro(100);

	if (onewire_reset(&ow) != 0)
	{
		return 0;
	}

	onewire_write_byte(&ow, 0xcc);
	onewire_write_byte(&ow, 0x4e);
	onewire_write_byte(&ow, 0x00);
	onewire_write_byte(&ow, 0x00);
	onewire_write_byte(&ow, 0xff); // 12 resolution bits

	if (onewire_reset(&ow) != 0)
	{
		onewire_line_low(&ow);// Lets not waste power now
		return 0;
	}

	onewire_write_byte(&ow, 0xcc);
	onewire_write_byte(&ow, 0x44);

	onewire_line_high(&ow);

	sleep(80); // 750ms held high for parasitic power max conversion time

	if (onewire_reset(&ow) != 0)
	{
		onewire_line_low(&ow);// Lets not waste power now
		return 0;
	}

	onewire_write_byte(&ow, 0xcc);
	onewire_write_byte(&ow, 0xbe);

	one = onewire_read_byte(&ow);
	two = onewire_read_byte(&ow);

	if (onewire_reset(&ow) != 0)
	{
		onewire_line_low(&ow);// Lets not waste power now
		return 0;
	}
	
	onewire_line_low(&ow);// Lets not waste power now

	result = ( ((long)two) << 8) | one;

	return result;
}

