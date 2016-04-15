/**
 * @file actions.h
 * @brief File containing function prototypes and includes for actions.c
 *
 * Function prototypes for functions that implements different
 * actions that a MONITOR or the system can use to manipulate the
 * state of the SENSORS or print stats or similar.
 *
 * Be advised that any reference to MONITOR in this file means
 * a client connected to the server who's role is that of a
 * monitor of the system and not a monitor like a peripheral
 * connected to a computer. The names of these roles are under
 * discussion and will probably be changed to avoid misunderstanding.
 *
 * @author Aril Schultzen
 * @date 9.11.2015
 */

#ifndef ACTIONS_H
#define ACTIONS_H

#include "sensor_server.h"
#include <dirent.h>

/** @brief Kicks a client (both MONITOR or SENSOR)
 *
 * Marks the client so respond() in session.c can
 * disconnect it the next time that client transmits
 * data. The kick is in other words not instant, this
 * is however an easy way to gracefully disconnect a
 * client.
 *
 * @param client Pointer to the client_table_entry for the candidate to be kicked.
 * @return Void
 */
void kick_client(struct client_table_entry* client);

/** @brief Prints clients solved time to MONITOR
 *
 * Extracts the time solved by the GPS receiver, transmitted
 * via NMEA and stored in the client_table_struct at the server,
 * and transmits it to the MONITOR that requested it.
 *
 * @param monitor Pointer to MONITOR who made the request.
 * @param client Pointer to SENSOR whose time was requested.
 * @return Void
 */
void print_client_time(struct client_table_entry *monitor, struct client_table_entry* client);

/** @brief Prints a table of clients to the MONITOR
 *
 * Transmits a table of the connected clients to the MONITOR.
 *
 * @param monitor Pointer to MONITOR who made the request.
 * @return Void
 */
void print_clients(struct client_table_entry *monitor);

/** @brief Prints table of available commands to requesting MONITOR.
 *
 * @param monitor Pointer to MONITOR who made the request.
 * @return Void
 */
void print_help(struct client_table_entry *monitor);

/** @brief Prints location of SENSOR to requesting MONITOR.
 *
 * Prints a overview of current as well as MIN, MAX and AVERAGE
 * values of LAT, LON, ALT and SPEED recovered from NMEA.
 *
 * @param monitor Pointer to MONITOR who made the request.
 * @param client Pointer to SENSOR whose location is requested.
 * @return Void
 */
void print_location(struct client_table_entry *monitor, struct client_table_entry* client);

/** @brief Prints difference between current position and average.
 *
 * Prints the difference between the current position values
 * recorded from NMEA, and the calculated averages.
 * Two sensors in close proximity (100m >) should be
 * subjected to the same noise. If the difference between
 * sensor A (current-avg) and sensor B (current-avg) changes,
 * this could mean that one of them is being spoofed.
 *
 * @param monitor Pointer to MONITOR who made the request.
 * @return Void
 */
void print_avg_diff(struct client_table_entry *monitor);

/** @brief Restarts the warm-up procedure for the given SENSOR
 *
 * Sets the SENSORs warmup_started to NOW, warmup to 1 and ready to 0.
 * This "triggers" a restart of the warm-up procedure.
 *
 * @param client Pointer to client whose warm-up procedure to restart.
 * @return Void
 */
void restart_warmup(struct client_table_entry* client);

/** @brief Dumps NMEA data to file for given client
 *
 * @param client Pointer to client whose data should be dumped.
 * @param filename Pointer to filename.
 * @param human_readable Switch to determine if humanly readable data should be made as well.
 * @return 1 if success, 0 if fail.
 */
int datadump(struct client_table_entry* client, char *filename, int human_readable);

/** @brief Restore NMEA data from file
 *
 * @param client Pointer to client whose data should be restored from file
 * @param filename Pointer to filename.
 * @return 1 if success, 0 if fail.
 */
int datarestore(struct client_table_entry* client, char *filename);

/** @brief List files in folder
 *
 * @param monitor Pointer to requesting monitor
 * @return 1 if success, 0 if fail.
 */
int listdumps(struct client_table_entry* monitor);

/** @brief Sets a new warm-up time for a given SENSOR.
 *
 * @param client Pointer to client whose warm-up time to be given new value.
 * @param value New warm-up time in seconds.
 * @return Void
 */
void set_warmup(struct client_table_entry *client, int value);



#endif /* !ACTIONS_H */