/*
## CSAC Config ##################
#
# 57600			
# 8 bit			
# No parity		
# 
# While CSAC is off:
#
# sudo stty -F /dev/ttyS0 57600
# cat /dev/ttyS0 
#
# Turn the CSAC ON
# 
# Symmetricom CSAC <- Output
#
#################################
*/

#ifndef SERIAL_H
#define SERIAL_H

#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <string.h> /* memset */
#include <stdio.h>
#include <stdlib.h>
#include <features.h>
#include <fcntl.h>
#include <signal.h>

//Mine
#include "utils.h"
#include "protocol.h"

typedef enum e_serial_device {
    GPS,
    CSAC
} serial_device;

int open_serial(char *portname, serial_device device);

/** @brief Queries the CSAC with the command over serial connection
*
* Sends a command to the CSAC and reads buf_len bytes into
* the buffer. Does not deal with formatting in any way.
*
* @param file_descriptor FD for the CSAC serial connection
* @param query Command (query) to send to the CSAC.
* @param buffer Buffer to store the response
* @buf_len buf_len Length of buffer
*/ 
int serial_query(int file_descriptor, char *query, char *buffer, int buf_len);

#endif /* !SERIAL_H */
