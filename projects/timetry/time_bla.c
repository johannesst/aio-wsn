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
#include "list.h"

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
	struct  slave_list_struct slave_item;
	slave_item.slaveAddr=from;
	slave_item.connection=c;
	readDatagram(c,from,&data_pak);
	int i;//
	struct slave_list_struct tmp_slave;
	tmp_slave=list_head(slave_list);
	for(i=0;i<list_length(slave_list);i++){
		if(tmp_slave.slaveAddr=from){
			list_remove(slave_list,tmp_slave);
		}
		tmp_slave=list_item_next(tmp_slave);
	}
	list_add(slave_list,slave_item);
	
	switch(data_pak.type){
		case 1: // client asks for time
			// data_pak.time_local remains unchanged
			data_pak.time_master=getTimeSystem();
			data_pak.type=2;
			break;
		case 3:

		default:
			return; // Lena: when anything other then 1 is the case, we do not send an answer
		
	}
	// send reply only if the receiver is different from self

	if(!rimeaddr_cmp(from, &rimeaddr_node_addr))
		sendDatagram(c, from, &data_pak);
}

static const struct unicast_callbacks unicast_callbacks = {recv_uc};
static struct unicast_conn uc;

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(master_time_sync, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&uc));
  PROCESS_BEGIN();

  iAmTheMaster = 1;

  list_init(slave_list);
  unicast_open(&uc, 290, &unicast_callbacks); 
  printf("I am the MASTER, I have the RIME address %x-%x\n", rimeaddr_node_addr.u8[1], rimeaddr_node_addr.u8[0]);

  struct datagram data_pak;
  static struct etimer et;
  while(1){
	// do nothing, except react to incoming messages
	// for any element in slave_list send a beep command
	list_t tmplist=slave_list;	
	int i;
	
	tmp_slave=list_head(slave_list);
	for(i=0;i<list_length(slave_list);i++){
		data_pak.time_master=getTimeCorrected()+1000;
		data_pak.type

	}
	  PROCESS_YIELD(); 
  }
  
  

  PROCESS_END();
}
