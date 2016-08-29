/**
 * @file sensor_server.h
 * @author Aril Schultzen
 * @date 13.04.2016
 * @brief File containing function prototypes, structs and includes for sensor_server.c
 */

#ifndef SENSOR_SERVER_H
#define SENSOR_SERVER_H

#include <fcntl.h>
#include <sys/stat.h>
#include "session.h"
#include "serial.h"
#include "sensor_server_commons.h"

/*!@struct*/
/*!@brief Contains configuration values for the server
*/
struct server_config {
    int max_clients;
    int warm_up_seconds;
    int human_readable_dumpdata;
    char csac_path[100];
    double alt_ref;
 	double lon_ref;
	double lat_ref;
	double speed_ref;
	double alt_dev;
	double lon_dev;
	double lat_dev;
	double speed_dev;
};

/*
* Made extern because the sessions should
* exit if the server is given a SIGINT/TERM
*/
extern volatile sig_atomic_t done;

/* Also used by session and action */
extern struct client_table_entry *client_list;
extern struct server_data *s_data;
extern struct server_synchro *s_synch;
extern struct server_config *s_conf;

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

#endif /* !SENSOR_SERVER_H */