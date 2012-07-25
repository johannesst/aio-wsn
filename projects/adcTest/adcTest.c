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
  if ((unsigned long)rtime < (unsigned long)tmp){
	(*timeIterator)++;
  }
  tmp=rtime;
  long bigTime = ((unsigned long)rtime) | (((unsigned long)*timeIterator) << 16);
  return bigTime;
}


static unsigned int result;
static unsigned long int timeIterator = 0;

/*! \brief Initializes ADC Peripheral
     Configuration Registers */

void init_adc(void)
{
	DDRB    |= 0b00000001;        			// Set PORTB.0  as output
	PORTB   |= 0b00000001;        			// Initialize PORTB.0 with logic "one"
	ADCSRA   = 0b10000000;        			// Enable ADC with Clock prescaled by some value (0b10 = 2 => 2^2 = factor 4) 37200 or 74400
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


void waitMS(long int ms)
{
	long int cyc = 22L * ms;
	long int i;
	for(i = 0; i < cyc; i++)
	{
		convert();
	}
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
//	printf("##############dzdsfcdcuhlzceglirztzehcuilettrzcu####### adc_test_process talking now ###############\n");
//	printf("##############dzdsfcdcuhlzceglizcuilehutzjjcu####### adc_test_process talking now ###############\n");


	#define NPI_MAX 8
	
	int n, u; // generic loop index
	static int last;
	static int tmp;

	static int ringBuf[64];
	memset(ringBuf, 0, sizeof(int) * 64);
	static int ringIndex = 0;

	static char ringBufDif[64];
	memset(ringBufDif, 0, sizeof(char) * 64);
	static int ringIndexDif = 0;

	static long int backSum = 0; // Summe der letzten 100 samples 
	static long int t = 0;
	static long int bufTime[100];
	static int bufi = 0;
	unsigned long int time, lastZero = 0;
	static long int lastTime;
	static unsigned long int lastTimeBeep = 0;

	#define min 7
	#define max 9

	static char validCount = 0;

	
	static int i = 0;

	// Ständig wiederholen, aber dabei nur alle 1000 Samples unterbrechen lassen
	while(1) 
	{
		++i;

//		time = getTime(&timeIterator);
//		printf("Nothing @ time %u,%u (timeIterator: %u)\n", (unsigned int)(time >> 16), (unsigned int)time, timeIterator);
		for(n = 0; n < 1000; n++)
		{
			t ++;

		
			convert();

			// Möglichst exakt die Zeit, zu der gesamplet wurde
			time = getTime(&timeIterator);
			if(time > lastTimeBeep + 10000L)
			{
				printf("Beep @ time %u,%u\n", (unsigned int)(time >> 16), (unsigned int)time);
				lastTimeBeep = time;
				PORTC |= 0b00100000;
				waitMS(100);
				PORTC &= 0b11011111;
			}
		
			// Laufenden Mittelwert aktualisieren
			backSum -= (long int)ringBuf[ringIndex];
			ringBuf[ringIndex] = result;
			backSum += (long int)result;
			ringIndex = (ringIndex + 1) & 63;

			// Kurve ein-nullen, indem der laufende Mittelwert abgezogen wird (effiziente Teilung durch 64 per Bitshift).
			tmp = result - (backSum >> 6);

			// Gab es eine Nullstelle?
			if((last < 0 && tmp > 0)) // nur noch steigende Flanken || (last > 0 && tmp < 0))
			{
				// Zeit (in samples, nicht timeticks) seit der letzten Nullstelle
				int dif = t - lastZero;

				ringBufDif[ringIndex] = dif;
				ringIndexDif = (ringIndexDif + 1) & 63;

				/*
				bufTime[bufi++] = dif;
				
				if(bufi == 100)
				{	
					printf("\n100 dif times will follow now:\n");
					for(u = 0; u < 99; u++)
						printf("%u\n",(unsigned int)(bufTime[u]));
					printf("\n\n");
					bufi = 0;
				}*/

				// Ist der Zeitabstand zur letzten Nullstelle charakteristisch für unsere Frequenz?
				if(dif >= min && dif <= max)
				{
					// Gültige Halbwellen Hochzählen
					validCount++;

					// Haben wir fünf gültige Halbwellen hintereinander, und haben wir überhaupt noch platz, die zu speichern?
					if(validCount == 5) // && bufi < 512)  
					{
						// ist es schon lange genug her, dass wir so eine Folge hatten?
						if(time > lastTime + 1000)
						{
						// Für die Ausgabe merken							
							// bufTime[bufi++] = time; 

							// Zu Debugzwecken die letzten 64 Samples ausgeben, aber vorher noch 16 Nullen:
							/*for(u = 0; u < 64; u++)							
							{
								printf("0\n");
							}
*/

							leds_on(LEDS_GREEN);
							waitMS(200);
							leds_off(LEDS_GREEN);

							printf("Time since last beep: %i\n",(int)(time - lastTime));
/*
							for(u = 0; u < 64; u++)
							{
								int offs = (backSum >> 6);
								printf("%i, %i\n", (int)ringBuf[(ringIndex + u) & 63] - offs, (int)ringBufDif[(ringIndexDif + u) & 63]);
							}
*/
						}
						lastTime = time;
					}

					// Mehr als 20 halbwellen wollen wir nicht zählen, da es sonst ewig dauert, die wieder ab zu bauen
					if(validCount > 20)
						validCount = 20;
				}
				else
				{
					// ungültige Halbwelle abziehen, dabei nicht unter 0 kommen
					validCount--;
					if(validCount < 0)
						validCount = 0;
					
				}
				// Dieser Nulldurchgang ist ab jetzt der letzte Nulldurchgang
				lastZero = t;
			}

			// Aktuelles Sample als letztes Sample merken
			last = tmp;

			// Haben wir 60 Piep-Beginn-Zeiten gesammelt? Dann jetzt gesammelt ausgeben.


		}		
		PROCESS_PAUSE();
	}

	PROCESS_END();
}
