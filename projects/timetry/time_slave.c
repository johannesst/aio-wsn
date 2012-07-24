/**
 * @author Johannes Starosta based on an example by Robert Hartung, hartung@ibr.cs.tu-bs.de
 * 
 */

#include "contiki.h"
#include "net/rime.h"
#include "common.h"

#include "dev/button-sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rtimer.h"
#include "clock.h"
#include "timesync.h"
#include "common.h"

/*---------------------------------------------------------------------------*/
// Config area:
#define MASTER_ADDR_1 0x01
#define MASTER_ADDR_0 0x33
/*---------------------------------------------------------------------------*/

PROCESS(time_unicast_sender, "time Unicast Sender SLAVE");
AUTOSTART_PROCESSES(&time_unicast_sender);

static long int offset = 0;
static unsigned long int lastSysTimeSent = 0;

/**
 * Called by the system when a packet is received.
 */
static void recv_uc(struct unicast_conn *c, const rimeaddr_t *from)
{
  	struct datagram data_pak;
	readDatagram(c,from,&data_pak);
	
	switch(data_pak.type){
		case 1:
//			data_pak.time_local;//me_local;
//			data_pak.time_master;//=time_master;
//			data_pak.type=2;
//			break;
			return;
		case 2:
			{
			unsigned long int now = getTimeSystem() - offset;
			unsigned long int pingPongTime = now - data_pak.time_local;
			printf("Message traveled back and forth in %lu time units. ", pingPongTime);
			unsigned long int masterTimeNow = data_pak.time_master + pingPongTime / 2;
			long int dif = now - masterTimeNow;
			offset += dif;
			printf("Offset adjusted by %li, new offset is %li\n", dif, offset);
			long int timeBetweenMessages = now - lastSysTimeSent;
			int rate = dif * 1000L / timeBetweenMessages;
			printf("Time between Messages: %li. Change rate per time is %i.\n", timeBetweenMessages, rate);
			lastSysTimeSent = data_pak.time_local;
			return;
			}
		default:
			return; // Lena: when anything other then 2 is the case, we do not send an answer
	}

	if(!rimeaddr_cmp(from, &rimeaddr_node_addr))
		sendDatagram(c, from, &data_pak);
}

static const struct unicast_callbacks unicast_callbacks = {recv_uc};
static struct unicast_conn uc;

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(time_unicast_sender, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&uc));
  
  PROCESS_BEGIN();

  // Declare variables. Everything outside the loop should be static, because everything that is not static will loose its content when we enter the loop.
  static rimeaddr_t masterAddr;
  static struct etimer et;
  static struct datagram data_pak;

  unicast_open(&uc, 290, &unicast_callbacks);

  printf("I am a SLAVE, I have the RIME address %x-%x\n", rimeaddr_node_addr.u8[1], rimeaddr_node_addr.u8[0]);

  // some dummy data, used to change program size in case of verification errors.
  char* dummy = "sdfsdgdfdfffhewkfheskgfhesfhewthherghud";
  printf("Used dummy data: %i", strlen(dummy));
  printf("Used dummy data: %i", strlen(dummy));

//  float f = -177.245463453f;
//  printFloat(f);
//  printf(" == -177.245463453f ?\n");

  masterAddr.u8[0] = MASTER_ADDR_0;
  masterAddr.u8[1] = MASTER_ADDR_1;



  data_pak.time_master=0L;
  data_pak.type=1;

  while(1){
	data_pak.time_local = getTimeSystem() - offset;
	sendDatagram(&uc, &masterAddr, &data_pak);

	etimer_set(&et, CLOCK_SECOND);
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }

  PROCESS_END();
}
