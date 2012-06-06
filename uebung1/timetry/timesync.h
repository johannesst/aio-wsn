//#include "clock.h"

/*---------------------------------------------------------------------------*/
//static int zaehler=0;
//static int bitzaehler=0;
//rtimer_clock_t offset;
unsigned long time_local;
unsigned long time_master;
long timeIterator;
static long bigTime;
long getTime(long *);
struct datagram{
	int type; //switch (type){
		  //case 0 bestätigung
		  //case 1 ping paket vom slave
		  //case 2 pong paket vom master
		  //case 3 sende akkustisches signal 
		  //case 4 empfange akkustisches signal
	unsigned long time_local;
	unsigned long time_master;
	char end;
};
