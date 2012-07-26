#ifndef TIME_MASTER_H
#define TIME_MASTER_H

#define MAX_SLAVES 12

#include "net/rime.h"

struct slave_list_struct{
	struct slave_list_struct  *next;
	rimeaddr_t slaveAddr;
	unsigned long int nextBeepTime;
	unsigned long int lastBeepHeard;
};

void addSlave(const rimeaddr_t *addr);


#endif 
