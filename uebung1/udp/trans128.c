/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include "dev/button-sensor.h"
#include "dev/leds.h"
#include <string.h>

#include "simple-udp.h"
#include "net/uip.h"
#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

#define MAX_PAYLOAD_LEN 15
#define IP_TRANS122		0xfe80,0,0,0,0x2001,0x22ff,0xfe33,0x4455
#define IP_TRANS128		0xfe80,0,0,0,0x2a01,0x22ff,0xfe33,0x4455
#define UDP_PORT		3000
#define SECRET			"eihais8ca6Aej0\n"

static int zaehler=0;
static int bitzaehler=0;
static struct  simple_udp_connection udp_connection;

PROCESS(udp_server_process, "UDP server process");
AUTOSTART_PROCESSES(&udp_server_process);
/*---------------------------------------------------------------------------*/

static void send_data(const uip_ipaddr_t *addr){
simple_udp_sendto(&udp_connection, SECRET , MAX_PAYLOAD_LEN,addr);
}
/*---------------------------------------------------------------------------*/

static void 
receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{

//    PRINTF("Received unicast  message '%s' from %s",(char *) data, (char *) sender_addr);

	if(strcmp((char *)data,SECRET)==0){
		switch(zaehler){
			case 6:	
				leds_on(LEDS_ALL);
				zaehler=0;
				bitzaehler=0;
				break;
			case 5:	
				leds_on(LEDS_ALL);
				zaehler=6;
				bitzaehler++;
				break;
	
			case 4:
				leds_off(LEDS_GREEN);
				leds_on(LEDS_YELLOW);
			//	bitzaehler=2;
				zaehler=5;
				break;
			case 3:
				leds_off(LEDS_GREEN);
				leds_on(LEDS_YELLOW);
				bitzaehler++;
				zaehler=4;
				break;
			case 2:
				leds_on(LEDS_GREEN);
				leds_off(LEDS_YELLOW);
				zaehler=3;
				//bitzaehler=1;
				break;
	
			case 1:
				leds_on(LEDS_GREEN);
				leds_off(LEDS_YELLOW);
				zaehler=2;
				bitzaehler++;
				break;
	
			case 0:
				leds_off(LEDS_ALL);
				zaehler=1;
				bitzaehler=0;
				break;
			default:
				leds_off(LEDS_ALL);
				zaehler=0;
				bitzaehler=0;
			break;
		}
		PRINTF ("zaehler: %d \n",bitzaehler);
	}else{
		PRINTF("unicast message received from wrong node  %s\n",(char *) data);
    	//	PRINT6ADDR(sender_addr);
	}


  }


/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{

//  uint16_t ipaddr=(uint16_t) IP_TRANS128;
//#if UIP_CONF_ROUTER
  uip_ipaddr_t ipaddr;//=IP_TRANS128;
  uip_ip6addr(&ipaddr,0xfe80,0,0,0,0x2001,0x22ff,0xfe33,0x4455);
//#endif /* UIP_CONF_ROUTER */

  PROCESS_BEGIN();



  SENSORS_ACTIVATE(button_sensor);//activate button
  leds_init();
  leds_off(LEDS_ALL);
  simple_udp_register(&udp_connection, UDP_PORT, NULL, UDP_PORT, receiver);


  while(1) {
//	  printf("bla1");
   PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
  // PROCESS_YIELD();
   send_data(&ipaddr);
    
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
