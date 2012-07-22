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
	ADCSRA   = 0b10000011;        			// Enable ADC with Clock prescaled by some value (0b10 = 2 => 2^2 = factor 4) 37200 or 74400
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
0.0008317398933697197,
-0.0005100205134740197,
-0.0032464298292837978,
-0.007856636732330745,
-0.012707626321608483,
-0.014618010079141791,
-0.010775581557413483,
-0.0008204973810931881,
0.012309600197781309,
0.02347120682912779,
0.02784451098813323,
0.02347120682912779,
0.012309600197781309,
-0.0008204973810931881,
-0.01077558155741349,
-0.014618010079141793,
-0.012707626321608486,
-0.007856636732330747,
-0.0032464298292837995,
-0.0005100205134740197,
0.0008317398933697197
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


#define RESET		0
#define BRIGHT 		1
#define DIM		2
#define UNDERLINE 	3
#define BLINK		4
#define REVERSE		7
#define HIDDEN		8

#define BLACK 		0
#define RED		1
#define GREEN		2
#define YELLOW		3
#define BLUE		4
#define MAGENTA		5
#define CYAN		6
#define	WHITE		7

static int l_attr, l_fg, l_bg;

void textcolor(int attr, int fg, int bg)
{	
	if(attr == l_attr && fg == l_fg && bg == l_bg)
		return;
	
	/* Command is the control command to the terminal */
	printf("%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40);
	l_attr = attr;
	l_fg = fg;
	l_bg = bg;
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
	long int tmp;
	

	PROCESS_BEGIN();

	init_output();
	leds_init();
	init_adc();
	
	// Some garbage code. Whenever there are verification errors, just add or remove one of those lines
	printf("##############dzdsfcdcuhlzceglizcuilecu####### adc_test_trhprocess talking now ###############\n");
	printf("##############dzdsfcdcuhlzceglizcudsfsdfilecu####### adc_testhrt_process talking now ###############\n");
	printf("##############dzdsfcdcuhlzceglizcuilecu####### adc_tthrest_process talking now ###############\n");
	printf("##############dzdsffwefewfcdcuhlzceglizcuilecu####trh### adc_test_process talking now ###############\n");
//	printf("##############dzdsfcdcuhlzcgeeglizcuilecu#######thr adc_test_process talking now ###############\n");
	printf("##############dzdsfcdcuhdsfceggtrlizcuilecu###hthr#### adc_test_process talking now ###############\n");
//	printf("##############dzdsfcdcuhlzceglirztzehcuilettrzcu####### adc_test_process talking now ###############\n");
//	printf("##############dzdsfcdcuhlzceglizcuilehutzjjcu####### adc_test_process talking now ###############\n");

	// Convert the float kernel to integer values for faster computation
	for(i = 0; i < 21; i++)
		kernelInt[i] = (int)(kernelFloat[i] * 100000.0f);
	
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

		nsi = -1;

		int np[500];
		int npi = 0;		

		for(i = 10; i < 502; i++)
		{
			tmp = 0;
			
			//for(n = -10; n <= 10; n++)
			//	tmp += (long int)(buf[i+n]-avg) * kernelInt[n]; 
			//sum += abs(tmp);
			//out[i] = tmp / 30;
//			printf("%i, %i\n", buf[i], out[i]);
			tmp = buf[i] - avg;
			out[i] = tmp;
			if((last < 0 && tmp > 0) || (last > 0 && tmp < 0))
				np[npi++] = i;
			last = tmp;
		}


		// Number of "nulldurchgänge" which had the right distance to the previous one.
		int goodCount = 0;
		int badCount = 0;

		for(i = 6; i < npi; i++)
		{	
			// Gesamtdauer der letzten 3 Perioden. Idealerweise wäre das 75.
			int dif = np[i] - np[i-6];
			if(dif < 70 || dif > 76)
			{
				badCount++;
			//	textcolor(BRIGHT, RED, BLACK);	
			//	printf("%i ", dif);
			}
			else
			{
				goodCount++;
			//	textcolor(RESET, GREEN, BLACK);	
			//	printf("%i ", dif);
			}
		}

		if(goodCount * 2 > badCount)
		{
			leds_on(LEDS_GREEN);
			textcolor(RESET, GREEN, BLACK);	
		}
		else
		{
			leds_off(LEDS_GREEN);
			textcolor(RESET, RED, BLACK);	
		}



		// random output, sometimes
/*
		if(goodCount % 20 == 0)
			for(i = 10; i < 502; i++)
				printf("%i, %i\n", buf[i], out[i]);
*/

		//printf("\nG: %i, B: %i\n", goodCount, badCount);
		printf("\n%i", goodCount);
		textcolor(RESET, WHITE, BLACK);	

/*			{
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
*/
		//}
		// avg = durchschnittliche Amplitude (nach abs)
		avg = (int)(sum/492);
		
		PROCESS_PAUSE();
	}

	PROCESS_END();
}
