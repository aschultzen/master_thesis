#ifndef SENSOR_SERVER_H
#define SENSOR_SERVER_H

#include <fcntl.h>
#include <sys/stat.h> 

// Mine
#include "session.h"
#include "serial.h"
#include "sensor_server_commons.h"

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

#endif /* !SENSOR_SERVER_H */