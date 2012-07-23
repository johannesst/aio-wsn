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

long getTime(long *timeIterator){
	
  static rtimer_clock_t tmp;

  rtimer_clock_t rtime=RTIMER_NOW();
  if (rtime<tmp){
	timeIterator++;
  }
  long i=(long) timeIterator;
  tmp=rtime;
  long bigTime = ((unsigned long)rtime) + ((unsigned long)i << 16);
  return bigTime;
}


static unsigned int result;
static long timeIterator;

/*! \brief Initializes ADC Peripheral
     Configuration Registers */

void init_adc(void)
{
	DDRB    |= 0b00000001;        			// Set PORTB.0  as output
	PORTB   |= 0b00000001;        			// Initialize PORTB.0 with logic "one"
	ADCSRA   = 0b10000001;        			// Enable ADC with Clock prescaled by some value (0b10 = 2 => 2^2 = factor 4) 37200 or 74400
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

void init_output()
{
	// Prepare registers to output something to the speaker
	// We need to do this twice in a row to disable the external Memory controller. (according to DooMMasteR)
	MCUCR |= 0b10000000;
	MCUCR |= 0b10000000;
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


PROCESS(adc_test_process, "adc test process");
AUTOSTART_PROCESSES(&adc_test_process);

PROCESS_THREAD(adc_test_process, ev, data)
{

	PROCESS_BEGIN();

	init_output();
	leds_init();
	init_adc();
	
	// Some garbage code. Whenever there are verification errors, just add or remove one of those lines
	printf("##############dzdsfcdcuhlzceglizcuilecu####### adc_test_trhprocess talking now ###############\n");
	printf("##############dzdsfcdcuhlzceglizcudsfsdfilecu####### adc_testhrt_process talking now ###############\n");
	printf("##############dzdsfcdcuhlzceglizcuilecu####### adc_tthrest_process talking now ###############\n");
	printf("##############dzdsffwefewfcdcuhlzceglizcuilecu####trh### adc_test_process talking now ###############\n");
	printf("##############dzdsfcdcuhlzcgeeglizcuilecu#######thr adc_test_process talking now ###############\n");
	printf("##############dzdsfcdcuhdsfceggtrlizcuilecu###hthr#### adc_test_process talking now ###############\n");
	printf("##############dzdsfcdcuhlzceglirztzehcuilettrzcu####### adc_test_process talking now ###############\n");
//	printf("##############dzdsfcdcuhlzceglizcuilehutzjjcu####### adc_test_process talking now ###############\n");


	#define NPI_MAX 8
	
	int i, n; // generic loop index
	static int last;
	static int tmp;
	static long int np[NPI_MAX];
	static int npi = 0;	

	static int ringBuf[100];

	memset(ringBuf, 0, sizeof(int) * 100);
	static int ringIndex = 0;

	static long int backSum = 0; // Summe der letzten 100 samples 

	static long int t = 0;

	static int l = 0;

	int u = NPI_MAX - 1;
	static int min;
	min = u * 6.25 * 0.8;
	static int max;
	max = u * 6.25 * 1.2;
	static long int bufTime[512];
	//int bufRaw[512];
	//static char buf[512];
	static int bufi = 0;
	int time, lastFit;
	static long int lastTime;

	min = 6;
	max = 9;

	static char validCount = 0;
	// Main loop
	while(1) 
	{
		l++;
//		printf("l:%i\n", l);
		for(n = 0; n < 1000; n++)
		{
			t ++;
			convert();
			time = (int)getTime(&timeIterator);


			backSum -= (long int)ringBuf[ringIndex];
			ringBuf[ringIndex] = result;
			backSum += (long int)result;
			ringIndex = (ringIndex + 1) & 63;

			tmp = result - (backSum >> 6);
/*
			if(t > 512 && t < 1024)
			{
				buf[t - 512] = tmp;
				bufRaw[t - 512] = result;
			//	bufTime[t - 512] = time;
			}

*/
//			printf("ri: %i\tv+: %i\tv0: %i\tbs:%i\n", ringIndex, result, tmp, (int)backSum);
		

			if((last < 0 && tmp > 0) || (last > 0 && tmp < 0))
			{
				// np[npi] = t;
				// Gesamtdauer der letzten u Perioden. Idealerweise wäre das u*3.
//				int dif = t - np[(npi+7) & 7];
				int dif = t - lastFit;


				/*
buf[bufi++] = dif;
				if(bufi == 512)
				{	
					printf("\n\n");
					for(u = 0; u < 512; u++)
						printf("%i\n",buf[u]);
					printf("\n\n");
					bufi = 0;
				}
*/

//				printf("%i\t", dif);
				//printf("%i, %i, %i\n", time, (int)(t - np[(npi-2+8) & 7]), dif);
//				if(bufi < 512)
//					buf[bufi++] = dif;
				if(dif >= min && dif <= max)
				{
					validCount++;
					if(validCount == 5 && bufi < 512)
					{
						if(time > lastTime + 1000)
							bufTime[bufi++] = time;
						lastTime = time;
					//	printf("Time: %i\n", time);
					}
					if(validCount > 20)
						validCount = 20;
//					textcolor(BRIGHT, GREEN, BLACK);
//					printf("%i\t",dif);
//						leds_on(LEDS_GREEN);
				}
				else
				{
					validCount--;
					if(validCount < 0)
						validCount = 0;
					
//					textcolor(BRIGHT, RED, BLACK);
//					printf("%i\t",dif);
//						leds_off(LEDS_GREEN);
				}
				//npi = (npi + 1) % NPI_MAX;				
				lastFit = t;
			}

			last = tmp;
//		printf("%i\n",bufi);
			if(bufi == 60)
			{	
				printf("\n\n");
				for(u = 0; u < 59; u++)
					printf("%u, %u\n",(unsigned int)(bufTime[u]>>16),(unsigned int)(bufTime[u]));
				printf("\n\n");
				bufi = 0;
			}

		}		
		PROCESS_PAUSE();
	}

	PROCESS_END();
}
