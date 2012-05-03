/**
 * @author Robert Hartung, hartung@ibr.cs.tu-bs.de
 * 
 */

#include "contiki.h"
#include "net/rime.h"

#include "dev/button-sensor.h"
#include "dev/leds.h"
#include <stdio.h>
#define SECRET  "eihais8ca6Aej0"

/*---------------------------------------------------------------------------*/

PROCESS(rime_unicast_sender, "Rime Unicast Sender");
AUTOSTART_PROCESSES(&rime_unicast_sender);

/*---------------------------------------------------------------------------*/
static int zaehler=0;
static int bitzaehler=0;

static void recv_uc(struct unicast_conn *c, const rimeaddr_t *from)
{

  	char *datapntr;
	datapntr = packetbuf_dataptr();
	datapntr[15] = '\0';
	if(strcmp(datapntr,SECRET)==0){
		printf("unicast message received from %d.%d: '%s'\n", from->u8[0], from->u8[1], datapntr);
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
		printf ("zaehler: %d \n",bitzaehler);
	}else{
		printf("unicast message received from wrong node %d.%d: '%s'\n", from->u8[0], from->u8[1], datapntr);
	}
}

static const struct unicast_callbacks unicast_callbacks = {recv_uc};
static struct unicast_conn uc;

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(rime_unicast_sender, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&uc));
  
  PROCESS_BEGIN();

  leds_init();
  leds_off(LEDS_ALL);
  SENSORS_ACTIVATE(button_sensor);//activate button
  unicast_open(&uc, 290, &unicast_callbacks); // channel = 122

  while(1)
  {
    static struct etimer et;
    rimeaddr_t addr;
    
    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);

    //etimer_set(&et, CLOCK_SECOND);
    //PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		
    packetbuf_copyfrom(SECRET, 15); // String + Length to be send
    //packetbuf_copyfrom("ekhais8ca6Aej0", 15); // String + Length to be send
    
    addr.u8[0] = 0x22; // Address of receiving Node
    addr.u8[1] = 0x1;
    
    if(!rimeaddr_cmp(&addr, &rimeaddr_node_addr))
    {
    	printf("Message sent\n"); // debug message
      unicast_send(&uc, &addr);
    }
  }

  PROCESS_END();
}
