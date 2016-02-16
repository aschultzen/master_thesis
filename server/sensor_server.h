#ifndef SENSOR_SERVER_H
#define SENSOR_SERVER_H

/* See sensor_server_client_notes.md for details */

// Mine
#include "net.h"
#include "session.h"

extern volatile sig_atomic_t done;
extern struct client_table_entry *client_list;

#endif /* !SENSOR_SERVER_H */