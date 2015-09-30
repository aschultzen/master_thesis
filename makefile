OBJS = net.o utils.o sensor_server.o
CC = gcc
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG) -ansi -pedantic -O2
LFLAGS = -Wall $(DEBUG)

server : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o server

sensor_server.o : sensor_server.h net.h sensor_server.cpp
	$(CC) $(CFLAGS) sensor_server.cpp

net.o : net.h utils.h net.cpp 
	$(CC) $(CFLAGS) net.cpp

utils.o : utils.h utils.cpp
	$(CC) $(CFLAGS) utils.cpp

clean:
	\rm *.o
