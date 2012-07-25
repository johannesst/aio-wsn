#include "contiki.h"

#include "listen.h"
#include "timesync.h"
#include <avr/io.h>
#include <stdio.h> 
#include <string.h> // for memset

static int last;
static int tmp;
static unsigned int result;

static int ringBuf[64];
static int ringIndex = 0;

static char ringBufDif[64];
static int ringIndexDif = 0;

static long int backSum = 0; // Summe der letzten 100 samples 
static long int t = 0;
static unsigned long int time, lastZero = 0;
static long int lastTime;
static unsigned long int lastTimeBeep = 0;
static char validCount = 0;


void initAdc()
{
	DDRB    |= 0b00000001;        			// Set PORTB.0  as output
	PORTB   |= 0b00000001;        			// Initialize PORTB.0 with logic "one"
	ADCSRA   = 0b10000000;        			// Enable ADC with Clock prescaled by some value (0b10 = 2 => 2^2 = factor 4) 37200 or 74400
	DIDR0    = 0b00111111;        			// Disable Digital Input on ADC Channel 0 to reduce power consumption
	ADMUX    = 0b11000000;       			// Disable Left-Adjust and select Internal 1.1V reference and ADC Channel 0 as input

	memset(ringBufDif, 0, sizeof(char) * 64);
	memset(ringBuf, 0, sizeof(int) * 64);
}


void convert()
{

	ADCSRA  |= (1<<ADSC);					// Start ADC Conversion

	while((ADCSRA & (1<<ADIF)) != 0x10);	// Wait till conversion is complete

	result   = ADC;                         // Read the ADC Result
	ADCSRA  |= (1 << ADIF);					// Clear ADC Conversion Interrupt Flag
}



char listenForBeep()
{
	char ret = 0;
	
	convert();
	t ++;

	// Möglichst exakt die Zeit, zu der gesamplet wurde
	// TODO man könnte ressourcen sparen, indem man hier die unkorrigierte Zeit nehmen würde, und die korrektur nur durchführt, wenn man etwas gefunden hat, dessen absoluter Zeitpunkt relevant ist.
	//time = getTimeSystem();

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

		//printf("%i\t", dif);

		ringBufDif[ringIndex] = dif;
		ringIndexDif = (ringIndexDif + 1) & 63;

		//if(!ringIndexDif)
		//	debugPrint();

		// Ist der Zeitabstand zur letzten Nullstelle charakteristisch für unsere Frequenz?
		if(dif >= min && dif <= max)
		{
			// Gültige Wellen Hochzählen
			validCount++;

			// Haben wir fünf gültige Halbwellen hintereinander, und haben wir überhaupt noch platz, die zu speichern?
			if(validCount == 5) // && bufi < 512)  
			{
				// ist es schon lange genug her, dass wir so eine Folge hatten?
				if(t > lastTime + 5000)
				{
					printf("I found a beeep#########################\n");
					ret = 1;
				}
				lastTime = t;
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

	return ret;
}

void debugPrint()
{
	int u;	
	printf("\n\nDEBUG:\nvalue, timeDif\n");
							for(u = 0; u < 64; u++)
							{
								int offs = (backSum >> 6);
								printf("%i, %i\n", (int)ringBuf[(ringIndex + u) & 63] - offs, (int)ringBufDif[(ringIndexDif + u) & 63]);
							}
	printf("\n(DEBUG Ende)\n\n");
}
