#include "sensor_client.h"

/* CONFIG */
#define CONFIG_SERIAL_INTERFACE "serial_interface:"
#define CONFIG_CLIENT_ID "client_id:"
#define CONFIG_LOG_NAME "log_file_name:"
#define CONFIG_ENTRIES 3
#define CONFIG_FILE_PATH "client_config.ini"
#define DEFAULT_SERIAL_INTERFACE "/dev/ttyACM0"

struct config_map_entry conf_map[1];

static int identify(int session_fd, int id);
static int create_connection(struct sockaddr_in *serv_addr, int *session_fd, char *ip, int portno);
static void receive_nmea(int gps_serial, struct raw_nmea_container *nmea_c);
static int format_nmea(struct raw_nmea_container *nmea_c);
static void initialize_config(struct config_map_entry *conf_map, struct config *cfg);
static int start_client(int portno, char* ip);
static int usage(char *argv[]);


/* Identify the client for the server */
static int identify(int session_fd, int id)
{
    /* Converting from int to string */
    char id_str[5];
    bzero(id_str, 5);
    sprintf(id_str, " %d", id); //Notice the space in the second parameter.
    int read_status = 0;

    /* Declaring message string */
    char identify_message[sizeof(PROTOCOL_IDENTIFY) + sizeof(id_str) + 1];

    /* copying */
    memcpy(identify_message, PROTOCOL_IDENTIFY, sizeof(PROTOCOL_IDENTIFY));
    memcpy(&identify_message[8],id_str, sizeof(id_str));

    write(session_fd, identify_message, sizeof(identify_message));

    char buffer[100];
    while ( (read_status = read(session_fd, buffer, sizeof(buffer)-1)) > 0) {
        if(strstr((char*)buffer, PROTOCOL_OK ) == (buffer)) {
            /* ID not used. Accepting. */
            t_print("ID %d accepted by server.\n", id);
            return 0;
        } else {
            /* ID in use. Rejected. */
            t_print("ID %d rejected by server, already in use.\n", id);
            return -1;
        }
    }
    /* Something happened during read. read() returns -1 at error */
    return read_status;
}

/* Create connection to server */
static int create_connection(struct sockaddr_in *serv_addr, int *session_fd, char *ip, int portno)
{
    if((*session_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        t_print("Could not create socket\n");
        return -1;
    }

    memset(serv_addr, '0', sizeof(*serv_addr));

    serv_addr->sin_family = AF_INET;
    serv_addr->sin_port = htons(portno);

    if(inet_pton(AF_INET, ip, &(serv_addr->sin_addr))<=0) {
        t_print("inet_pton error occured!\n");
        return 1;
    }

    if( connect(*session_fd, (struct sockaddr *)serv_addr, sizeof(*serv_addr)) < 0) {
        return 1;
    }

    return 0;
}

/* Get chosen NMEA from GPS receiver */
static void receive_nmea(int gps_serial, struct raw_nmea_container *nmea_c)
{
    char buffer[200];
    int position = 0;
    memset(buffer, '\0',sizeof(buffer));

    bool rmc = false;
    bool gga = false;

    /* Get a load of THIS timebomb!! */
    while(1) {
        while(position < 100) {
            read(gps_serial, buffer+position, 1);
            if( buffer[position] == '\n' ) break;
            position++;
        }

        if(strstr(buffer, RMC ) != NULL) {
            memcpy(nmea_c->raw_rmc, buffer, position+1);
            nmea_c->raw_rmc[position + 2] = '\0';
            rmc = true;
        }

        if(strstr(buffer, GGA ) != NULL) {
            memcpy(nmea_c->raw_gga, buffer, position+1);
            nmea_c->raw_rmc[position + 2] = '\0';
            gga = true;
        }

        if(rmc && gga) {
            break;
        }
        position = 0;
    }
}

/* Send received NMEA data to server */
static int format_nmea(struct raw_nmea_container *nmea_c)
{
    int nmea_prefix_length = 6;
    memcpy(nmea_c->output, "NMEA \n", nmea_prefix_length);
    int total_length = 0;
    int newline_length = 1;

    /* RMC */
    int rmc_length = strlen(nmea_c->raw_rmc);
    memcpy( nmea_c->output+nmea_prefix_length, nmea_c->raw_rmc, rmc_length );
    //nmea_c->output[nmea_prefix_length + rmc_length + newline_length] = '\n';

    /* Updating total length */
    total_length = rmc_length + nmea_prefix_length; //+ newline_length;

    /* GGA */
    int gga_length = strlen(nmea_c->raw_gga);
    memcpy( nmea_c->output+total_length, nmea_c->raw_gga, gga_length );
    nmea_c->output[total_length + gga_length + newline_length] = '\n';

    /* Updating total length */
    total_length += gga_length + newline_length;

    return total_length;
}

static int make_log(char *content, int id, char* log_name){
    /* Allocating memory for filename buffer */
    int filename_length = strlen(log_name) + 100;
    char filename[filename_length];

    /* Clearing buffer */
    memset(filename,'\0' ,filename_length);

    /* Copying name from loaded config */
    strcpy(filename, log_name);

    /* Casting int to string */
    char id_string[10];
    memset(id_string,'\0', 10);
    sprintf(id_string, "%d", id);

    /* Concating filename and ID */
    strcat(filename, id_string);

    return log_to_file(filename, content, 1);
}

/* Setting up the config structure specific for the server */
static void initialize_config(struct config_map_entry *conf_map, struct config *cfg)
{
    conf_map[0].entry_name = CONFIG_SERIAL_INTERFACE;
    conf_map[0].modifier = FORMAT_STRING;
    conf_map[0].destination = &cfg->serial_interface;

    conf_map[1].entry_name = CONFIG_CLIENT_ID;
    conf_map[1].modifier = FORMAT_INT;
    conf_map[1].destination = &cfg->client_id;

    conf_map[2].entry_name = CONFIG_LOG_NAME;
    conf_map[2].modifier = FORMAT_STRING;
    conf_map[2].destination = &cfg->log_name;
}

static int start_client(int portno, char* ip)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);

    struct sockaddr_in serv_addr;
    int session_fd = 0;
    int connection_attempts = 1;
    int con_status;

    struct raw_nmea_container nmea_c;
    memset(&nmea_c, 0, sizeof(nmea_c));

    struct config cfg;

    initialize_config(conf_map, &cfg);
    int load_config_status = load_config(conf_map, CONFIG_FILE_PATH, CONFIG_ENTRIES);
    if(!load_config_status) {
        t_print("Failed to load the config, using default values\n");
        memcpy(cfg.serial_interface, DEFAULT_SERIAL_INTERFACE, strlen(DEFAULT_SERIAL_INTERFACE)*sizeof(char));
    } else {
        if(cfg.client_id == 0 || cfg.client_id > ID_MAX) {
            t_print("Client ID can not be less than 1 or more than %d!\n", ID_MAX);
            exit(0);
        }
    }

    /* Establishing connection to GPS receiver */
    int gps_serial = open_serial(cfg.serial_interface, GPS);
    if(gps_serial == -1) {
        t_print("Connection to GPS receiver failed! Exiting...\n");
        exit(0);
    } else {
        t_print("Connection to GPS receiver established!\n");
    }

    /* Establishing connection to server */
    while(connection_attempts <= CONNECTION_ATTEMPTS_MAX) {
        con_status = create_connection(&serv_addr, &session_fd, ip, portno);
        if(con_status == 0) {
            t_print("Connected to server!\n");
            break;
        }
        t_print("Connection attempt %d failed. Code %d\n", connection_attempts, con_status);
        sleep(1);
        connection_attempts++;
    }

    /* Identifying client for server */
    if( identify(session_fd, cfg.client_id) == -1 ) {
        exit(0);
    }

    while (1) {
        receive_nmea(gps_serial, &nmea_c);
        int trans_length = format_nmea(&nmea_c);
         /* Writing to socket (server) */
        write(session_fd, nmea_c.output, trans_length);
        make_log(nmea_c.output, cfg.client_id, cfg.log_name);
    }
    return 0;
}

static int usage(char *argv[])
{
    t_print("Usage: %s -s <SERVER IP> -p <SERVER PORT>\n", argv[0]);
    return 0;
}

int main(int argc, char *argv[])
{
    char *ip_address = NULL;
    char *port_number = NULL;

    if(argc < 5) {
        usage(argv);
        return 0;
    }

    while (1) {
        char c;

        c = getopt (argc, argv, "s:p:");
        if (c == -1) {
            break;
        }
        switch (c) {
        case 's':
            ip_address = optarg;
            break;
        case 'p':
            port_number = optarg;
            break;
        default:
            usage(argv);
        }
    }

    if(ip_address == NULL || port_number == NULL) {
        t_print("Missing parameters!\n");
        exit(0);
    }

    start_client(atoi(port_number), ip_address);
    return 0;
}
