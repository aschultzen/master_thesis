#ifndef SENSOR_SERVER_H
#define SENSOR_SERVER_H

/* See sensor_server_client_notes.md for details */

// Mine
#include "net.h"
#include "utils.h"

struct client_table_entry{ 
	int id;
	pid_t pid;
} __attribute__ ((packed));

#endif /* !SENSOR_SERVER_H */