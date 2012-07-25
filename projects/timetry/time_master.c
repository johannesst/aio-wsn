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
#include "display.h"
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
	slave_item.slaveAddr=*from;
	readDatagram(c,from,&data_pak);
	int i;
	struct slave_list_struct *tmp_slave;
	tmp_slave=list_head(slave_list);
	// is slave already in slave_list? If not, add him :)
	for(i=0;i<list_length(slave_list);i++){
		if(tmp_slave->slaveAddr.u8[0]==from->u8[0])
			if(tmp_slave->slaveAddr.u8[1]==from->u8[1]){
				list_remove(slave_list,&tmp_slave);
		}
		tmp_slave=list_item_next(&tmp_slave);
	}
	list_add(slave_list,&slave_item);
	
	switch(data_pak.type){
		case 1: // client asks for time
			// data_pak.time_local remains unchanged
			data_pak.time_master=getTimeSystem();
			data_pak.type=2;
			break;
		case 5: // client reports a beep
			printf("Client %x-%x heard a beep at %lu (= %lu ms).\n", from->u8[1], from->u8[0], data_pak.time_local, sysToMilli(data_pak.time_local));
			return;
		default:
			return; // Lena: when anything other then 1 is the case, we do not send an answer
		
	}
	// send reply only if the receiver is different from self

	if(!rimeaddr_cmp(from, &rimeaddr_node_addr))
		sendDatagram(c, from, &data_pak);
}

static struct unicast_callbacks unicast_callbacks = {recv_uc};

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(master_time_sync, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&uc));
  PROCESS_BEGIN();

  iAmTheMaster = 1;

  list_init(slave_list);
  unicast_open(&uc, 290, &unicast_callbacks); 
  initNetwork(&unicast_callbacks);

/*  printf("FUFUUUuuuuuuUUUU verification Erroe!");
  printf("fffffffffffFUFUUUuuuuuuUUUU verification Erroe!");
  printf("FUFUUUuuuuuuUUUU verification Erroe!");
  printf("FUFUUUuuuuuuUUUU verification Erroe!");
*/
  drawTable();
  gotoXY(1,20);
  printf("I am the MASTER, I have the RIME address %x-%x\n", rimeaddr_node_addr.u8[1], rimeaddr_node_addr.u8[0]);
 
  static struct etimer et;
  struct datagram data_pak;
  static rimeaddr_t masterAddr;
  masterAddr.u8[0] = MASTER_ADDR_0;
  masterAddr.u8[1] = MASTER_ADDR_1;
  data_pak.type=4;
  while(1){
  	gotoXY(1,1);
	unsigned long int time = getTimeCorrected();
	
	printf("Current time: %8lu,%3lu s", time / 1000, time % 1000);
	gotoXY(1,20);


	// for any element in slave_list send a beep command
	list_t tmplist=slave_list;	
	int i;
	struct slave_list_struct *tmp_slave;
	tmp_slave=list_head(slave_list);
	for(i=0;i<list_length(tmplist);i++){
		data_pak.time_local=getTimeCorrected();
		data_pak.time_master=getTimeCorrected()+milliToSys(1000);
		sendDatagram(&uc,tmp_slave.slaveAddr,&data_pak);	
		tmp_slave=list_item_next(tmp_slave);

	}
	// except react to incoming messages
	etimer_set(&et, CLOCK_SECOND);
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	//	PROCESS_YIELD(); 
  }
  
  

  PROCESS_END();
}
