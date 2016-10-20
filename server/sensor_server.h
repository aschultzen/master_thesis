/**
 * @file sensor_server.h
 * @author Aril Schultzen
 * @date 13.04.2016
 * @brief File containing function prototypes, structs and includes for sensor_server.c
 */

#ifndef SENSOR_SERVER_H
#define SENSOR_SERVER_H

#define PATH_LENGTH_MAX 1000
#define CLIENT_TIMEOUT 5
#define MONITOR_TIMEOUT 1000
#define UNIDENTIFIED_TIMEOUT 10

#include <fcntl.h>
#include <sys/stat.h>
#include "session.h"
#include "serial.h"
#include "sensor_server_common.h"
#include "csac_filter.h"

/*!@struct*/
/*!@brief Contains configuration values for the server
*/
struct server_config {
    int max_clients;
    int warm_up_seconds;
    int human_readable_dumpdata;
    char csac_path[PATH_LENGTH_MAX];
    int logging;
    char log_path[PATH_LENGTH_MAX];
    int csac_logging;
    char csac_log_path[PATH_LENGTH_MAX];
};

/*
* Made extern because the sessions should
* exit if the server is given a SIGINT/TERM
*/
//extern volatile sig_atomic_t done;

/* Also used by session and action */
extern struct client_table_entry *client_list;
extern struct server_data *s_data;
extern struct server_synchro *s_synch;
extern struct server_config *s_conf;
extern struct csac_filter_data *cfd;

/** @brief Removes a client whose ID matches parameter
 *
 * Iterates through the linked list and removes the
 * node containing the client whose ID matches the parameter.
 * @param id ID for the client
 * @return Void
 */
void remove_client_by_id(int id);

/** @brief Returns a client whose ID matches parameter
 *
 * Iterates through the linked list and returns
 * a pointer to the client_table_entry struct in the
 * list that corresponds with the parameter.
 * @param id ID for the client
 * @return client_table_entry *
 */
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

int set_timeout(struct client_table_entry *target,
                       struct timeval h_timeout);

#endif /* !SENSOR_SERVER_H */