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

/*---------------------------------------------------------------------------*/

PROCESS(time_unicast_sender, "time Unicast Sender");
AUTOSTART_PROCESSES(&time_unicast_sender);

/*---------------------------------------------------------------------------*/
//static int zaehler=0;
//static int bitzaehler=0;
//rtimer_clock_t offset;
unsigned long time_local;
unsigned long time_master;
struct datagram {
	int type; //switch (type){
		  //case 0 bestätigung
		  //case 1 ping paket vom slave
		  //case 2 pong paket vom master
		  //case 3 sende akkustisches signal 
		  //case 4 empfange akkustisches signal
	unsigned long time_local;
	unsigned long time_master;
	char end;
};


static void recv_uc(struct unicast_conn *c, const rimeaddr_t *from)
{
  	char *datapntr;
	datapntr = packetbuf_dataptr();
	struct datagram blab;
	memcpy(&blab,datapntr,sizeof(datapntr));
	//antwortpaket basteln
	switch(blab.type){
		case 0:
			blab.type=2;
			break;
		case 1:
			blab.time_local=time_local;
			blab.time_master=time_master;
			blab.type=2;
			break;
		case 2:
			blab.type=1;
			printf("pong antwort vom master. Lokale Zeit: %lu Entfernte Zeit: %lu \n",time_local,blab.time_master);
			time_local=blab.time_master;
			time_master=blab.time_master;
			break;
		//default:
		//	bla;
		//	break
	}
	printf("unicast message received from %d.%d: %u,%lu,%lu \n", from->u8[0], from->u8[1], blab.type,blab.time_local,blab.time_master);
	//if(!rimeaddr_cmp(from, &rimeaddr_node_addr))
    	//{
    	//	printf("Answermessage sent\n"); // debug message
        //	unicast_send(c, from);
    	//}
}

//		printf("unicast message received from wrong node %d.%d: '%s'\n", from->u8[0], from->u8[1], datapntr);

static const struct unicast_callbacks unicast_callbacks = {recv_uc};
static struct unicast_conn uc;

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(time_unicast_sender, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&uc));
  
  PROCESS_BEGIN();
//  SENSORS_ACTIVATE(button_sensor);//activate button
  unicast_open(&uc, 290, &unicast_callbacks); // channel = 122
  static rtimer_clock_t tmp;
  static long bigTime = 0;
  static long bla_time=0;
 // printf("BigTime: %lu \n", bigTime);
  struct datagram blab;
  blab.end=0;
  blab.type=2;
  while(1)
  {

  blab.type=2;
  rtimer_clock_t rtime=RTIMER_NOW();
  if (rtime<tmp){
	bla_time++;
  }
  tmp=rtime;
  bigTime = ((unsigned long)rtime) + 	((unsigned long)bla_time << 16);
  time_local=bigTime;
  time_master=bigTime;
  static struct etimer et;
  rimeaddr_t addr;
  packetbuf_copyfrom(&blab,sizeof(blab)); // String + Length to be send
  // PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);

  etimer_set(&et, CLOCK_SECOND);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	
  addr.u8[0] = 0x22; // Address of receiving Node
  addr.u8[1] = 0x1;
  if(!rimeaddr_cmp(&addr, &rimeaddr_node_addr))
  {
	  printf("unicast message sent  %u,%lu,%lu \n",  blab.type,blab.time_local,blab.time_master);
	  //printf("Message sent\n"); // debug message
	  unicast_send(&uc, &addr);
  }
  }

  PROCESS_END();
}
