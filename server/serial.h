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

#endif /* !SERIAL_H */
