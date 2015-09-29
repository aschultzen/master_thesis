#ifndef SENSOR_SERVER_H
#define SENSOR_SERVER_H

/* See sensor_server_client_notes.md for details */

//Strict C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/wait.h>

// C++
#include <string>
#include <iostream>


#define BUFFER_SIZE 512
#define TIME_OUT 10

#define DISCONNECT "DISCONNECT"

#endif /* !SENSOR_SERVER_H */