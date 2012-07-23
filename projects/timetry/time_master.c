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
	struct datagram data_pak;
	printf("empfangener string: %s   mit länge %i \n",datapntr,strlen(datapntr));
	
	//char* token;
	//token=strtok(datapntr,"@");
	//data_pak.type=atoi(token);//strtol(token,NULL,10);
	int i=sscanf(datapntr,"%1i@%10lu@%10lu\0",&data_pak.type,&data_pak.time_local,&data_pak.time_master);
	printf("type: %i\n",data_pak.type);
	printf("time_local: %lu\n",data_pak.time_local);
	printf("time_maste: %lu\n",data_pak.time_master);
	
//	printf("data.type %d\n",data_pak.type);
	
	switch(data_pak.type){
		case 0:
			break;
		case 1:
	  		bigTime = getTime(&timeIterator); 
  			time_local=bigTime;
 			time_master=bigTime;
			data_pak.time_local=time_local;//gTime;
			data_pak.time_master=time_master;//igTime;
			data_pak.type=2;
			printf("Master. Zeit gesetzt lokale zeit: %lu \n ",time_local);//e_master
			break;
		case 2:
			break;
		//default:
		//	bla;
		//break
		
	}
	if(!rimeaddr_cmp(from, &rimeaddr_node_addr)){	
	  	char string[30];
	  	int size=snprintf(string,30,"%1i@%10lu@%10lu",data_pak.type,data_pak.time_local,data_pak.time_master);
	        packetbuf_copyfrom(&string,sizeof(string)); // String + Length to be send
		printf("sent message  %s with len %d \n",string,strlen(string));
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
 // SENSORS_ACTIVATE(button_sensor);//activate button
  unicast_open(&uc, 290, &unicast_callbacks); // channel = 122
  static struct etimer et;
  while(1){
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
