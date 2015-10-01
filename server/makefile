OBJS = sensor_server.o net.o utils.o 
CC = gcc
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG) -std=gnu99 -pedantic -O2
#CFLAGS = -Wall -c $(DEBUG) -march=armv7-a -mtune=arm7

LFLAGS = -Wall $(DEBUG)

server : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o server

sensor_server.o : sensor_server.h net.h sensor_server.c
	$(CC) $(CFLAGS) sensor_server.c

net.o : net.h utils.h net.c 
	$(CC) $(CFLAGS) net.c

utils.o : utils.h utils.c
	$(CC) $(CFLAGS) utils.c

clean:
	\rm *.o
