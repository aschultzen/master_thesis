/**
 * @file session.h
 * @author Aril Schultzen
 * @date 13.04.2016
 * @brief File containing function prototypes and includes for session.c
 */

#ifndef SESSION_H
#define SESSION_H

#include "sensor_server_common.h"
#include "filters.h"
#include "actions.h"
#include "sensor_server.h"

int respond(struct client_table_entry *cte);

#endif /* !SESSION_H */