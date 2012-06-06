/**
 * @author Johannes Starosta based on an example by Robert Hartung, hartung@ibr.cs.tu-bs.de
 * 
 */

#include "contiki.h"
#include "net/rime.h"

#include "dev/button-sensor.h"
#include <stdio.h>
#include <string.h>
#include "rtimer.h"
#include "clock.h"
#include "timesync.h"

/*---------------------------------------------------------------------------*/

PROCESS(time_unicast_sender, "time Unicast Sender");
AUTOSTART_PROCESSES(&time_unicast_sender);



static void recv_uc(struct unicast_conn *c, const rimeaddr_t *from)
{
  	char *datapntr;
	datapntr = packetbuf_dataptr();
	struct datagram data;
	memcpy(&data,datapntr,sizeof(datapntr));
	//static long bigTime;
	//antwortpaket basteln
	printf("unicast message received from %d.%d: Type %u, local %lu, master %lu \n", from->u8[0], from->u8[1], data.type,data.time_local,data.time_master);
	switch(data.type){
		case 0:
		//	printf("antwort bestätigt");
			break;
		case 1:
	  		bigTime = getTime(&timeIterator); 
  			time_local=bigTime;
 			time_master=bigTime;
			data.time_local=bigTime;
			data.time_master=bigTime;
			data.type=2;
			break;
		case 2:
			//data.type=0;
			//printf("pong antwort vom master. Lokale Zeit: %lu Entfernte Zeit: %lu \n",time_local,data.time_master);
			break;
		//default:
		//	bla;
		//	break
	}
	if(!rimeaddr_cmp(from, &rimeaddr_node_addr))
    	{
		  printf("lokale (master) zeit: %lu entfernte zeit: %lu \n",time_local,data.time_master);
  //  		printf("Answermessage sent\n"); // debug message
	        packetbuf_copyfrom(&data,sizeof(data)); // String + Length to be send
        	unicast_send(c, from);
    	}
}

static const struct unicast_callbacks unicast_callbacks = {recv_uc};
static struct unicast_conn uc;
//		printf("unicast message received from wrong node %d.%d: '%s'\n", from->u8[0], from->u8[1], datapntr);


/*---------------------------------------------------------------------------*/

PROCESS_THREAD(time_unicast_sender, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&uc));
  
  PROCESS_BEGIN();
//  SENSORS_ACTIVATE(button_sensor);//activate button
  unicast_open(&uc, 290, &unicast_callbacks); // channel = 122
  bigTime = 0;
  timeIterator=0;
 // printf("BigTime: %lu \n", bigTime);
  struct datagram data;
  data.end=0;
  data.type=2;
  while(1){
	  bigTime = getTime(&timeIterator); 
	  time_local=bigTime;
	  time_master=bigTime;
	  data.time_local=time_local;
	  data.time_master=time_master;
	  static struct etimer et;
	  rimeaddr_t addr;
	  packetbuf_copyfrom(&data,sizeof(data)); // String + Length to be send
	  // PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
	
	  etimer_set(&et,20* CLOCK_SECOND);
	  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		
	  addr.u8[0] = 0x22; // Address of receiving Node
	  addr.u8[1] = 0x1;
	  if(!rimeaddr_cmp(&addr, &rimeaddr_node_addr)){
		  printf("lokale (master) zeit: %lu entfernte zeit: %lu \n",time_local,data.time_master);
		  //printf("unicast message sent  %u,%lu,%lu \n",  data.type,data.time_local,data.time_master);
		  //printf("Message sent\n"); // debug message
		  //unicast_send(&uc, &addr);
	  }
  }

  PROCESS_END();
}
