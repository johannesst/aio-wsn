#ifndef TIME_MASTER_H
#define TIME_MASTER_H

#define MAX_SLAVES 12

#include "net/rime.h"

struct slave_list_struct{
	struct slave_list_struct  *next;
	rimeaddr_t slaveAddr;
};


#endif
