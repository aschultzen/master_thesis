#ifndef SESSION_H
#define SESSION_H

#include "sensor_server.h"

void setup_session(int session_fd, struct client_table_entry *new_client);

#endif /* !SESSION_H */