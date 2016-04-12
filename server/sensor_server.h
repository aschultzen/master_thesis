#ifndef SENSOR_SERVER_H
#define SENSOR_SERVER_H

#include <fcntl.h>
#include <sys/stat.h> 
#include "session.h"
#include "serial.h"
#include "sensor_server_commons.h"

/* VERSION */
#define PROGRAM_VERSION "0.1a"

/* ERRORS */
#define ERROR_MAX_CLIENTS_REACHED "CONNECTION REJECTED: MAXIMUM NUMBER OF CLIENTS REACHED\n"
#define ERROR_CONFIG_LOAD_FAILED "CONFIG LOAD FAILED: CONFIG FILE CORRUPTED\n"
#define ERROR_SEMAPHORE_CREATION_FAILED "SEMAPHORE CREATION FAILED\n"
#define ERROR_SOCKET_OPEN_FAILED "ERROR: FAILED TO OPEN SOCKET\n"
#define ERROR_SOCKET_BINDING "ERROR: FAILED TO BIND ON %d\n"
#define ERROR_CONNECTION_ACCEPT "ERROR: FAILED TO ACCEPT CONNECTION (%d)\n"
#define ERROR_FAILED_FORK "ERROR: FORK FAILED (%d)\n"
#define ERROR_MISSING_PARAMS "MISSING PARAMETERS!\n"

/* GENERAL STRINGS */
#define PROCESS_REAPED "Process %d reaped. Status: %d\n"
#define SIGTERM_RECEIVED "[%d] SIGTERM received!\n"
#define SIGINT_RECEIVED "[%d] SIGINT received!\n"
#define STOPPING_SERVER "Stopping server...\n"
#define CONFIG_LOADED "Config loaded!\n"
#define SERVER_RUNNING "Server is running. Accepting connections.\n"
#define WAITING_FOR_CONNECTIONS "Waiting for connections...\n"
#define CON_ACCEPTED "Connection accepted\n"
#define CLIENT_DISCONNECTED "[%d] Disconnected\n"
#define SERVER_STOPPED "Server STOPPED!\n"
#define SERVER_STARTING "Sensor server starting...\n"
#define CLIENT_KICKED "Client was kicked\n"

/* USAGE() STRINGS */
#define USAGE_DESCRIPTION "Required argument:\n\t -p <PORT NUMBER>\n\n"
#define USAGE_PROGRAM_INTRO "Sensor_server: Server part of GPS Jamming/Spoofing system\n\n"
#define USAGE_USAGE "Usage: %s [ARGS]\n\n"

/* CONFIG */
#define CONFIG_SERVER_MAX_CONNECTIONS "max_clients:"
#define CONFIG_SERVER_WARM_UP "warm_up:"
#define CONFIG_FILE_PATH "config.ini"
#define CONFIG_ENTRIES 2

/* 
* CONFIG STRUCT
* 
* Used as container for the config_loader function
* config_server_max_connections: max number of permitted connections
*/
struct server_config {
	int max_clients;
	int warm_up_seconds;
} __attribute__ ((packed));

/* 
* Made extern because the sessions should
* exit if the server is given a SIGINT/TERM
*/
extern volatile sig_atomic_t done;

/* Also used by session and action */
extern struct client_table_entry *client_list;
extern struct server_data *s_data;
extern struct server_synchro *s_synch;
extern struct server_config *s_conf;

void remove_client_by_id(int id);
struct client_table_entry* get_client_by_id(int id);
void print_server_data(struct client_table_entry *monitor);

#endif /* !SENSOR_SERVER_H */