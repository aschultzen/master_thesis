#ifndef SENSOR_CLIENT_H
#define SENSOR_CLIENT_H

// Mine
#include "net.h"
#include "utils.h"
#include "protocol.h"
#include "nmea.h"
#include "utils.h"
#include "serial.h"

struct config {
    char serial_interface[100];
    int client_id;
};

#endif /* !SENSOR_CLIENT_H */