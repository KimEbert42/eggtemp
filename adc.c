#include "eggs.h"
#include "adc.h"

void chiptemp_begin()
{
	ADC10CTL0 = 0;
	ADC10CTL1 = INCH_10 + ADC10DIV_3;   // Channel 10 = Temp Sensor
	ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON;// + ADC SAMPLE AND HOLD 64 x ADC10CLKs + REF V ON + ADC10 ON;
}

void chiptemp_end()
{
	ADC10CTL0 = 0; // Turn off ADC10 to save power
}

long chiptemp_read()
{
	long adc;
 
	chiptemp_begin();

	// ENC = enable conversion, ADC10SC = start conversion
	ADC10CTL0 |= ENC + ADC10SC;
	while (!(ADC10CTL0 & ADC10IFG))
		/* wait until conversion is completed */ ;
 
	adc = (long)ADC10MEM;
 
	// shut off conversion and lower flag to save power.
	// ADC10SC is reset automatically.
	while (ADC10CTL0 & ADC10BUSY)
		/* wait for non-busy per section 22.2.6.6 in User's Guide */ ;

	ADC10CTL0 &= ~ENC;
	ADC10CTL0 &= ~ADC10IFG;
 
	chiptemp_end();

	return adc;
}

int get_temp_f(int oversample_bits)
{
	long i = 0;
	long adc = 0;
	long max = 1L << (oversample_bits * 2);
	long cal_15T30 = ((long)CAL_ADC_15T30) << (oversample_bits);
	long cal_15T85 = ((long)CAL_ADC_15T85) << (oversample_bits);

	for (i = 0; i < max; i++) 
		adc += chiptemp_read();

	adc = adc >> oversample_bits;
	
	// Use calibration values
	// We multiply by 10,000 to get better accuracy out of interger math
	// Finally divide by 100 to get two decimal places
	adc = (
	 ((adc - cal_15T30) * 10000L * (85L-30L))
	/
	 (cal_15T85 - cal_15T30)
	+
	 (30L * 10000L)
	);
	adc = 9L*adc/5L+(32L * 10000L);
	return (int)(adc / 100L);
}
