#ifndef SENSOR_SERVER_H
#define SENSOR_SERVER_H

#include <fcntl.h>
#include <sys/stat.h> 
#include "session.h"
#include "serial.h"
#include "sensor_server_commons.h"

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

/** @brief Prints information about the server.
 *
 * Transmits info about the server:
 * Time when started, PID, number of clients, 
 * number of sensors, max number of clients, 
 * sensor warm-up time and version. 
 * 
 * @param client MONITOR who made the request.
 * @return Void
 */
void print_server_data(struct client_table_entry *monitor);

#endif /* !SENSOR_SERVER_H */