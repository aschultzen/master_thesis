#ifndef CONFIG_H
#define CONFIG_H

/* Config file entries mappings */
#define CONFIG_SERVER_MAX_CONNECTIONS "max_clients:"
#define CONFIG_FILE_PATH "config.ini"

/* 
* CONFIG STRUCT
* 
* Used as container for the config_loader function
* config_server_max_connections: max number of permitted connections
*/

struct config {
	int config_server_max_connections;
} __attribute__ ((packed));

#endif /* !CONFIG_H */