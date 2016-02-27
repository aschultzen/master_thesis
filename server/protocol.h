#ifndef PROTOCOL_H
#define PROTOCOL_H

/* CONSTRAINS */
#define MAX_COMMAND_SIZE 20
#define MAX_PARAMETER_SIZE 2048 //2 KiB for good measure 	

#define MIN_COMMAND_SIZE 3
#define MIN_PARAMETER_SIZE 0

/* COMMANDS TO USE WHEN COMMUNICATING*/
#define PROTOCOL_DISCONNECT "DISCONNECT"
#define PROTOCOL_EXIT "EXIT"
#define PROTOCOL_GET_TIME "GET_TIME"
#define PROTOCOL_IDENTIFY "IDENTIFY"
#define PROTOCOL_STORE "STORE"
#define PROTOCOL_OK "OK\n"
#define PROTOCOL_NMEA "NMEA"
#define PROTOCOL_PRINTCLIENTS "PRINTCLIENTS"
#define PROTOCOL_PRINTSERVER "PRINTSERVER"
#define PROTOCOL_GOODBYE "Goodbye!\n"

/* COMMAND CODES */
/* Used by respond() */
#define CODE_DISCONNECT 1
#define CODE_GET_TIME 2
#define CODE_IDENTIFY 3
#define CODE_STORE 4
#define CODE_NMEA 5
#define CODE_PRINTCLIENTS 6
#define CODE_PRINTSERVER 7

/* ERRORS*/
#define ERROR_ILLEGAL_COMMAND "ILLEGAL COMMAND\n"
#define ERROR_NO_ID "CLIENT NOT IDENTIFIED\n"
#define ERROR_ID_IN_USE "ID IN USE\n"
#define ERROR_ILLEGAL_MESSAGE_SIZE "ILLEGAL MESSAGE SIZE\n"
#define ERROR_MAX_CLIENTS_REACHED "CONNECTION REJECTED: MAXIMUM NUMBER OF CLIENTS REACHED\n"

/* NMEA SENTENCES */
#define GGA "$GNGGA"
#define GSA "$GNGSA"
#define RMC "$GNRMC"
#define SENTENCE_LENGTH 100

/* NMEA SENTENCES DELIMITER POSITIONS */
# define ALTITUDE_START 9
# define LATITUDE_START 3
# define LONGITUDE_START 5

/* 
* NMEA Struct
* This might be misplaced!
* 
* low -> Lowest recorded value
* high -> Highest recorded value
* current -> Current value
* The values are used to filter out false positives.
*/
struct nmea_container{
	/* Raw data */
	char raw_gga[SENTENCE_LENGTH];
	char raw_rmc[SENTENCE_LENGTH];

	/* Latitude */
	double lat_low;
	double lat_high;
	double lat_current;

	/* Longitude */
	double lon_low;
	double lon_high;
	double lon_current;

	/* Altitude */
	double alt_low;
	double alt_high;
	double alt_current;
};

/* 
* Roles of client, either SENSOR or MONITOR. 
* A monitor is only used to monitor the programs state.
*/
enum client_type{
    SENSOR,
    MONITOR
};

/*
* command_code struct is used by the parser
* to convey an easy to compare command code, as well
* as any parameter belonging to that command
*/ 
struct command_code{
	int code;
	char parameter[MAX_PARAMETER_SIZE];
} __attribute__ ((packed));

#endif /* !PROTOCOL_H */