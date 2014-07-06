#ifndef __ADC_H
#define __ADC_H

#define LED_0 BIT0 
#define LED_1 BIT6
#define LED_OUT P1OUT
#define LED_DIR P1DIR

#define CAL_ADC_15T30 (*(unsigned int *)(0x10DA+0x08))
#define CAL_ADC_15T85 (*(unsigned int *)(0x10DA+0x0A))
#define CAL_ADC_25T30 (*(unsigned int *)(0x10DA+0x0E))
#define CAL_ADC_25T85 (*(unsigned int *)(0x10DA+0x10))

long chiptemp_read();
int get_temp_f(int oversample_bits);

#endif
