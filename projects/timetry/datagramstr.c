#include "contiki.h"
#include "net/rime.h"

#include "dev/button-sensor.h"
#include <stdio.h>
#include <string.h>
#include "rtimer.h"
#include "clock.h"
#include "timesync.h"

char* datagramtostr (struct datagram data){
	char* string[37];
	snprintf(string,37,"%d@%lu@%lu@%c",data.type,data.time_local,data.time_master,data.end);
	return string;
}

void  strtodatagram(char* string,struct* datagram data){
	char* input=string;
	char* token;
	token=strtok(input,"@");
	data.type=atoi(token);
	for(int i=0;i<2;i++){
		if(token!=NULL){
			token=strtok(NULL,"@");
			if(i=0){
				data.time_local=strtoul(token,NULL,10);
			}else if(i=1){
				data.time_master=strtoul(token,NULL,10);
			}else if(i=2){
				data.end='\0';//=token
			}
		}
	}
}

