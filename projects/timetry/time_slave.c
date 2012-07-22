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
	/*
	struct datagram data_pak;
	printf("received from master %s with len %d  \n",datapntr,strlen(datapntr));
	data_pak.time_local=-255;
	data_pak.time_master=-255;
	char* token;
	token=strtok(datapntr,"@");
	printf("\n \n %s \n ",token);
	data_pak.type=strtol(token,NULL,10);
	printf("typ: %d",data_pak.type);
	int i =0;
	while(token!=NULL){
			token=strtok(NULL,"@");
			if(i==0){
				data_pak.time_local=strtoul(token,NULL,10);
			}else if(i==1){
				data_pak.time_master=strtoul(token,NULL,10);
			}else{
				break;
			}
	}
	//memcpy(&data_pak,datapntr,sizeof(datapntr));
	//antwortpaket basteln
	
	switch(data_pak.type){
		case 0:
			break;
		case 1:
			data_pak.time_local=time_local;
			data_pak.time_master=time_master;
			data_pak.type=2;
			break;
		case 2:
			data_pak.type=0;
			//printf("pong antwort vom master. Lokale Zeit: %lu Entfernte Zeit: %lu \n",data_pak.time_local,data_pak.time_master);
			time_local=data_pak.time_master;
			time_master=data_pak.time_master;
			break;
		//default:
		//	bla;
		//	break
	}
	//printf("unicast message received from %d.%d: %u,%lu,%lu \n", from->u8[0], from->u8[1], data.type,data.time_local,data.time_master);
	//printf("unicast message received from %d.%d: %lu \n", from->u8[0], from->u8[1], data);
	printf("Lokale Zeit alt: %lu \n ",bigTime);
	bigTime=data_pak.time_master;
	printf("Lokale Zeit neu: %lu \n ",bigTime);
	if(!rimeaddr_cmp(from, &rimeaddr_node_addr))
    	{
	  	snprintf(datapntr,21,"%d@%lu@%lu",data_pak.type,data_pak.time_local,data_pak.time_master);
	        packetbuf_copyfrom(&datapntr,sizeof(datapntr)); // String + Length to be send
    	//	printf("Answermessage sent\n"); // debug message
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
  struct datagram data_pak;
  bigTime = getTime(&timeIterator); 
  data_pak.time_local=time_local=bigTime;
  data_pak.time_master=time_master=-255;
  data_pak.type=1;
  int i = 0; 
  while(1){
	  static struct etimer et;
	  rimeaddr_t addr;
	  char string[30];
	  int size=snprintf(string,30,"%i@%lu@%lu",data_pak.type,time_master,time_local);
	  //if (size>=30){
		//  snprintf(dataptr,10+size,"%i@%lu@%lu");
//	  }
	  packetbuf_copyfrom(&string,strlen(string)+1); // String + Length to be send
//	  PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
	  addr.u8[0] = 0x28; // Address of receiving Node
	  addr.u8[1] = 0x1;
	
	 // etimer_set(&et, CLOCK_SECOND);
	 // PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		
	  if(!rimeaddr_cmp(&addr, &rimeaddr_node_addr)){
		  //printf("unicast message sent  %u,%lu,%lu \n",  data.type,data.time_local,data.time_master);
		  //printf("Message sent\n"); // debug message
		  unicast_send(&uc, &addr);
		  printf("unicast message sent   %s with length %d \n",  string,strlen(string)+1);//ta.time_local,data.time_master);
	 }
	//  */
	/*if (i<3){
		i++;
	}else if(i>=3)
		break;
		*/
  }

  PROCESS_END();
}
