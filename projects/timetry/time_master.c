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
char tableChanged = 1;

LIST(slave_list);
MEMB(slave_mem, struct slave_list_struct, MAX_SLAVES);



void handleDatagram(struct datagram  * data_pak, struct unicast_conn *c, const rimeaddr_t *from)
{
	struct slave_list_struct *tmp_slave;

	//printf("I got a packet.\n");

	// is slave already in slave_list? If not, add him :)
	char found = 0;
	for(tmp_slave = list_head(slave_list); tmp_slave != NULL; tmp_slave = list_item_next(tmp_slave)) {
			if(tmp_slave->slaveAddr.u8[0]==from->u8[0] && tmp_slave->slaveAddr.u8[1]==from->u8[1])
			{
				found = 1;
				break;
				// list_remove(slave_list,&tmp_slave);
			}
		tmp_slave=list_item_next(&tmp_slave);
	}
	if(!found)
		addSlave(from);
	
	switch(data_pak->type){
		case 1: // client asks for time
			// data_pak.time_local remains unchanged
			data_pak->time_master=getTimeSystem();
			data_pak->type=2;
			//printf("Client asked for time.\n");
			break;
		case 5: // client reports a beep
			tmp_slave->lastBeepHeard = data_pak->time_local;
			int i = 0;
			// sender im inne der akustischen Richtung
			struct slave_list_struct *tmp_sender;
			char funden = 0;
			for(tmp_sender = list_head(slave_list); tmp_sender != NULL; tmp_sender = list_item_next(tmp_sender)) {
				long int dif = data_pak->time_local - tmp_sender->nextBeepTime;
				if(dif > -milliToSys(20) && dif < milliToSys(150))
				{
					printf("From (Y) %x-%x to (X) %x-%x in %li ms.\n", tmp_sender->slaveAddr.u8[1], tmp_sender->slaveAddr.u8[0], from->u8[1], from->u8[0], sysToMilli(dif));
					tmp_slave->difFrom[i] = dif;
					tmp_slave->difFromSum[i] += dif;
					tmp_slave->difFromCount[i] ++;
					funden = 1;
				}
				i++;
			}
			if(!funden)
				printf("Unknown signal at  %x-%x, time is %li.\n", from->u8[1], from->u8[0], sysToMilli(data_pak->time_local));
			tableChanged=1;
			//printf("Client %x-%x heard a beep at %lu (= %lu ms).\n", from->u8[1], from->u8[0], data_pak->time_local, sysToMilli(data_pak->time_local));
			return;
		default:
			return; // Lena: when anything other then 1 is the case, we do not send an answer
		
	}
	// send reply only if the receiver is different from self

	//if(!rimeaddr_cmp(from, &rimeaddr_node_addr))
	sendDatagram(c, from, data_pak);
}

void addSlave(const rimeaddr_t *addr)
{
	printf("Packet from unknown slave %x-%x.\n", addr->u8[1], addr->u8[0]);

	struct  slave_list_struct* slave_item = memb_alloc(&slave_mem);
	slave_item->slaveAddr=*addr;
	slave_item->next = NULL;
	int i;
	for(i = 0; i < 5; i++)
	{
		slave_item->difFrom[i] = 0;
		slave_item->difFromSum[i] = 0;
		slave_item->difFromCount[i] = 0;
	}
	list_add(slave_list,slave_item);
	drawTable(slave_list);
	fillTable(slave_list);
}

void showTime()
{
	saveLocation();
	gotoXY(1,1);
	unsigned long int time = sysToMilli(getTimeCorrected());
	printf("Current time: %8lu,%3lu s", time / 1000, time % 1000);
	restoreLocation();
}


/*---------------------------------------------------------------------------*/

PROCESS_THREAD(master_time_sync, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&uc));
  PROCESS_BEGIN();

  iAmTheMaster = 1 ;

  list_init(slave_list);
  //unicast_open(&uc, 290, &unicast_callbacks); 
  initNetwork(&handleDatagram);

  // Add the master as first "slave"
  addSlave(&rimeaddr_node_addr);

//  printf("FUFUUUuuuuuuUUUU verification Erroe!");
/*  printf("fffffffffffFUFUUUuuuuuuUUUU verification Erroe!");
  printf("FUFUUUuuuuuuUUUU verification Erroe!");
  printf("FUFUUUuuuuuuUUUU verification Erroe!");
*/
  clearScreen();
  drawTable(slave_list);
  gotoXY(1,35);
  printf("I am the MASTER, I have the RIME address %x-%x\n", rimeaddr_node_addr.u8[1], rimeaddr_node_addr.u8[0]);
  
 
  static struct etimer et;
  static struct datagram data_pak;
  static rimeaddr_t masterAddr;
  masterAddr.u8[0] = MASTER_ADDR_0;
  masterAddr.u8[1] = MASTER_ADDR_1;
  data_pak.type=4;
  static unsigned long int nextPlanTime = 0;

  int zeit = 1000;

  while(1){

	showTime();

	// for any element in slave_list send a beep command
	struct slave_list_struct *tmp_slave;
	
	//fillTable(slave_list);

	unsigned long int now = getTimeCorrected();

	if(now > nextPlanTime)
	{
		fillTable(slave_list);
		int i = 0;

		for(tmp_slave = list_head(slave_list); tmp_slave != NULL; tmp_slave = list_item_next(tmp_slave)) {
			i++;
			tmp_slave->nextBeepTime = now +  (unsigned long)(milliToSys(zeit) * i);

			int n;
			for(n = 0; n < 6; n++)
				tmp_slave->difFrom[n] = 1000;			

			if(tmp_slave->slaveAddr.u8[0] == rimeaddr_node_addr.u8[0] && tmp_slave->slaveAddr.u8[1] == rimeaddr_node_addr.u8[1])
				nextBeepTime = tmp_slave->nextBeepTime;
			else
			{
				data_pak.time_local = now;
				data_pak.time_master = tmp_slave->nextBeepTime;
				sendDatagram(&uc,&tmp_slave->slaveAddr,&data_pak);	
			}
		}
		nextPlanTime = now + milliToSys(zeit) * (unsigned long)(i + 2);
		tableChanged = 1;
		gotoXY(1,27); printf("%c[%dJ", 0x1B, 0); // clear below
	setColor(YELLOW, BLACK, BRIGHT);
	gotoXY(48,27); printf(" ,------.  ,---.   ,--. ,--.    \n");
	gotoXY(48,28); printf(" |  .---' /  O  \\  |  | |  |    \n");
	gotoXY(48,29); printf(" |  `--, |  .-.  | |  | |  |    \n");
	gotoXY(48,30); printf(" |  |`   |  | |  | |  | |  '--. \n");
	gotoXY(48,31); printf(" `--'    `--' `--' `--' `-----' \n");
	gotoXY(48,32); printf("Failed Audio Indoor Localization \n");
	gotoXY(1,27);
	setColor(WHITE, BLACK, NORMAL);

		// printf("I made a new plan. Next plan will be made at %lu ms. (i is %i)\n", sysToMilli(nextPlanTime), i);
	}
	etimer_set(&et, milliToTimer(500));
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	//PROCESS_PAUSE(); 
//PROCESS_YIELD();
  }
  
  

  PROCESS_END();
}


