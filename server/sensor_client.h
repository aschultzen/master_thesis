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
    char log_name[100];
    int log_nmea;
};

/* Used by the client */
struct raw_nmea_container {
    /* Raw data */
    char raw_gga[SENTENCE_LENGTH];
    char raw_rmc[SENTENCE_LENGTH];
    char output[SENTENCE_LENGTH * 2];
};

#endif /* !SENSOR_CLIENT_H */
