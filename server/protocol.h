#ifndef PROTOCOL_H
#define PROTOCOL_H

/* CONSTRAINS */
#define MAX_COMMAND_SIZE 20
#define MAX_PARAMETER_SIZE 100

#define MIN_COMMAND_SIZE 3
#define MIN_PARAMETER_SIZE 0

/* COMMANDS TO USE WHEN COMMUNICATING*/
#define PROTOCOL_DISCONNECT "DISCONNECT"
#define PROTOCOL_GET_TIME "GET_TIME"
#define PROTOCOL_IDENTIFY "IDENTIFY"
#define PROTOCOL_STORE "STORE"
#define PROTOCOL_OK "OK\n"

/* COMMAND CODES */
/* Used by respond() */
#define CODE_DISCONNECT 1
#define CODE_GET_TIME 2
#define CODE_IDENTIFY 3
#define CODE_STORE 4

/* ERRORS*/
#define ERROR_ILLEGAL_COMMAND "ILLEGAL COMMAND\n"
#define ERROR_NO_ID "CLIENT NOT IDENTIFIED\n"
#define ERROR_ID_IN_USE "ID IN USE\n"
#define ERROR_ILLEGAL_MESSAGE_SIZE "ILLEGAL MESSAGE SIZE\n"
#endif /* !PROTOCOL_H */