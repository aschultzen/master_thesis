/**
 * @file session.h
 * @author Aril Schultzen
 * @date 13.04.2016
 * @brief File containing function prototypes and includes for session.c
 */

#ifndef SESSION_H
#define SESSION_H

#include "sensor_server_commons.h"
#include "analyzer.h"
#include "actions.h"
#include "sensor_server.h"

/** @brief Sets up and starts the session with the client
 * 
 * Initializes and prepares the session and calls respond().
 * 
 * @return Void
 */
void setup_session(int session_fd, struct client_table_entry *new_client);

#endif /* !SESSION_H */