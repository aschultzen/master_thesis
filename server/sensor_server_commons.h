#ifndef SENSOR_SERVER_COMMONS_H
#define SENSOR_SERVER_COMMONS_H

#include "net.h"
#include <semaphore.h>
#include "colors.h"

/* STRUCTS */

/*
* command_code struct is used by the parser
* to convey an easy to compare command code, as well
* as any parameter belonging to that command
*/ 
struct command_code{
	int code;
	char parameter[MAX_PARAMETER_SIZE];
} __attribute__ ((packed));

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
	struct transmission_s transmission;
	struct timeval heartbeat_timeout;
	struct command_code cm;
	struct nmea_container nmea;
	pid_t pid;
	time_t timestamp;
	time_t warmup_started;				
	int client_id;							
	int client_type;							
	int checksum_passed;
	int warmup;
	int moved;
 	char ip[INET_ADDRSTRLEN];
} __attribute__ ((packed));

/* Server info shared with processes */
struct server_data{
	int number_of_clients;	/* Number of clients currently connected */
	int number_of_sensors;	/* Number of sensors, subset of clients */
	time_t started;			/* When the server was started */
	pid_t pid;
	char version[4];
} __attribute__ ((packed));

/* Synchronization elements shared with processes */
struct server_synchro{
	sem_t ready_mutex;
	sem_t client_list_mutex;
	volatile int ready_counter;
} __attribute__ ((packed));

/* 
* Roles of client, either SENSOR or MONITOR. 
* A monitor is only used to monitor the programs state.
*/
enum client_type{
    SENSOR,
    MONITOR
};

#endif /* !SENSOR_SERVER_COMMONS_H */