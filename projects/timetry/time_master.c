/**
 * @author Johannes Starosta based on an example by Robert Hartung, hartung@ibr.cs.tu-bs.de
 * 
 */

#include "time_master.h"

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

LIST(slave_list);
MEMB(slave_mem, struct slave_list_struct, MAX_SLAVES);

/**
 * Called by the system when a packet is received.
 */
static void recv_uc(struct unicast_conn *c, const rimeaddr_t *from)
{
    	struct datagram data_pak;
	
	readDatagram(c,from,&data_pak);
	int i;
	struct slave_list_struct *tmp_slave;

	// is slave already in slave_list? If not, add him :)
	char found = 0;
	for(tmp_slave = list_head(slave_list); tmp_slave != NULL; tmp_slave = list_item_next(tmp_slave)) {
			if(tmp_slave->slaveAddr.u8[0]==from->u8[0] && tmp_slave->slaveAddr.u8[1]==from->u8[1])
			{
				found = 1;
				// list_remove(slave_list,&tmp_slave);
			}
		tmp_slave=list_item_next(&tmp_slave);
	}
	if(!found)
		addSlave(from);
	
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

void addSlave(const rimeaddr_t *addr)
{
	printf("Packet from unknown slave %x-%x.\n", addr->u8[1], addr->u8[0]);

	struct  slave_list_struct* slave_item = memb_alloc(&slave_mem);
	slave_item->slaveAddr=*addr;
	slave_item->next = NULL;
	list_add(slave_list,slave_item);
	drawTable(slave_list);
}

static struct unicast_callbacks unicast_callbacks = {recv_uc};

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(master_time_sync, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&uc));
  PROCESS_BEGIN();

  iAmTheMaster = 1 ;

  list_init(slave_list);
  unicast_open(&uc, 290, &unicast_callbacks); 
  initNetwork(&unicast_callbacks);

  // Add the master as first "slave"
  addSlave(&rimeaddr_node_addr);

 // printf("FUFUUUuuuuuuUUUU verification Erroe!");
/*  printf("fffffffffffFUFUUUuuuuuuUUUU verification Erroe!");
  printf("FUFUUUuuuuuuUUUU verification Erroe!");
  printf("FUFUUUuuuuuuUUUU verification Erroe!");
*/
  clearScreen();
  drawTable(slave_list);
  gotoXY(1,20);
  printf("I am the MASTER, I have the RIME address %x-%x\n", rimeaddr_node_addr.u8[1], rimeaddr_node_addr.u8[0]);
 
  static struct etimer et;
  struct datagram data_pak;
  static rimeaddr_t masterAddr;
  masterAddr.u8[0] = MASTER_ADDR_0;
  masterAddr.u8[1] = MASTER_ADDR_1;
  data_pak.type=4;
  struct slave_list_struct master_entry;
  master_entry.slaveAddr=masterAddr;
  list_add(slave_list,&master_entry);
  while(1){
//	gotoXY(1,1);
	unsigned long int time = getTimeCorrected();
	
//	printf("Current time: %8lu,%3lu s", time / 1000, time % 1000);
//	gotoXY(1,20);


	// for any element in slave_list send a beep command
	struct slave_list_struct *tmp_slave;
	
	fillTable(slave_list);

	for(tmp_slave = list_head(slave_list); tmp_slave != NULL; tmp_slave = list_item_next(tmp_slave)) {
		data_pak.time_local=getTimeCorrected();
		data_pak.time_master=getTimeCorrected()+milliToSys(1000);

		sendDatagram(&uc,&tmp_slave->slaveAddr,&data_pak);	

	}
	// except react to incoming messages
	etimer_set(&et, CLOCK_SECOND);
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	//	PROCESS_YIELD(); 
  }
  
  

  PROCESS_END();
}
