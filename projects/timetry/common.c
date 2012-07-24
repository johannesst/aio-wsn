#include "contiki.h"
#include "net/rime.h"
#include "timesync.h"
#include <stdio.h>
#include <string.h>


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
	int i=sscanf(datapntr,"%1i@%10lu@%10lu\0",&data_pak->type,&data_pak->time_local,&data_pak->time_master);
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

  initOutput();

  static struct etimer et2;
  static long fiveSec;
  fiveSec = milliToSys(5000);
  static long twoHundredMilli;
  twoHundredMilli = milliToSys(200);
  static unsigned long int nextBeepTime;
  unsigned long int tc;

  printf("Common process started. Waiting for time sync to settle.\n");
  etimer_set(&et2, milliToTimer(5000));
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et2));
  printf("Sync should be ok now. Starting...\n");


  while(1){
	planAgain:

	tc =  getTimeCorrected();
	nextBeepTime = (tc / fiveSec + 1) * fiveSec;
	unsigned long int timeToWait = nextBeepTime - tc;

	printf("Planning to beep at %lu ms abs.\n", sysToMilli(nextBeepTime));
	while(timeToWait > twoHundredMilli)
	{
		if(timeToWait > fiveSec * 2)
		{
			printf("Something went wrong with that plan, planning again...");
			goto planAgain;
		}
		unsigned long partTime = timeToWait * 4 / 5;
		printf("Now it is %lu ms abs. Still %lu ms to wait. Going to sleep for %lu ms.\n", sysToMilli(tc), sysToMilli(timeToWait), sysToMilli(partTime));
		etimer_set(&et2, sysToTimer(partTime));
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et2));
		tc =  getTimeCorrected();
		timeToWait = nextBeepTime - tc;
	}

	printf("Now it is %lu ms abs. Only %lu ms to wait, actively waiting now.\n", sysToMilli(tc), sysToMilli(timeToWait));
	while(tc < nextBeepTime)
	{
		tc =  getTimeCorrected();
	}
	
	printf("BEEEEEEP! (I was %lu ms late)\n", sysToMilli(tc - nextBeepTime));
	PORTC |= 0b00100000;
	etimer_set(&et2, milliToTimer(250));
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et2));
	PORTC &= 0b11011111;
  }
  
  PROCESS_END();
}
