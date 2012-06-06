#include "contiki.h"
#include "net/rime.h"

#include "dev/button-sensor.h"
#include <stdio.h>
#include <string.h>
#include "rtimer.h"
#include "clock.h"
#include "timesync.h"

void getTime(static long *bigTime,static long *timeIterator,unsigned long *time_local,unsigned long *time_master){
	

  rtimer_clock_t rtime=RTIMER_NOW();
  if (rtime<tmp){
	timeIterator++;
  }
  tmp=rtime;
  bigTime = ((unsigned long)rtime) + 	((unsigned long)timeIterator << 16);
  time_local=bigTime;
  time_master=bigTime;
}

void setDataTime(struct datagram *data,static long *bigTime){
	data.time_local=bigTime;
	data.time_master=bigTime;
}
