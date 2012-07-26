#ifndef COMMON_H
#define COMMON_H

#include "net/rime.h"
#include "timesync.h"

// Network stuff
#define MASTER_ADDR_1 0x01
#define MASTER_ADDR_0 0x28

struct unicast_conn uc;
rimeaddr_t masterAddr;

void initNetwork(struct unicast_callbacks* cb);
void sendDatagram(struct unicast_conn *c, const rimeaddr_t *to, struct datagram* data_pak);
void readDatagram(struct unicast_conn *c, const rimeaddr_t *from, struct datagram* data_pak);



// Audio out
unsigned long int nextBeepTime;
void beepOn(char beepPort);
void beepOff(char beepPort);
void beepAllOff();

#endif
