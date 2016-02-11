#ifndef NET_H
#define NET_H

#define _GNU_SOURCE 1
#include <unistd.h>
#include <sys/mman.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdbool.h>

/* My own header files */ 
#include "utils.h"
#include "protocol.h"

/* GENERAL */
#define SERVER_MAX_CONNECTIONS 10
#define IO_BUFFER_SIZE MAX_PARAMETER_SIZE
#define MAX_CLIENTS 10
#define ID_MAX 1000	//Highest ID number allowed
#define MONITOR_MAX 2
#define CLIENT_TIMEOUT 5
#define MONITOR_TIMEOUT 100
#define DISPLAY_SIZE 8
#define CONNECTION_ATTEMPTS_MAX 10

/*
* CLIENT TABLE STRUCT 
*
* list_head list: The head in the list of clients
* pid: Process ID for the client connection (See "fork")
* session_fd: The file descriptor for the session. 
* client_id: The connected clients ID
* iobuffer: A general purpose buffer for in and output
* heartbeat_timeout: Number of seconds of inactivity before disconnect
* ip: Clients IP Address.
* cm: Command code. Used for quick comparison after commands
* are parsed by command parser.
*/
struct client_table_entry{ 
	struct list_head list;
	pid_t pid;
	int session_fd;					
	int client_id;							
	char iobuffer[IO_BUFFER_SIZE]; 
	int client_type;							
	struct timeval heartbeat_timeout; 							
	struct command_code cm;	
	char ip[INET_ADDRSTRLEN];
	struct nmea_container nmea; 
} __attribute__ ((packed));

int s_read(struct client_table_entry *cte);
int s_write(struct client_table_entry *cte, char *message, int length);
int parse_input(struct client_table_entry *cte);
int protocol_send(struct client_table_entry *cte, char *message);

#endif /* !NET_H */