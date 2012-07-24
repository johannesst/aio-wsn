#include "net/rime.h"
#include "timesync.h"
#include <stdio.h>
#include <string.h>
/**
 * Sends a datagram. All parametes must point to initalized data structures.
 */
void sendDatagram(struct unicast_conn *c, const rimeaddr_t *to, struct datagram* data_pak)
{
	char string[30];
	snprintf(string,30,"%1i@%10lu@%10lu", data_pak->type, data_pak->time_local, data_pak->time_master);
	packetbuf_copyfrom(&string,sizeof(string)); // String + Length to be send
	unicast_send(c, to);
	printf("Sent message %s to %x-%x\n", string, to->u8[1], to->u8[0]);
}

/**
 * Decodes a datagram that has been received. All parametes must point to initalized/allocated data structures. Data is read from the input buffer and written into data_pak.
 */
void readDatagram(struct unicast_conn *c, const rimeaddr_t *from, struct datagram* data_pak)
{
  	char *datapntr;
	datapntr = packetbuf_dataptr();
	printf("Received message %s from %x-%x\n", datapntr, from->u8[1], from->u8[0]);
	
	// convert received string to a datagram struct:
	int i=sscanf(datapntr,"%1i@%10lu@%10lu\0",&data_pak->type,&data_pak->time_local,&data_pak->time_master);
}

/*
void printFloat(float f)
{
	printf("%i.", (int)f);
	if(f < 0.0f)
		f *= -1.0f;
	f -= (int)f;

	int i;
	for(i = 0; i < 6; i++)
	{
		f /= 10.0f;
		printf("%i", (int)f);
	}
}
*/
