#include "contiki.h"
#include "net/rime.h"

#include "dev/button-sensor.h"
#include <stdio.h>
#include <string.h>
#include "rtimer.h"
#include "clock.h"
#include "timesync.h"


static unsigned long int timeUpper = 0;

/**
 * Returns time as long int (64 bit) value. One unit equals about 0.13 milliseconds, which might be 1000 or 1024 CPU clocks at 8 MHz.
 * IMPORTANT: You must call this function at least once every 8500ms - otherwise, the clock stops to work correctly.
 *
 * TODO Add another Contiki Process which runs every ~4000ms to update the value.
 */
unsigned long getTimeSystem()
{
  static rtimer_clock_t tmp;

  rtimer_clock_t rtime=RTIMER_NOW();
  if ((unsigned long)rtime < (unsigned long)tmp){
	timeUpper++;
  }
  tmp=rtime;
  return ((unsigned long)rtime) | (timeUpper << 16);
}

