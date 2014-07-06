#include "eggs.h"

void inline flash_setup()
{
	__disable_interrupt(); // Please don't interrupt me
	FCTL2 = FWKEY + FSSEL1 + FN4; // MCLK at correct speed for 8MHz clock
	FCTL3 = FWKEY; // CLEAR LOCK
}

void inline flash_teardown()
{
	FCTL1 = FWKEY; // Clear Write or Read
	FCTL3 = FWKEY + LOCK; 
	__enable_interrupt(); // Please interrupt
}

void flash_write_int(int *place, int data)
{
	flash_setup();
	FCTL1 = FWKEY + WRT; // Enable Write
	*place = data;
	flash_teardown();	
}

void flash_write_byte(char *place, char data)
{
	flash_setup();
	FCTL1 = FWKEY + WRT; // Enable Write
	*place = data;
	flash_teardown();	
}

void flash_erase(int *segment)
{
	flash_setup();
	FCTL1 = FWKEY + ERASE; // Enable Erase
	*segment = 0; // Dummy write
	flash_teardown();	
}


