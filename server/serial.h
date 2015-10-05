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

void open_serial(char *portname, char *connections);

#define SERIAL_SLEEP 200000

#endif /* !SERIAL_H */
