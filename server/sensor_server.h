#ifndef SENSOR_SERVER_H
#define SENSOR_SERVER_H

#include <fcntl.h>
#include <sys/stat.h> 
#include <semaphore.h>

// Mine
#include "net.h"
#include "session.h"
#include "nmea.h"

/* 
* Made extern because the sessions should
* exit if the server is given a SIGINT/TERM
*/
extern volatile sig_atomic_t done;

/* Used in the session source code */
extern struct client_table_entry *client_list;
extern struct server_data *s_data;
extern struct server_synchro *s_synch;
extern struct config *cfg;

#define PROGRAM_VERSION "0.1a"

void remove_client_by_id(int id);
struct client_table_entry* get_client_by_id(int id);

/* Server info shared with processes */
struct server_data{
	int number_of_clients;	/* Number of clients currently connected */
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

#endif /* !SENSOR_SERVER_H */