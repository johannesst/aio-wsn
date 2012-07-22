/**
 * @author Johannes Starosta based on an example by Robert Hartung, hartung@ibr.cs.tu-bs.de
 * 
 */

#include "contiki.h"
#include "net/rime.h"

#include "dev/button-sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rtimer.h"
#include "clock.h"
#include "timesync.h"

/*---------------------------------------------------------------------------*/

PROCESS(time_unicast_sender, "time Unicast Sender");
AUTOSTART_PROCESSES(&time_unicast_sender);


 // static long bigTime = 99999999;

static void recv_uc(struct unicast_conn *c, const rimeaddr_t *from)
{
  	char *datapntr;
	datapntr = packetbuf_dataptr();
	//struct datagram data;
	char *end;
	 unsigned long  data=strtoul(datapntr,&end,10);
	printf("datapntr %s data %u end %s\n",datapntr,data,end);

//	memcpy(&data,datapntr,sizeof(datapntr));

	//antwortpaket basteln
	/*
	switch(data.type){
		case 0:
			break;
		case 1:
			data.time_local=time_local;
			data.time_master=time_master;
			data.type=2;
			break;
		case 2:
			data.type=0;
			printf("pong antwort vom master. Lokale Zeit: %lu Entfernte Zeit: %lu \n",time_local,data.time_master);
			getTime
			time_local=data.time_master;
			time_master=data.time_master;
			break;
		//default:
		//	bla;
		//	break
	}*/
	//printf("unicast message received from %d.%d: %u,%lu,%lu \n", from->u8[0], from->u8[1], data.type,data.time_local,data.time_master);
	//printf("unicast message received from %d.%d: %lu \n", from->u8[0], from->u8[1], data);
	printf("Lokale Zeit alt: %lu \n ",bigTime);
	bigTime=data;
	printf("Lokale Zeit neu: %lu \n ",bigTime);;
	/*if(!rimeaddr_cmp(from, &rimeaddr_node_addr))
    	{
    		printf("Answermessage sent\n"); // debug message
        	unicast_send(&c, from);
    	}*/
}

//		printf("unicast message received from wrong node %d.%d: '%s'\n", from->u8[0], from->u8[1], datapntr);

static const struct unicast_callbacks unicast_callbacks = {recv_uc};
static struct unicast_conn uc;

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(time_unicast_sender, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&uc));
  
  PROCESS_BEGIN();
  SENSORS_ACTIVATE(button_sensor);//activate button
  unicast_open(&uc, 290, &unicast_callbacks); // channel = 122
  static long timeIterator=0;
  static rtimer_clock_t tmp;
 // printf("BigTime: %lu \n", bigTime);
  int i = 0; 
  while(1){
	  PROCESS_YIELD();
	  

	  //getTime(&bigTime,timeIterator,time_local,time_master); 
//	  bigTime = getTime(&timeIterator); 
//	  printf("bigTime %lu \n",bigTime);
	  //setDataTime(&data,&bigTime);
/*
	  static struct etimer et;
	  rimeaddr_t addr;
	  packetbuf_copyfrom(&data,sizeof(data)); // String + Length to be send
//	  PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
	
	  etimer_set(&et, CLOCK_SECOND);
//	  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		
	  addr.u8[0] = 0x28; // Address of receiving Node
	  addr.u8[1] = 0x1;
//	  if(!rimeaddr_cmp(&addr, &rimeaddr_node_addr)){
		  //printf("unicast message sent  %u,%lu,%lu \n",  data.type,data.time_local,data.time_master);
		  //printf("Message sent\n"); // debug message
		  //unicast_send(&uc, &addr);
	//  }
	//  */
	if (i<3){
		i++;
	}else if(i>=3)
		break;
  }

  PROCESS_END();
}
