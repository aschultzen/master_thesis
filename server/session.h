#ifndef SESSION_H
#define SESSION_H

#include "sensor_server_commons.h"
#include "analyzer.h"
#include "actions.h"
#include "sensor_server.h"

#define ID_MAX 1000	/* Highest ID number allowed */
#define ID_AS_STRING_MAX 4
#define CLIENT_TIMEOUT 5
#define MONITOR_TIMEOUT 1000

void setup_session(int session_fd, struct client_table_entry *new_client);

#endif /* !SESSION_H */