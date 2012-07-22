#include "contiki.h"
#include "net/rime.h"

#include "dev/button-sensor.h"
#include <stdio.h>
#include <string.h>
#include "rtimer.h"
#include "clock.h"
#include "timesync.h"

long getTime(long *timeIterator){
	
  static rtimer_clock_t tmp;

  rtimer_clock_t rtime=RTIMER_NOW();
  if (rtime<tmp){
	timeIterator++;
  }
  long i=(long) timeIterator;
  tmp=rtime;
  long bigTime = ((unsigned long)rtime) + ((unsigned long)i << 16);
  return bigTime;
}

