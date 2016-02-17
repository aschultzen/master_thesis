#ifndef SENSOR_SERVER_H
#define SENSOR_SERVER_H

#include <fcntl.h>
#include <sys/stat.h> 
#include <semaphore.h>

// Mine
#include "net.h"
#include "session.h"

/* 
* Made extern because the sessions should
* exit if the server is given a SIGINT/TERM
*/
extern volatile sig_atomic_t done;

/* Used in the session source code */
extern struct client_table_entry *client_list;

/* Name of client list semaphore */
#define CLIENT_LIST_SEM_NAME "CLSN"

#endif /* !SENSOR_SERVER_H */