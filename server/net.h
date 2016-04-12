#ifndef NET_H
#define NET_H

#define _GNU_SOURCE 1
#include <unistd.h>
#include <sys/mman.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdbool.h>

/* My own header files */ 
#include "utils.h"
#include "protocol.h"
#include "nmea.h"

/* GENERAL */
#define IO_BUFFER_SIZE MAX_PARAMETER_SIZE
#define CONNECTION_ATTEMPTS_MAX 10

struct transmission_s{
	int session_fd;
	char iobuffer[IO_BUFFER_SIZE]; 
} __attribute__ ((packed));

int s_read(struct transmission_s *tsm);
int s_write(struct transmission_s *tsm, char *message, int length);

#endif /* !NET_H */