#ifndef SENSOR_CLIENT_H
#define SENSOR_CLIENT_H

// Mine
#include "net.h"
#include "utils.h"
#include "protocol.h"
#include "nmea.h"
#include "utils.h"
#include "serial.h"

/* CONFIG */
#define CONFIG_SERIAL_INTERFACE "serial_interface:"
#define CONFIG_CLIENT_ID "client_id:"
#define CONFIG_ENTRIES 2
#define CONFIG_FILE_PATH "client_config.ini"
#define DEFAULT_SERIAL_INTERFACE "/dev/ttyACM0"
struct config_map_entry conf_map[1];

struct config {
	char serial_interface[100];
	int client_id;
} __attribute__ ((packed));

struct config cfg;


#endif /* !SENSOR_CLIENT_H */