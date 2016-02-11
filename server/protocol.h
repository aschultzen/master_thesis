#ifndef PROTOCOL_H
#define PROTOCOL_H

/* CONSTRAINS */
#define MAX_COMMAND_SIZE 20
#define MAX_PARAMETER_SIZE 2048 //2 KiB for good measure 	

#define MIN_COMMAND_SIZE 3
#define MIN_PARAMETER_SIZE 0

/* COMMANDS TO USE WHEN COMMUNICATING*/
#define PROTOCOL_DISCONNECT "DISCONNECT"
#define PROTOCOL_GET_TIME "GET_TIME"
#define PROTOCOL_IDENTIFY "IDENTIFY"
#define PROTOCOL_STORE "STORE"
#define PROTOCOL_OK "OK\n"
#define PROTOCOL_NMEA "NMEA"
#define PROTOCOL_LISTCLIENTS "LISTCLIENTS"

/* COMMAND CODES */
/* Used by respond() */
#define CODE_DISCONNECT 1
#define CODE_GET_TIME 2
#define CODE_IDENTIFY 3
#define CODE_STORE 4
#define CODE_NMEA 5
#define CODE_LISTCLIENTS 6

/* ERRORS*/
#define ERROR_ILLEGAL_COMMAND "ILLEGAL COMMAND\n"
#define ERROR_NO_ID "CLIENT NOT IDENTIFIED\n"
#define ERROR_ID_IN_USE "ID IN USE\n"
#define ERROR_ILLEGAL_MESSAGE_SIZE "ILLEGAL MESSAGE SIZE\n"
#define ERROR_MAX_CLIENTS_REACHED "CONNECTION REJECTED: MAXIMUM NUMBER OF CLIENTS REACHED\n"

/* NMEA SENTENCES */
#define GGA "$GPGGA"
#define GSA "$GPGSA"
#define SENTENCE_LENGTH 100

/* 
* NMEA Struct
* This might be misplaced!
* 
*/
struct nmea_container{
	char gga[SENTENCE_LENGTH];
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