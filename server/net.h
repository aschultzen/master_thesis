#ifndef NET_H
#define NET_H

//Strict C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/mman.h>

// Mine
#include "utils.h"

/*
* Starts the server.
* Takes a port as param.
*/

//Size 32 bytes
struct session_info{
	int session_fd;				/* 4 B */
	int client_id;				/* 4 B */
	void *iobuffer; 			/* 8 B */
	struct timeval tv; 			/* 16 B */
	char ip[INET_ADDRSTRLEN]; 	/* 16 B */
} __attribute__ ((packed));

/* MOVE TO DEFS.H?*/
/* GENERAL */
#define MAX_CONNECTIONS 10
#define BUFFER_SIZE 512
#define CLIENT_MAX 10
#define MONITOR_MAX 2
#define CLIENT_TIMEOUT 5
#define MONITOR_TIMEOUT 100
#define DISPLAY_SIZE 8

/* PROTOCOL */
#define DISCONNECT "DISCONNECT"
#define GET_TIME "GET_TIME"
#define IDENTIFY "IDENTIFY"
#define STORE "STORE"
#define ILL_COM "ILLEGAL COMMAND\n"
#define NO_ID "CLIENT NOT IDENTIFIED\n"

int s_read(struct session_info *s_info);
int s_write(struct session_info *s_info, char *message, int length);


#endif /* !NET_H */