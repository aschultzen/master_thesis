#ifndef SESSION_H
#define SESSION_H

#include "sensor_server_commons.h"
#include "analyzer.h"

#define HORIZONTAL_BAR "=============================================================\n"
#define CLIENT_TABLE_LABEL "CLIENT TABLE\n"
#define SERVER_TABLE_LABEL "SERVER DATA\n"
#define NEW_LINE "\n"

void setup_session(int session_fd, struct client_table_entry *new_client);

#endif /* !SESSION_H */