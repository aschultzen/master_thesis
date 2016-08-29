SERVER_OBJS = sensor_server.o net.o utils.o serial.o session.o filters.o actions.o
CLIENT_OBJS = sensor_client.o net.o utils.o serial.o

CC = gcc
DEBUG = -g

CFLAGS = -Wall -Wextra -c $(DEBUG) -std=gnu99 -pedantic -O3

cpu := $(shell uname -m)

ifeq ($(cpu),armv7l)
	CFLAGS = -Wall -c $(DEBUG) -march=armv7-a -mtune=arm7 -fsigned-char
endif

LFLAGS = -Wall $(DEBUG)

server : $(SERVER_OBJS)
	$(CC) $(LFLAGS) $(SERVER_OBJS) -o server -lpthread

client : $(CLIENT_OBJS)
	$(CC) $(LFLAGS) $(CLIENT_OBJS) -o client

sensor_server.o : sensor_server.h net.h sensor_server.c
	$(CC) $(CFLAGS) sensor_server.c 

sensor_client.o : sensor_client.h sensor_client.c
	$(CC) $(CFLAGS) sensor_client.c

net.o : net.h utils.h net.c 
	$(CC) $(CFLAGS) net.c

utils.o : utils.h list.h utils.c config.h
	$(CC) $(CFLAGS) utils.c

serial.o : serial.h serial.c
	$(CC) $(CFLAGS) serial.c

session.o : session.h session.c sensor_server.h
	$(CC) $(CFLAGS) session.c 

filters.o : filters.h filters.c sensor_server.h
	$(CC) $(CFLAGS) filters.c

actions.o : actions.h actions.c sensor_server.h
	$(CC) $(CFLAGS) actions.c 

clean:
	\rm *.o
