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
#define PROTOCOL_GET_TIME "GETTIME"
#define PROTOCOL_IDENTIFY "IDENTIFY"
#define PROTOCOL_STORE "STORE"
#define PROTOCOL_NMEA "NMEA"
#define PROTOCOL_PRINTCLIENTS "PRINTCLIENTS"
#define PROTOCOL_PRINTSERVER "PRINTSERVER"
#define PROTOCOL_KICK "KICK"
#define PROTOCOL_HELP "HELP"
#define PROTOCOL_PRINT_LOCATION "PRINTLOC"
#define PROTOCOL_WARMUP "WARMUP"
#define PROTOCOL_PRINTTIME "PRINTTIME"
#define PROTOCOL_DUMPLOC "DUMPLOC"

/* RESPONSES */
#define PROTOCOL_GOODBYE "Goodbye!\n"
#define PROTOCOL_OK "OK\n"

/* COMMAND CODES */
/* Used by respond() */
#define CODE_DISCONNECT 	1
#define CODE_GET_TIME 		2
#define CODE_IDENTIFY 		3
#define CODE_STORE 			4
#define CODE_NMEA 			5
#define CODE_PRINTCLIENTS 	6
#define CODE_PRINTSERVER 	7
#define CODE_KICK 			8
#define CODE_HELP 			9
#define CODE_PRINT_LOCATION 10
#define CODE_WARMUP 		11
#define CODE_PRINTTIME 		12
#define CODE_DUMPLOC 		13
#define CODE_MOVED 			14 

/* ERRORS*/
#define ERROR_ILLEGAL_COMMAND "ILLEGAL COMMAND\n"
#define ERROR_NO_ID "CLIENT NOT IDENTIFIED\n"
#define ERROR_ID_IN_USE "ID IN USE\n"
#define ERROR_ILLEGAL_MESSAGE_SIZE "ILLEGAL MESSAGE SIZE\n"

/* SIZES */
#define TIME_SIZE 9 /* SIZE OF TIME AS CHARS eg.142546.00, FROM GNRMC */

#endif /* !PROTOCOL_H */