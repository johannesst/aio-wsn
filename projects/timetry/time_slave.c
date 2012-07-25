/**
 * @author Johannes Starosta based on an example by Robert Hartung, hartung@ibr.cs.tu-bs.de
 * 
 */

#include "contiki.h"
#include "net/rime.h"
#include "common.h"

#include "dev/button-sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rtimer.h"
#include "clock.h"
#include "timesync.h"
#include "common.h"

// All processes share a connection, but have their own datagrams.
static struct datagram data_pak;


PROCESS(slave_time_sync, "slave_time_sync");
PROCESS_NAME(common_process);
AUTOSTART_PROCESSES(&slave_time_sync,&common_process);

long int offset; // defined, initialized and used in getTime.c
long int rate; // defined, initialized and used in getTime.c
unsigned long int lastSysTimeSent; // defined, initialized and used in getTime.c

/**
 * Called by the system when a packet is received.
 */
static void recv_uc(struct unicast_conn *c, const rimeaddr_t *from)
{
  	struct datagram data_pak;
	readDatagram(c,from,&data_pak);
	
	switch(data_pak.type){
		case 1:
//			data_pak.time_local;//me_local;
//			data_pak.time_master;//=time_master;
//			data_pak.type=2;
//			break;
			return;
		case 2:
			{
			unsigned long int now = getTimeSystem();
			unsigned long int pingPongTime = now - data_pak.time_local;
			//printf("Message traveled back and forth in %lu time units. ", pingPongTime);
			unsigned long int masterTimeNow = data_pak.time_master + pingPongTime / 2;
			long int dif = (now - masterTimeNow) - offset;
			offset += dif;
			//printf("Offset adjusted by %li, new offset is %li\n", dif, offset);
			long int timeBetweenMessages = now - lastSysTimeSent;
			rate = dif * 1000L / timeBetweenMessages;
			//printf("Time between Messages: %li. Change rate per time is %i promill.\n", timeBetweenMessages, rate);
			lastSysTimeSent = data_pak.time_local;

			return;
			}
		default:
			return; // Lena: when anything other then 2 is the case, we do not send an answer
	}

	if(!rimeaddr_cmp(from, &rimeaddr_node_addr))
		sendDatagram(c, from, &data_pak);
}

static const struct unicast_callbacks unicast_callbacks = {recv_uc};

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(slave_time_sync, ev, data)
{
	PROCESS_EXITHANDLER(unicast_close(&uc));
	PROCESS_BEGIN();

	initNetwork(&unicast_callbacks);
	beepAllOff();

	// some dummy data, used to change program size in case of verification errors.
	char* dummy = "sdfsdgdfdfffhewkfheskgfhesfhewthherghud";
	printf("Used dummy data: %i\n", strlen(dummy));
	printf("Used dummy data: %i\n\n", strlen(dummy));

	// Declare variables. Everything outside the loop should be static, because everything that is not static will loose its content when we enter the loop.
	static struct etimer et;


	printf("I am a SLAVE, I have the RIME address %x-%x\n", rimeaddr_node_addr.u8[1], rimeaddr_node_addr.u8[0]);

	data_pak.time_master=0L;
	data_pak.type=1;

	while(1){
		data_pak.time_local = getTimeSystem();
		sendDatagram(&uc, &masterAddr, &data_pak);
		etimer_set(&et, CLOCK_SECOND);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	}

	PROCESS_END();
}
