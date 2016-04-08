#ifndef CONFIG_H
#define CONFIG_H

/* Config file entries mappings */
#define CONFIG_SERVER_MAX_CONNECTIONS "max_clients:"
#define CONFIG_SERVER_WARM_UP "warm_up:"
#define CONFIG_FILE_PATH "config.ini"

#define FORMAT_INT "%d"
#define FORMAT_FLOAT "%f"

struct config_map_entry {
	char *entry_name;
	char *modifier;
	void *destination;
} __attribute__ ((packed));

/* 
* CONFIG STRUCT
* 
* Used as container for the config_loader function
* config_server_max_connections: max number of permitted connections
*/

struct config {
	int max_clients;
	int warm_up_seconds;
} __attribute__ ((packed));

#endif /* !CONFIG_H */