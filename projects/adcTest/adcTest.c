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
static unsigned int compare;

/*! \brief Initializes ADC Peripheral
     Configuration Registers */

void init_adc(void)
{
	DDRB    |= 0b00000001;        			// Set PORTB.0  as output
	PORTB   |= 0b00000001;        			// Initialize PORTB.0 with logic "one"
	ADCSRA   = 0b10000100;        			// Enable ADC with Clock prescaled by 16 ; If Clock speed is 8MHz, then ADC clock = 8MHz/16 = 500kHz
	DIDR0    = 0b00111111;        			// Disable Digital Input on ADC Channel 0 to reduce power consumption
    ADMUX    = 0b11000000;       			// Disable Left-Adjust and select Internal 1.1V reference and ADC Channel 0 as input

	compare  = (unsigned int)465;      		// (465 -> 0.5V Equvlaent Counts for 1.1 V ADC Reference)

}

/*! \brief ADC Conversion Routine
 *  in single ended mode */

void convert(void)
{

	ADCSRA  |= (1<<ADSC);					// Start ADC Conversion

	while((ADCSRA & (1<<ADIF)) != 0x10);	// Wait till conversion is complete

	result   = ADC;                         // Read the ADC Result
	ADCSRA  |= (1 << ADIF);					// Clear ADC Conversion Interrupt Flag

	if(result <= compare)                   // Compare the converted result with 0.5 V
	PORTB   &= 0b11111110;        			// If less Clear PORTB.0
	else
	PORTB   |= 0b00000001;					// If more Set PORTB.0

}

PROCESS(adc_test_process, "adc test process");
AUTOSTART_PROCESSES(&adc_test_process);

static int counter = 0;

PROCESS_THREAD(adc_test_process, ev, data)
{
	PROCESS_BEGIN();
	printf("\n\n##################### adc_test_process talking now ###############\n\n");
	
	leds_init();
	init_adc();

	// Button
	static char down = 0;
	SENSORS_ACTIVATE(button_sensor);

	static char w [] = " _.-:ou#UW&&&§§§§§§§§§";
	int i, max;

	// Main loop
	while(1) {
		max = 0;
		for(i = 0; i < 1000; i++)
		{
			convert();
			//printf("%i,",result);
			if(result > max)
				max = result;
			if(result > 11)
				leds_on(LEDS_GREEN);
			else
				leds_off(LEDS_GREEN);
		}
		printf("Max: %i\tb", max);
		// PROCESS_YIELD();

		/*
		PROCESS_WAIT_EVENT();
		if(ev == sensors_event && data == &button_sensor)
		{

			// Button up or down? Both creates the same event.
			down = !down;
			if(down)
			{
				char strdings [100];
				short i;
				for(i = 0; i < 100; i++)
				{
					convert();
					int o = (result * 200l) / 1024l;
					if(o > 10)
						o = 10;
					//printf("Index %i\n", o);
					strdings[i] = w[o];
				}
				strdings[99] = 0;
				printf("Button pressed, value is %s\n", strdings);	
			}
		}
		else
			printf("Some other event happended.\n");
		*/
		

	}

	PROCESS_END();
}
