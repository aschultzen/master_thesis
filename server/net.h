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
#include "protocol.h"

/* MOVE TO DEFS.H?*/
/* GENERAL */
#define MAX_CONNECTIONS 10
#define BUFFER_SIZE 512
#define CLIENT_MAX 10
#define MONITOR_MAX 2
#define CLIENT_TIMEOUT 5
#define MONITOR_TIMEOUT 100
#define DISPLAY_SIZE 8
#define MAX_COMMAND_SIZE 20
#define MAX_PARAMETER_SIZE 100

struct comm{
	int command_code;
	char parameter[MAX_PARAMETER_SIZE];
} __attribute__ ((packed));

//Size 32 bytes
struct session_info{
	int session_fd;				/* 4 B */
	int client_id;				/* 4 B */
	void *iobuffer; 			/* 8 B */	// Deprecated?
	struct timeval tv; 			/* 16 B */
	char ip[INET_ADDRSTRLEN]; 	/* 16 B */
	struct comm cm;				/* 120 B */
} __attribute__ ((packed));

int s_read(struct session_info *s_info);
int s_write(struct session_info *s_info, char *message, int length);
int parse_input(struct session_info *s_info);


#endif /* !NET_H */