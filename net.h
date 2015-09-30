#ifndef NET_H
#define NET_H

//Strict C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/wait.h>

// Mine
#include "utils.h"

/*
* Starts the server.
* Takes a port as param.
*/
int start_server(int port);

/* GENERAL */
#define BUFFER_SIZE 512
#define TIME_OUT 10
#define CLIENT_MAX 10
#define MONITOR_MAX 2

/* PROTOCOL */
#define DISCONNECT "DISCONNECT"
#define GET_TIME "GET_TIME"
#define IDENTIFY "IDENTIFY"
#define STORE "STORE"

#endif /* !NET_H */