
#ifndef TIMESNYC_H
#define TIMESNYC_H

			

unsigned long getTimeCorrected();
unsigned long getTimeSystem();

long milliToTimer(long milli);
long milliToSys(long milli);
long sysToMilli(long sys);
long sysToTimer(long sys);

struct datagram{
	int type; //switch (type){
		  //case 1 ping paket vom slave
		  //case 2 pong paket vom master
		  //case 3 sende akkustisches signal 
		  //case 4 empfange akkustisches signal
	unsigned long int time_local;
	unsigned long int time_master;
	char end;
};

#endif
