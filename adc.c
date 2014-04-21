#include "eggs.h"
#include "adc.h"

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

