/**
 * @file sensor_server_commons.h
 * @author Aril Schultzen
 * @date 13.04.2016
 * @brief File containing structs and defines used by session.c, analyzer.c, sensors_server.c and actions.c
 */

#ifndef SENSOR_SERVER_COMMONS_H
#define SENSOR_SERVER_COMMONS_H

#include <semaphore.h>
#include "net.h"
#include "colors.h"

/* General */
#define SERVER_TABLE_LABEL "SERVER DATA\n"
#define HORIZONTAL_BAR "================================================================================\n"
#define ERROR_NO_CLIENT "ERROR: No such client\n"
#define ERROR_NO_FILENAME "ERROR: No FILENAME specified\n"
#define MAX_FILENAME_SIZE 30
#define ID_AS_STRING_MAX 10

/* Errors */
#define ERROR_CODE_NO_FILE -1
#define ERROR_CODE_READ_FAILED -2
#define ERROR_NO_FILE "ERROR:No such file\n"
#define ERROR_READ_FAILED "ERROR:Failed to read file\n"

/*
* command_code struct is used by the parser
* to convey an easy to compare command code, as well
* as any parameter belonging to that command
*/
struct command_code {
    int code;
    char parameter[MAX_PARAMETER_SIZE];
    int id_parameter;
};

/*!@struct*/
/*!@brief Data used by the red_dev_filter.
* Read from file.
*/
struct ref_dev_data {
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
* CLIENT TABLE STRUCT
*
* list_head list: The head in the list of clients
* pid: Process ID for the client connection (See "fork")
* session_fd: The file descriptor for the session.
* client_id: The connected clients ID
* iobuffer: A general purpose buffer for in and output
* heartbeat_timeout: Number of seconds of inactivity before disconnect
* ip: Clients IP Address.
* cm: Command code. Used for quick comparison after commands
* are parsed by command parser.
*/

struct disturbed_values{
    int lat_disturbed;
    int lon_disturbed;
    int alt_disturbed;
    int speed_disturbed;
};

struct ref_dev{
    struct ref_dev_data rdd;
    int moved;
    int was_moved;
    struct disturbed_values dv;
};

struct min_max{
    int moved;
    int was_moved;
    int warmup;
    struct disturbed_values dv;
    time_t warmup_started; /** When warm-up of SENSOR started */
};

struct filters {
    struct min_max mmf;
    struct ref_dev rdf;
};

/*!@struct*/
/*!@brief Contain information about every client that is connected.
*/
struct client_table_entry {
    struct list_head list; /** The head in the list of clients */
    struct transmission_s transmission;
    struct timeval heartbeat_timeout;
    struct command_code cm;
    struct nmea_container nmea;
    pid_t pid; /** The process ID */
    time_t timestamp; /** When last analyzed */
    int client_id; /** Clients ID */
    char client_id_string[ID_AS_STRING_MAX];
    int client_type; /** Client type, SENSOR or MONITOR */
    //int warmup; /** Currently warming up status */
    //int moved; /** Moved status */
    int ready;	/** Ready status */
    //int was_moved;	/** Moved when last checked. Used to check if returned */
    int marked_for_kick; /** Marked for kicked at next opportunity */
    char ip[INET_ADDRSTRLEN]; /** Clients IP address */
    struct filters fs;
    //struct ref_dev_data rdd;
};

/* Server info shared with processes */
struct server_data {
    int number_of_clients;	/* Number of clients currently connected */
    int number_of_sensors;	/* Number of sensors, subset of clients */
    time_t started;			/* When the server was started */
    pid_t pid;
    char version[4];
    int csac_fd;
};

/* Synchronization elements shared with processes */
struct server_synchro {
    sem_t ready_mutex;
    sem_t client_list_mutex;
    volatile int ready_counter;
};

/*
* Roles of client, either SENSOR or MONITOR.
* A monitor is only used to monitor the programs state.
*/
enum client_type {
    SENSOR,
    MONITOR
};

#endif /* !SENSOR_SERVER_COMMONS_H */