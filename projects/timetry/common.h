#ifndef COMMON_H
#define COMMON_H

#include "net/rime.h"
#include "timesync.h"


void sendDatagram(struct unicast_conn *c, const rimeaddr_t *to, struct datagram* data_pak);
void readDatagram(struct unicast_conn *c, const rimeaddr_t *from, struct datagram* data_pak);
void printFloat(float f);

#endif
