#include "contiki.h"
#include "net/rime.h"
#include "timesync.h"
#include <stdio.h>
#include <string.h>
#include "net/rime.h"
#include "dev/leds.h"

#include "common.h"
#include "listen.h"
#include "dev/button-sensor.h"


void initNetwork(struct unicast_callbacks* cb)
{
	unicast_open(&uc, 290, cb);
	masterAddr.u8[0] = MASTER_ADDR_0;
  	masterAddr.u8[1] = MASTER_ADDR_1;
}

/**
 * Sends a datagram. All parametes must point to initalized data structures.
 */
void sendDatagram(struct unicast_conn *c, const rimeaddr_t *to, struct datagram* data_pak)
{
	char string[30];
	snprintf(string,30,"%1i@%10lu@%10lu", data_pak->type, data_pak->time_local, data_pak->time_master);
	packetbuf_copyfrom(&string,sizeof(string)); // String + Length to be send
	unicast_send(c, to);
//	printf("Sent message %s to %x-%x\n", string, to->u8[1], to->u8[0]);
}

/**
 * Decodes a datagram that has been received. All parametes must point to initalized/allocated data structures. Data is read from the input buffer and written into data_pak.
 */
void readDatagram(struct unicast_conn *c, const rimeaddr_t *from, struct datagram* data_pak)
{
  	char *datapntr;
	datapntr = packetbuf_dataptr();
//	printf("Received message %s from %x-%x\n", datapntr, from->u8[1], from->u8[0]);
	
	// convert received string to a datagram struct:
	sscanf(datapntr,"%1i@%10lu@%10lu\0",&data_pak->type,&data_pak->time_local,&data_pak->time_master);
}

void initOutput()
{
	// Prepare registers to output something to the speaker
	// We need to do this twice in a row to disable the external Memory controller. (according to DooMMasteR)
	MCUCR |= 0b10000000;
	MCUCR |= 0b10000000;
	// Set all 4 lines as outputs
	DDRC |= 0b00111100;
	// Set all 4 lines low
	PORTC &= 0b11000011;
}

PROCESS(common_process, "common_process");

PROCESS_THREAD(common_process, ev, data)
{
  PROCESS_BEGIN();


  printf("Common process started. Waiting for time sync to settle.\n");
  initOutput();
  initAdc();
  SENSORS_ACTIVATE(button_sensor);

  static struct etimer et2;
  static long fiveSec;
  static long twoHundredMilli;
  static unsigned long int tc;

  fiveSec = milliToSys(5000);
  twoHundredMilli = milliToSys(200);
  nextBeepTime = 0 ; 

  etimer_set(&et2, milliToTimer(5000));
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et2));
  printf("Sync should be ok now. Starting...\n");

  static struct datagram data_pak;


  while(1){
	tc =  getTimeCorrected();
	//nextBeepTime = (tc / fiveSec + 1) * fiveSec;

	//if(tc < nextBeepTime - twoHundredMilli)	
	//	printf("Still %lu ms to wait, waiting and listening now.\n", sysToMilli((nextBeepTime - twoHundredMilli) - tc));
	while(tc < nextBeepTime - twoHundredMilli)
	{
		int i;
		for(i = 0; i < 1000; i++)
		{
			char erg = listenForBeep();
			if(erg)
			{
				tc =  getTimeCorrected();
				printf("Piep %i in Common registriert, um %lu.\n", (int)erg , tc);
				data_pak.time_master=0L;
				data_pak.type=5;
				data_pak.time_local = tc;
				sendDatagram(&uc, &masterAddr, &data_pak);
			}
		}
		tc =  getTimeCorrected();
		PROCESS_PAUSE();
		if(ev == sensors_event && data == &button_sensor)
			debugPrint();
	}

	//printf("Only %lu ms to wait, busy deaf waiting now.\n", sysToMilli(nextBeepTime - tc));
	while(tc < nextBeepTime)
	{
		tc =  getTimeCorrected();
	}
	
	tc =  getTimeCorrected();
	printf("BEEEEEEP! (I was %lu ms late)\n", sysToMilli(tc - nextBeepTime));
	beepOn(0);
	etimer_set(&et2, milliToTimer(50));
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et2));
	beepOff(0);
  }
  
  PROCESS_END();
}

// Schaltet einen der Pieper an. 0 ist der default-onboard-pieper
void beepOn(char beepPort)
{
	// Das könnte man auch als shift-Operation machen, aber da die Port-Reihenfolge nicht zwingend der Bit-Reihenfolge entspricht, ist das einfacher so. Außerdem gibt es derzeit nur den Port 0.
	switch(beepPort)
	{
		case 0:
			PORTC |= 0b00100000;
			leds_on(LEDS_GREEN);
			break;
	}	
}


// Schaltet einen der Pieper aus. 0 ist der default-onboard-pieper
void beepOff(char beepPort)
{
	// Das könnte man auch als shift-Operation machen, aber da die Port-Reihenfolge nicht zwingend der Bit-Reihenfolge entspricht, ist das einfacher so. Außerdem gibt es derzeit nur den Port 0.
	switch(beepPort)
	{
		case 0:
			PORTC &= 0b11011111;
			leds_off(LEDS_GREEN);
			break;
	}	
}


// Schaltet alle Pieper aus.
void beepAllOff()
{
	PORTC &= 0b11000011;
}


