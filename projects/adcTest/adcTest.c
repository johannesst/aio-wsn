/**
 * \file
 *         Solution to assignent 1 in WSN SS2012
 * \author
 *         Lena Schimmel
 */

#include "contiki.h"

#include <stdio.h> /* For printf() */
#include "dev/leds.h"
#include "dev/button-sensor.h"

#include <avr/io.h>

static unsigned int result;

/*! \brief Initializes ADC Peripheral
     Configuration Registers */

void init_adc(void)
{
	DDRB    |= 0b00000001;        			// Set PORTB.0  as output
	PORTB   |= 0b00000001;        			// Initialize PORTB.0 with logic "one"
	ADCSRA   = 0b10000010;        			// Enable ADC with Clock prescaled by some value (0b10 = 2 => 2^2 = factor 4) 37200 or 74400
	DIDR0    = 0b00111111;        			// Disable Digital Input on ADC Channel 0 to reduce power consumption
    ADMUX    = 0b11000000;       			// Disable Left-Adjust and select Internal 1.1V reference and ADC Channel 0 as input
}

/*! \brief ADC Conversion Routine
 *  in single ended mode */

void convert(void)
{

	ADCSRA  |= (1<<ADSC);					// Start ADC Conversion

	while((ADCSRA & (1<<ADIF)) != 0x10);	// Wait till conversion is complete

	result   = ADC;                         // Read the ADC Result
	ADCSRA  |= (1 << ADIF);					// Clear ADC Conversion Interrupt Flag
}

// Kernel to filter 2k at 74400 sampling rate
float kernelFloat[] =
{
	-0.0009198699708216283,	-0.0012695753139722737,	-0.002017413033257837,	-0.002827476505139998,	-0.0031855130624914504,	-0.0026482164558801757,	-0.0010728928512934721,
	0.0012735642454366897,	0.0037770533905657905,	0.005688756923090621,	0.0064031652675276,	0.005688756923090621,	0.0037770533905658044,	0.0012735642454366897,
	-0.0010728928512934721,	-0.0026482164558801896,	-0.003185513062491447,	-0.00282747650514,	-0.0020174130332578383,	-0.0012695753139722737,	-0.0009198699708216283
};

void init_output()
{
	// Prepare registers to output something to the speaker
	// We need to do this twice in a row to disable the external Memory controller. (according to DooMMasteR)
	MCUCR &= ~0b10000000;
	MCUCR &= ~0b10000000;
	// Set all 4 lines as outputs
	DDRC |= 0b00111100;
	// Set all 4 lines high
	PORTC |= 0b00111100;
}

int kernelInt[21];

PROCESS(adc_test_process, "adc test process");
AUTOSTART_PROCESSES(&adc_test_process);

PROCESS_THREAD(adc_test_process, ev, data)
{
	int i,n; // generic loop indices
	int buf[512]; // buffer for original sound samples
	int out[512]; // buffer for processed sound samples	
	int avg; // variable to hold some average value (is (re-)used in multiple ways)
	long int sum; // variable to hold some sum (is (re-)used in multiple ways)
 	int nsi;
	int cnt;
	int last;
	int tmp;
	

	PROCESS_BEGIN();

	init_output();
	leds_init();
	init_adc();
	
	// Some garbage code. Whenever there are verification errors, just add or remove one of those lines
	printf("##############dzdsfcdcuhlzceglizcuilecu####### adc_test_trhprocess talking now ###############\n");
	printf("##############dzdsfcdcuhlzceglizcudsfsdfilecu####### adc_testhrt_process talking now ###############\n");
	printf("##############dzdsfcdcuhlzceglizcuilecu####### adc_tthrest_process talking now ###############\n");
//	printf("##############dzdsffwefewfcdcuhlzceglizcuilecu####trh### adc_test_process talking now ###############\n");
//	printf("##############dzdsfcdcuhlzcgeeglizcuilecu#######thr adc_test_process talking now ###############\n");
	printf("##############dzdsfcdcuhlzceggtrlizcuilecu###hthr#### adc_test_process talking now ###############\n");
	printf("##############dzdsfcdcuhlzceglirztzehcuilettrzcu####### adc_test_process talking now ###############\n");
	printf("##############dzdsfcdcuhlzceglizcuilehutzjjcu####### adc_test_process talking now ###############\n");

	// Convert the float kernel to integer values for faster computation
	for(i = 0; i < 21; i++)
		kernelInt[i] = (int)(kernelFloat[i] * 10000.0f);
	
	// Main loop
	while(1) 
	{
		sum = 0;		
		for(i = 0; i < 512; i++)
		{
			convert();
			buf[i] = result;
			sum += result;
		}

		// avg = durchschnittlicher Level ( = Gleichspannungsanteil)
		avg = sum / 512;	
		sum = 0;	

		for(i = 10; i < 502; i++)
		{
			for(n = -10; n <= 10; n++)
				out[i] = (buf[i+n]-avg) * kernelInt[n]; 
			sum += abs(out[i]);
		}

		// avg = durchschnittliche Amplitude (nach abs)
		avg = (int)(sum/492);

		nsi = -1;

		// Number of "nulldurchgänge" which had the right distance to the previous one.
		int goodCount = 0;

		for(i = 10; i < 502; i++)
		{
			tmp = out[i];
			if((last < 0 && tmp > 0) || (last > 0 && tmp < 0))
			{	
				if(nsi != -1)
				{
					if((i - nsi) < 15 || (i - nsi) > 21)
					{
						goodCount--;
						if(goodCount < 0)
						{
							leds_off(LEDS_GREEN);
							goodCount = 0;
						}
					}
					else
					{
						goodCount ++;
						if(goodCount > 4)
							leds_on(LEDS_GREEN);
					}
					//printf("%i, %i\n", 3720/(i - nsi)*10, avg);
				}
				nsi = i;
			}

			last = tmp;
		}
		
		
		PROCESS_PAUSE();
	}

	PROCESS_END();
}
