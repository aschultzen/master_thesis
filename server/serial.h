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

void open_serial(char *portname, char *connections);

#define SERIAL_SLEEP 200000	/* Sleep duration between checks on */
							/* shared variables */

#endif /* !SERIAL_H */
