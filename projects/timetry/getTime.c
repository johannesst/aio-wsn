#include "contiki.h"
#include "net/rime.h"

#include "dev/button-sensor.h"
#include <stdio.h>
#include <string.h>
#include "rtimer.h"
#include "clock.h"
#include "timesync.h"

long int offset = 0;
long int rate = 0;
char iAmTheMaster = 0;
unsigned long int lastSysTimeSent = 0;

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

/**
 * Returns corrected time as long int (64 bit) value. This is only working after 3 or 4 time sync messages have been exchanged between client and server.
 *
 * One unit equals about 0.13 milliseconds, which might be 1000 or 1024 CPU clocks at 8 MHz.
 */
unsigned long getTimeCorrected()
{
  long int sysTimeNow = getTimeSystem();

  if(iAmTheMaster)
	return sysTimeNow;

  long int timeBetweenMessages = sysTimeNow - lastSysTimeSent;
  long int driftSinceThen = rate * timeBetweenMessages / 1000L;


  unsigned long ret = sysTimeNow - offset - driftSinceThen;
  return ret;
}

// CLOCK_SECOND * 3 ~~ 23892 system time units
// CLOCK_SECOND ~~ 7964
// 1 system time unit ~~ 0,125565ms

long milliToTimer(long milli)
{
	return milli * ((long)(CLOCK_SECOND)) / 1000L;
}

long milliToSys(long milli)
{
	return milli * 7964L / 1000L;
}

long sysToMilli(long sys)
{
	return sys * 1000 / 7964L;
}

long sysToTimer(long sys)
{
	return sys * ((long)(CLOCK_SECOND)) / 7964L;
}
