#include "eggs.h"
#include "adc.h"

void chiptemp_begin()
{
	ADC10CTL0 = 0;
	ADC10CTL1 = INCH_10 + ADC10DIV_3;   // Channel 10 = Temp Sensor
	ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON;// + REF2_5V;
}

void chiptemp_end()
{
	ADC10CTL0 = 0; // Turn off ADC10 to save power
}

int chiptemp_read()
{
	long adc;
 
	chiptemp_begin();

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
 
	chiptemp_end();

	// Use calibration values
	// We multiply by 10,000 to get better accuracy out of interger math
	// Finally divide by 100 to get two decimal places
	adc = (
	 ((adc - (long)CAL_ADC_15T30) * (long)10000 * (long)(85-30))
	/
	 ((long)(CAL_ADC_15T85 - CAL_ADC_15T30))
	+
	 ((long)300000)
	);
	adc = (long)9*adc/(long)5+(long)320000;
	return (int)(adc / 100);
}

