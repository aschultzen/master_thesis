SERVER_OBJS = sensor_server.o net.o utils.o serial.o
CLIENT_OBJS = sensor_client.o net.o utils.o 

CC = gcc
DEBUG = -g

CFLAGS = -Wall -c $(DEBUG) -std=gnu99 -pedantic -O2
#CFLAGS = -Wall -c $(DEBUG) -march=armv7-a -mtune=arm7 -I/usr/include/mysql

LFLAGS = -Wall $(DEBUG)

server : $(SERVER_OBJS)
	$(CC) $(LFLAGS) $(SERVER_OBJS) -o server

client : $(CLIENT_OBJS)
	$(CC) $(LFLAGS) $(CLIENT_OBJS) -o client

sensor_server.o : sensor_server.h net.h sensor_server.c
	$(CC) $(CFLAGS) sensor_server.c

sensor_client.o : sensor_client.h sensor_client.c
	$(CC) $(CFLAGS) sensor_client.c

net.o : net.h utils.h net.c 
	$(CC) $(CFLAGS) net.c

utils.o : utils.h list.h utils.c
	$(CC) $(CFLAGS) utils.c

serial.o : serial.h serial.c
	$(CC) $(CFLAGS) serial.c

clean:
	\rm *.o
