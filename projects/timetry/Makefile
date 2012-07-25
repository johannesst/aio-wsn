ifeq ($(CONTIKI),)
CONTIKI = ../../../contiki-inga
endif
#WITH_UIP6=1
#UIP_CONF_IPV6=1
#CFLAGS+= -DUIP_CONF_IPV6_RP

#all: trans122 trans128
all: time_master time_slave 

CONTIKI_SOURCEFILES += getTime.c
CONTIKI_SOURCEFILES += common.c
CONTIKI_SOURCEFILES += listen.c



include $(CONTIKI)/Makefile.include
