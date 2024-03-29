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
#include "serial.h"
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
void print_client_time(struct client_table_entry *monitor,
                       struct client_table_entry* client);

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
void print_location(struct client_table_entry *monitor,
                    struct client_table_entry* client);

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

/** @brief Dumps NMEA data to file for given client
 *
 * @param client Pointer to client whose data should be dumped.
 * @param filename Pointer to filename.
 * @param human_readable Switch to determine if humanly readable data should be made as well.
 * @return 1 if success, 0 if fail.
 */
int datadump(struct client_table_entry* client, char *filename,
             int human_readable);

/** @brief List dump files in folder
 *
 * @param monitor Pointer to requesting monitor
 * @return 1 if success, 0 if fail.
 */
int listdumps(struct client_table_entry* monitor);

/** @brief Loads NMEA data into the NMEA struct of a given client (target).
*
* @param target Pointer to the client whose NMEA data should be loaded
* from file.
* @param filename Pointer to the filename of the data file.
*/
int loaddata(struct client_table_entry* target,  char *filename);

/** @brief Uses the query_csac.py to communicate with the CSAC.
*		   Stores the response in a buffer.
*
* @param buffer Buffer to store the response
* @param query Command (query) to send to the CSAC.
*/
int query_csac(char *query, char *buffer);

/** @brief Uses the query_csac.py to communicate with the CSAC
*		   Prints the response from the CSAC back to the client
*
* @param monitor Monitor who made the request
* @param query Command (query) to send to the CSAC.
*/
int client_query_csac(struct client_table_entry *monitor, char *query);

/** @brief Loads data for the REF_DEV_FILTER into the client.
*
* @param target Client to load the data into
*/
int load_lsf_data(struct client_table_entry* target);

/** @brief Prints the current state of the CSAC filter.
*
* @param monitor Monitor to print the data to.
* @return Status of sprintf() used to build string.
*/
void print_cfd(struct client_table_entry *monitor, int update_count);

/** @brief Dumps the state of the CSAC filter to file.
*
* @param Path to desired file to use.
* @return 1 if successful, 0 else.
*/
int dump_cfd(char *path);
#endif /* !ACTIONS_H */