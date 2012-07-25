/**
 * @author Johannes Starosta based on an example by Robert Hartung, hartung@ibr.cs.tu-bs.de
 * 
 */

#include "contiki.h"
#include "net/rime.h"
#include "common.h"

#include "dev/button-sensor.h"
#include <stdio.h>
#include <string.h>
#include "rtimer.h"
#include "clock.h"
#include "timesync.h"

/*---------------------------------------------------------------------------*/

PROCESS(master_time_sync, "master_time_sync");
PROCESS_NAME(common_process);
AUTOSTART_PROCESSES(&master_time_sync,&common_process);

char iAmTheMaster; // defined, initialized and used in getTime.c

/**
 * Called by the system when a packet is received.
 */
static void recv_uc(struct unicast_conn *c, const rimeaddr_t *from)
{
   	struct datagram data_pak;
	readDatagram(c,from,&data_pak);
	
	switch(data_pak.type){
		case 1: // client asks for time
			// data_pak.time_local remains unchanged
			data_pak.time_master=getTimeSystem();
			data_pak.type=2;
			break;

		default:
			return; // Lena: when anything other then 1 is the case, we do not send an answer
		
	}
	// send reply only if the receiver is different from self

	if(!rimeaddr_cmp(from, &rimeaddr_node_addr))
		sendDatagram(c, from, &data_pak);
}

static const struct unicast_callbacks unicast_callbacks = {recv_uc};

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(master_time_sync, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&uc));
  PROCESS_BEGIN();

  iAmTheMaster = 1;

  initNetwork(&unicast_callbacks);

  printf("I am the MASTER, I have the RIME address %x-%x\n", rimeaddr_node_addr.u8[1], rimeaddr_node_addr.u8[0]);

  static struct etimer et;
  while(1){
	// do nothing, except react to incoming messages

	  /*
	  etimer_set(&et, CLOCK_SECOND);
	  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	  bigTime = getTime(&timeIterator); 
	  printf("Lokale Zeit: %lu \n",bigTime);
	  */
	//datapntr[3]='\0';
	/*
	struct datagram data_pak;
	data_pak.time_local=0;
	data_pak.time_master=0;
	data_pak.type=0;
	etimer_set(&et, CLOCK_SECOND);
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	char string[30];
	snprintf(string,30,"%i@%lu@%lu",data_pak.type,time_master,time_local);
	packetbuf_copyfrom(&string,strlen(string)+1); // String + Length to be send
	rimeaddr_t addr;
//	  PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
	addr.u8[0] = 0x22; // Address of receiving Node
	addr.u8[1] = 0x1;
	if(!rimeaddr_cmp(&addr, &rimeaddr_node_addr)){
	//  unicast_send(&uc, &addr);
	 // printf("unicast message sent   %s with length %d \n",  string,strlen(string)+1);//ta.time_local,data.time_master);
	 }*/
	  PROCESS_YIELD(); 
  }
  
  

  PROCESS_END();
}
