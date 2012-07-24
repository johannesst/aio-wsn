#ifndef TIMESNYC_H
#define TIMESNYC_H

unsigned long getTime(long *);
unsigned long getTimeSystem();

struct datagram{
	int type; //switch (type){
		  //case 0 bestätigung
		  //case 1 ping paket vom slave
		  //case 2 pong paket vom master
		  //case 3 sende akkustisches signal 
		  //case 4 empfange akkustisches signal
	unsigned long int time_local;
	unsigned long int time_master;
	char end;
};

#endif
