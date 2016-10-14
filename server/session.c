#include "session.h"

#define CLIENT_TIMEOUT 5
#define MONITOR_TIMEOUT 1000
#define UNIDENTIFIED_TIMEOUT 100

/* ERRORS*/
#define ERROR_ILLEGAL_COMMAND "ERROR:Illegal command\n"
#define ERROR_NO_ID "ERROR:Client not identified\n"
#define ERROR_ID_IN_USE "ERROR:ID in use\n"
#define ERROR_ILLEGAL_MESSAGE_SIZE "\rERROR:Illegal message size\n"
#define ERROR_WARMUP_NOT_SENSOR "ERROR:Warm-up only applies to sensors\n"
#define ERROR_DUMPDATA_FAILED "ERROR:Failed to dump data\n"
#define ERROR_LOADDATA_FAILED "ERROR:Failed to load data\n"
#define ERROR_NO_COMMAND  "ERROR:No command specified\n"
#define ERROR_KRLD_LOAD_FAILED "ERROR:Failed to load KRL data from file\n"
#define ERROR_CHECKSUM_FAILED "ERROR: Checksum failed!\n"
#define ERROR_ILLEGAL_NMEA "ERROR: Received illegal/corrupt NMEA data\n"

static int nmea_ready();
static int extract_nmea_data(struct client_table_entry *cte);
static void calculate_nmea_average(struct client_table_entry *cte);
static void calculate_nmea_diff(struct client_table_entry *cte);
static int set_timeout(struct client_table_entry *target,
                       struct timeval h_timeout);
static int parse_input(struct client_table_entry *cte);
static int respond(struct client_table_entry *cte);

/*
* Used by spawned client processes to "mark" that their NMEA
* data is ready for processing. Works as a barrier in a way.
*/
static int nmea_ready()
{
    struct client_table_entry* c_iter;
    struct client_table_entry* temp;
    int ready = 0;

    /* iterating through the list of clients */
    list_for_each_entry_safe(c_iter, temp, &client_list->list, list) {
        /* Is it a SENSOR?*/
        if(c_iter->client_type == SENSOR) {
            /* Is it ready?*/
            if(c_iter->ready){
                ready++;
            }
        }
    }
    /* if everyone is ready, clear ready flag and carry on */
    if(ready == s_data->number_of_sensors) {
        list_for_each_entry_safe(c_iter, temp, &client_list->list, list) {
            c_iter->ready = 0;
        }
        return 1;
    } else {
        return 0;
    }
}

/* Extract position data from NMEA */
static int extract_nmea_data(struct client_table_entry *cte)
{
    int buffsize = 100;
    char buffer[buffsize];
    memset(&buffer, 0, buffsize);

    /* Extracting latitude */
    if(substring_extractor(LATITUDE_START,LATITUDE_START + 1,',',buffer, buffsize,
                        cte->nmea.raw_rmc, strlen(cte->nmea.raw_rmc)))
    {
        cte->nmea.lat_current = atof(buffer);       
    } else {
        return 0;
    }


    /* Extracting longitude */
    if(substring_extractor(LONGITUDE_START,LONGITUDE_START + 1,',',buffer, buffsize,
                        cte->nmea.raw_rmc, strlen(cte->nmea.raw_rmc)))
    {
        cte->nmea.lon_current = atof(buffer);
    } else {
        return 0;
    }

    /* Extracting altitude */
    if(substring_extractor(ALTITUDE_START,ALTITUDE_START + 1,',',buffer, buffsize,
                        cte->nmea.raw_gga, strlen(cte->nmea.raw_gga)))
    {    
        cte->nmea.alt_current = atof(buffer);
    } else {
        return 0;
    }

    /* Extracting speed */
    if(substring_extractor(SPEED_START,SPEED_START + 1,',',buffer, buffsize,
                        cte->nmea.raw_rmc, strlen(cte->nmea.raw_rmc)))
    {
        cte->nmea.speed_current = atof(buffer);
    } else {
        return 0;
    }

    return 1;
}

/* Calculate the average NMEA values */
static void calculate_nmea_average(struct client_table_entry *cte)
{
    /* Updating number of samples */
    cte->nmea.n_samples++;

    /* Updating total */
    cte->nmea.lat_total = cte->nmea.lat_total + cte->nmea.lat_current;
    cte->nmea.lon_total = cte->nmea.lon_total + cte->nmea.lon_current;
    cte->nmea.alt_total = cte->nmea.alt_total + cte->nmea.alt_current;
    cte->nmea.speed_total = cte->nmea.speed_total + cte->nmea.speed_current;

    cte->nmea.lat_average = ( cte->nmea.lat_total / cte->nmea.n_samples );
    cte->nmea.lon_average = ( cte->nmea.lon_total / cte->nmea.n_samples );
    cte->nmea.alt_average = ( cte->nmea.alt_total / cte->nmea.n_samples );
    cte->nmea.speed_average = ( cte->nmea.speed_total / cte->nmea.n_samples );
}

/*
* Calculate the diff between current
* NMEA values and the average values.
*/
static void calculate_nmea_diff(struct client_table_entry *cte)
{
    cte->nmea.lat_avg_diff = (cte->nmea.lat_current - cte->nmea.lat_average);
    cte->nmea.lon_avg_diff = (cte->nmea.lon_current - cte->nmea.lon_average);
    cte->nmea.alt_avg_diff = (cte->nmea.alt_current - cte->nmea.alt_average);
    cte->nmea.speed_avg_diff = (cte->nmea.speed_current - cte->nmea.speed_average);
}

static int set_timeout(struct client_table_entry *target,
                       struct timeval h_timeout)
{
    /* setsockopt return -1 on error and 0 on success */
    target->heartbeat_timeout = h_timeout;
    if (setsockopt (target->transmission.session_fd, SOL_SOCKET,
                    SO_RCVTIMEO, (char *)&target->heartbeat_timeout, sizeof(struct timeval)) < 0) {
        t_print("an error: %s\n", strerror(errno));
        return 0;
    }
    return 1;
}

/*
* Parses input from clients. Return value indicates status.
* Uses the command_code struct to convey parameter and command code.
*
* Returns -1 if size is wrong
* Returns 0 if protocol is not followed
* Returns 1 if all is ok
*/

static int parse_input(struct client_table_entry *cte)
{
    char *incoming = cte->transmission.iobuffer;

    /* INPUT TO BIG */
    if(strlen(incoming) > (MAX_PARAMETER_SIZE + MAX_COMMAND_SIZE) + 2) {
        return -1;
    }

    /* INPUT TO SMALL */
    if(strlen(incoming) < (MIN_PARAMETER_SIZE + MIN_COMMAND_SIZE) + 2) {
        return -1;
    }

    /* ZEROING COMMAND CODE */
    cte->cm.code = 0;
    /* ZEROING ID_PARAMETER */
    cte->cm.id_parameter = 0;

    /* NMEA */
    if(strstr((char*)incoming, PROTOCOL_NMEA ) == (incoming)) {
        cte->cm.code = CODE_NMEA;
    }

    /* IDENTIFY */
    else if(strstr((char*)incoming, PROTOCOL_IDENTIFY ) == (incoming)) {
        int length = (strlen(incoming) - strlen(PROTOCOL_IDENTIFY) );
        memcpy(cte->cm.parameter, (incoming)+(strlen(PROTOCOL_IDENTIFY)*(sizeof(char))),
               length);
        cte->cm.code = CODE_IDENTIFY;
    }

    /* IDENTIFY SHORT */
    else if(strstr((char*)incoming, PROTOCOL_IDENTIFY_SHORT ) == (incoming)) {
        int length = (strlen(incoming) - strlen(PROTOCOL_IDENTIFY_SHORT) );
        memcpy(cte->cm.parameter,
               (incoming)+(strlen(PROTOCOL_IDENTIFY_SHORT)*(sizeof(char))), length);
        cte->cm.code = CODE_IDENTIFY;
    }

    /* DUMPDATA */
    else if(strstr((char*)incoming, PROTOCOL_DUMPDATA ) == (incoming)) {
        int length = (strlen(incoming) - strlen(PROTOCOL_DUMPDATA) );
        memcpy(cte->cm.parameter, (incoming)+(strlen(PROTOCOL_DUMPDATA)*(sizeof(char))),
               length);
        cte->cm.code = CODE_DUMPDATA;
    }

    /* DUMPDATA_SHORT */
    else if(strstr((char*)incoming, PROTOCOL_DUMPDATA_SHORT ) == (incoming)) {
        int length = (strlen(incoming) - strlen(PROTOCOL_DUMPDATA_SHORT) );
        memcpy(cte->cm.parameter,
               (incoming)+(strlen(PROTOCOL_DUMPDATA_SHORT)*(sizeof(char))), length);
        cte->cm.code = CODE_DUMPDATA;
    }

    /* PRINT_LOCATION */
    else if(strstr((char*)incoming, PROTOCOL_PRINT_LOCATION ) == (incoming)) {
        int length = (strlen(incoming) - strlen(PROTOCOL_PRINT_LOCATION) );
        memcpy(cte->cm.parameter,
               (incoming)+(strlen(PROTOCOL_PRINT_LOCATION)*(sizeof(char))), length);
        cte->cm.code = CODE_PRINT_LOCATION;
    }

    /* PRINT_LOCATION_SHORT */
    else if(strstr((char*)incoming, PROTOCOL_PRINT_LOCATION_SHORT ) == (incoming)) {
        int length = (strlen(incoming) - strlen(PROTOCOL_PRINT_LOCATION_SHORT) );
        memcpy(cte->cm.parameter,
               (incoming)+(strlen(PROTOCOL_PRINT_LOCATION_SHORT)*(sizeof(char))), length);
        cte->cm.code = CODE_PRINT_LOCATION;
    }

    /* PRINTTIME */
    else if(strstr((char*)incoming, PROTOCOL_PRINTTIME ) == (incoming)) {
        int length = (strlen(incoming) - strlen(PROTOCOL_PRINTTIME) );
        memcpy(cte->cm.parameter,
               (incoming)+(strlen(PROTOCOL_PRINTTIME)*(sizeof(char))), length);
        cte->cm.code = CODE_PRINTTIME;
    }

    /* PRINTCLIENTS */
    else if(strstr((char*)incoming, PROTOCOL_PRINTCLIENTS ) == (incoming) ||
            strstr((char*)incoming, PROTOCOL_PRINTCLIENTS_SHORT ) == (incoming)) {
        cte->cm.code = CODE_PRINTCLIENTS;
    }

    /* PRINTSERVER */
    else if(strstr((char*)incoming, PROTOCOL_PRINTSERVER ) == (incoming) ||
            strstr((char*)incoming, PROTOCOL_PRINTSERVER_SHORT ) == (incoming)) {
        cte->cm.code = CODE_PRINTSERVER;
    }

    /* KICK */
    else if(strstr((char*)incoming, PROTOCOL_KICK ) == (incoming)) {
        int length = (strlen(incoming) - strlen(PROTOCOL_KICK) );
        memcpy(cte->cm.parameter, (incoming)+(strlen(PROTOCOL_KICK)*(sizeof(char))),
               length);
        cte->cm.code = CODE_KICK;
    }

    /* EXIT */
    else if(strstr((char*)incoming, PROTOCOL_EXIT ) == (incoming)) {
        cte->cm.code = CODE_DISCONNECT;
    }

    /* DISCONNECT */
    else if(strstr((char*)incoming, PROTOCOL_DISCONNECT ) == (incoming) ||
            strstr((char*)incoming, PROTOCOL_DISCONNECT_SHORT ) == (incoming)) {
        cte->cm.code = CODE_DISCONNECT;
    }

    /* HELP */
    else if(strstr((char*)incoming, PROTOCOL_HELP ) == (incoming) ||
            strstr((char*)incoming, PROTOCOL_HELP_SHORT ) == (incoming)) {
        cte->cm.code = CODE_HELP;
    }

    /* PRINTAVGDIFF */
    else if(strstr((char*)incoming, PROTOCOL_PRINTAVGDIFF ) == (incoming) ||
            strstr((char*)incoming, PROTOCOL_PRINTAVGDIFF_SHORT ) == (incoming)) {
        cte->cm.code = CODE_PRINTAVGDIFF;
    }

    /* LISTDUMPS */
    else if(strstr((char*)incoming, PROTOCOL_LISTDUMPS ) == (incoming) ||
            strstr((char*)incoming, PROTOCOL_LISTDUMPS_SHORT ) == (incoming)) {
        cte->cm.code = CODE_LISTDUMPS;
    }

    /* LOADDATA */
    else if(strstr((char*)incoming, PROTOCOL_LOADDATA ) == (incoming)) {
        int length = (strlen(incoming) - strlen(PROTOCOL_LOADDATA) );
        memcpy(cte->cm.parameter, (incoming)+(strlen(PROTOCOL_LOADDATA)*(sizeof(char))),
               length);
        cte->cm.code = CODE_LOADDATA;
    }

    /* LOADDATA_SHORT */
    else if(strstr((char*)incoming, PROTOCOL_LOADDATA_SHORT ) == (incoming)) {
        int length = (strlen(incoming) - strlen(PROTOCOL_LOADDATA_SHORT) );
        memcpy(cte->cm.parameter,
               (incoming)+(strlen(PROTOCOL_LOADDATA_SHORT)*(sizeof(char))), length);
        cte->cm.code = CODE_LOADDATA;
    }

    /* QUERYCSAC */
    else if(strstr((char*)incoming, PROTOCOL_QUERYCSAC ) == (incoming)) {
        int length = (strlen(incoming) - strlen(PROTOCOL_QUERYCSAC) );
        memcpy(cte->cm.parameter,
               (incoming)+(strlen(PROTOCOL_QUERYCSAC)*(sizeof(char))), length);
        cte->cm.code = CODE_QUERYCSAC;
    }

    /* QUERYCSAC_SHORT */
    else if(strstr((char*)incoming, PROTOCOL_QUERYCSAC_SHORT ) == (incoming)) {
        int length = (strlen(incoming) - strlen(PROTOCOL_QUERYCSAC_SHORT) );
        memcpy(cte->cm.parameter,
               (incoming)+(strlen(PROTOCOL_QUERYCSAC_SHORT)*(sizeof(char))), length);
        cte->cm.code = CODE_QUERYCSAC;
    }

    /* PRINT_LOADKRLDATA */
    else if(strstr((char*)incoming, PROTOCOL_LOADKRLDATA ) == (incoming)) {
        int length = (strlen(incoming) - strlen(PROTOCOL_LOADKRLDATA) );
        memcpy(cte->cm.parameter,
               (incoming)+(strlen(PROTOCOL_LOADKRLDATA)*(sizeof(char))), length);
        cte->cm.code = CODE_LOADKRLDATA;
    }

    /* PRINT_LOADKRLDATA_SHORT */
    else if(strstr((char*)incoming, PROTOCOL_LOADKRLDATA_SHORT ) == (incoming)) {
        int length = (strlen(incoming) - strlen(PROTOCOL_LOADKRLDATA_SHORT) );
        memcpy(cte->cm.parameter,
               (incoming)+(strlen(PROTOCOL_LOADKRLDATA_SHORT)*(sizeof(char))), length);
        cte->cm.code = CODE_LOADKRLDATA;
    }

    /* PROTOCOL_PRINTCFD */
    else if(strstr((char*)incoming, PROTOCOL_PRINTCFD ) == (incoming)) {
        int length = (strlen(incoming) - strlen(PROTOCOL_PRINTCFD) );
        memcpy(cte->cm.parameter, (incoming)+(strlen(PROTOCOL_PRINTCFD)*(sizeof(char))),
               length);
        cte->cm.code = CODE_PRINTCFD;
        printf("PRINTCFD\n");
    }

    /* PROTOCOL_PRINTCFD_SHORT */
    else if(strstr((char*)incoming, PROTOCOL_PRINTCFD_SHORT ) == (incoming)) {
        int length = (strlen(incoming) - strlen(PROTOCOL_PRINTCFD_SHORT) );
        memcpy(cte->cm.parameter,
               (incoming)+(strlen(PROTOCOL_PRINTCFD_SHORT)*(sizeof(char))), length);
        cte->cm.code = CODE_PRINTCFD;
    }

    else {
        return 0;
    }

    /* Attempting to retrive ID */
    sscanf(cte->cm.parameter, "%d", &cte->cm.id_parameter);

    return 1;
}

/* Responds to client action */
static int respond(struct client_table_entry *cte)
{
    bzero(cte->cm.parameter, MAX_PARAMETER_SIZE);
    /* Only print ">" if client is monitor */
    if(cte->client_id < 0) {
        s_write(&(cte->transmission), ">", 1);
    }

    int read_status = s_read(&(cte->transmission)); /* Blocking */
    if(read_status == -1) {
        t_print("[ CLIENT %d ] Read failed or interrupted!\n", cte->client_id);
        return 0;
    }

    if(cte->marked_for_kick) {
        return 0;
    }

    int parse_status = parse_input(cte);

    if(parse_status == -1) {
        s_write(&(cte->transmission), ERROR_ILLEGAL_MESSAGE_SIZE,
                sizeof(ERROR_ILLEGAL_MESSAGE_SIZE));
    } else if(parse_status == 0) {
        s_write(&(cte->transmission), ERROR_ILLEGAL_COMMAND,
                sizeof(ERROR_ILLEGAL_COMMAND));
    }
    /* PARSING OK, CONTINUING */
    else {
        /* Comparing CODES to determine the correct action */
        if(cte->cm.code == CODE_DISCONNECT) {
            t_print("Client %d requested DISCONNECT.\n", cte->client_id);
            s_write(&(cte->transmission), PROTOCOL_GOODBYE, sizeof(PROTOCOL_GOODBYE));
            return 0;
        }

        else if(cte->cm.code == CODE_HELP) {
            print_help(cte);
        }

        else if(cte->cm.code == CODE_IDENTIFY) {
            if(cte->cm.id_parameter == 0) {
                s_write(&(cte->transmission), ERROR_ILLEGAL_COMMAND,
                        sizeof(ERROR_ILLEGAL_COMMAND));
                return 0;
            }

            /* Checking to see if the ID is in use */
            struct client_table_entry* client_list_iterate;
            list_for_each_entry(client_list_iterate, &client_list->list, list) {
                if(client_list_iterate->client_id == cte->cm.id_parameter) {
                    cte->client_id = 0;
                    t_print("[%s] bounced! ID %d already in use.\n", cte->ip,cte->cm.id_parameter);
                    s_write(&(cte->transmission), "ID in use!\n", 11);
                    return 0;
                }
            }

            /* Determining role */
            if(cte->cm.id_parameter < 0) {
                cte->client_type = MONITOR;
                struct timeval timeout = {MONITOR_TIMEOUT, 0};
                set_timeout(cte, timeout);

            } else {
                cte->client_type = SENSOR;
                sem_wait(&(s_synch->client_list_mutex));
                s_data->number_of_sensors++;
                sem_post(&(s_synch->client_list_mutex));
            }
            /* Everything is good, setting id and responding*/
            s_write(&(cte->transmission), PROTOCOL_OK, sizeof(PROTOCOL_OK));
            cte->client_id = cte->cm.id_parameter;
            t_print("[%s] ID set to: %d\n", cte->ip,cte->client_id);

            if(cte->client_type == SENSOR) {
                if(load_krl_data(cte)) {
                    t_print("Loaded filter data for client %d\n", cte->client_id);
                } else {
                    s_write(&(cte->transmission),ERROR_KRLD_LOAD_FAILED,
                            sizeof(ERROR_KRLD_LOAD_FAILED));
                }
            }

            return 1;
        }

        /* Stop here if client is unidentified */
        else if(cte->client_id == 0) {
            s_write(&(cte->transmission), ERROR_NO_ID, sizeof(ERROR_NO_ID));
            return 1;
        }

        else if(cte->cm.code == CODE_NMEA) {
            /* Fetching data from buffer */
            char *rmc_start = strstr(cte->transmission.iobuffer, RMC);
            char *gga_start = strstr(cte->transmission.iobuffer, GGA);

            if(rmc_start == NULL || gga_start == NULL){
                s_write(&(cte->transmission), ERROR_ILLEGAL_NMEA, strlen(ERROR_ILLEGAL_NMEA));
                return 1;
            }

            memcpy(cte->nmea.raw_rmc, rmc_start, gga_start - rmc_start);
            memcpy(cte->nmea.raw_gga, gga_start,
                   ( strlen(cte->transmission.iobuffer) - (rmc_start - cte->transmission.iobuffer)
                     - (gga_start - rmc_start)));

            /* Checking NMEA checksum */
            int rmc_checksum = calculate_nmea_checksum(cte->nmea.raw_rmc);
            int gga_checksum = calculate_nmea_checksum(cte->nmea.raw_gga);

            /* Continue to filters if ok */
            if(rmc_checksum && gga_checksum) {
                s_write(&(cte->transmission), PROTOCOL_OK, strlen(PROTOCOL_OK));
                cte->timestamp = time(NULL);
                cte->nmea.checksum_passed = 1;

                if(!extract_nmea_data(cte)){
                    return 1;
                }

                calculate_nmea_average(cte);
                calculate_nmea_diff(cte);

                /* Checksums where OK, client marked ready */
                cte->ready = 1;

                /* Acquiring ready-lock */
                sem_wait(&(s_synch->ready_mutex));

                /* Checking if the other clients are ready as well*/
                int ready = nmea_ready();

                /* If everyone is ready, process data */
                if(ready) {
                    fprintf(stderr, "Client %d entered the ready loop!\n", cte->client_id);
                    /* Last process ready gets the job of analyzing the data */
                    krl_filter();

                    /* Check the results of the filters */
                    raise_alarm();
                }
                /* Releasing ready-lock */
                sem_post(&(s_synch->ready_mutex));
            } else {
                cte->nmea.checksum_passed = 0;
                t_print("RMC and GGA received from %d , checksum failed!\n", cte->client_id);
                s_write(&(cte->transmission), ERROR_CHECKSUM_FAILED, strlen(ERROR_CHECKSUM_FAILED));
            }
        }

        else if(cte->cm.code == CODE_PRINT_LOCATION) {
            struct client_table_entry* candidate = get_client_by_id(cte->cm.id_parameter);
            if(candidate == NULL) {
                s_write(&(cte->transmission), ERROR_NO_CLIENT, sizeof(ERROR_NO_CLIENT));
            } else {
                print_location(cte, candidate);
            }
        }

        else if(cte->cm.code == CODE_LOADKRLDATA) {
            struct client_table_entry* candidate = get_client_by_id(cte->cm.id_parameter);
            if(candidate == NULL) {
                s_write(&(cte->transmission), ERROR_NO_CLIENT, sizeof(ERROR_NO_CLIENT));
            } else {
                if(load_krl_data(candidate)) {
                    s_write(&(cte->transmission), PROTOCOL_OK, sizeof(PROTOCOL_OK));
                } else {
                    s_write(&(cte->transmission),ERROR_KRLD_LOAD_FAILED,
                            sizeof(ERROR_KRLD_LOAD_FAILED));
                }
            }
        }

        else if(cte->cm.code == CODE_PRINTCLIENTS) {
            print_clients(cte);
        }

        else if(cte->cm.code == CODE_PRINTSERVER) {
            print_server_data(cte);
        }

        else if(cte->cm.code == CODE_PRINTTIME) {
            struct client_table_entry* candidate = get_client_by_id(cte->cm.id_parameter);
            if(candidate != NULL) {
                print_client_time(cte, candidate);
            } else {
                s_write(&(cte->transmission), ERROR_NO_CLIENT, sizeof(ERROR_NO_CLIENT));
            }
        }

        else if(cte->cm.code == CODE_KICK) {
            struct client_table_entry* candidate = get_client_by_id(cte->cm.id_parameter);
            if(candidate == NULL) {
                s_write(&(cte->transmission), ERROR_NO_CLIENT, sizeof(ERROR_NO_CLIENT));
            } else {
                kick_client(candidate);
            }
        }

        else if(cte->cm.code == CODE_DUMPDATA) {
            int filename_buffer_size = MAX_FILENAME_SIZE;
            char filename[filename_buffer_size];
            int target_id;
            char id_buffer[ID_AS_STRING_MAX];
            bzero(id_buffer, ID_AS_STRING_MAX);
            bzero(filename, filename_buffer_size);

            /* Attempting to extract filename */
            substring_extractor(2,3, ' ', filename, filename_buffer_size,cte->cm.parameter,
                                MAX_FILENAME_SIZE);

            /* If length of filename = 0 (no filename specified).. */
            if(strlen(filename) == 0) {
                /* ...Cast to int without a care */
                target_id = atoi(cte->cm.parameter);
            }
            /* Else, extract ID */
            else {
                substring_extractor(1,2, ' ', id_buffer, ID_AS_STRING_MAX,cte->cm.parameter,
                                    ID_AS_STRING_MAX);
                target_id = atoi(id_buffer);
            }

            if(!target_id) {
                s_write(&(cte->transmission), ERROR_ILLEGAL_COMMAND,
                        sizeof(ERROR_ILLEGAL_COMMAND));
            } else {
                struct client_table_entry* candidate = get_client_by_id(target_id);
                if(candidate != NULL) {
                    if(!datadump(candidate,filename, s_conf->human_readable_dumpdata)) {
                        s_write(&(cte->transmission), ERROR_DUMPDATA_FAILED,
                                sizeof(ERROR_DUMPDATA_FAILED));
                    }
                } else {
                    s_write(&(cte->transmission), ERROR_NO_CLIENT, sizeof(ERROR_NO_CLIENT));
                }
            }
        }

        else if(cte->cm.code == CODE_LOADDATA) {
            int filename_buffer_size = MAX_FILENAME_SIZE;
            char filename[filename_buffer_size];
            int target_id;
            char id_buffer[ID_AS_STRING_MAX];
            bzero(id_buffer, ID_AS_STRING_MAX);
            bzero(filename, filename_buffer_size);

            substring_extractor(2,3, ' ', filename, filename_buffer_size,cte->cm.parameter,
                                MAX_FILENAME_SIZE);

            /* No filename specified, abort */
            if(strlen(filename) == 0) {
                s_write(&(cte->transmission), ERROR_NO_FILENAME, sizeof(ERROR_NO_FILENAME));
                return 1;
            }
            /* Extract target id and move on */
            else {
                substring_extractor(1,2, ' ', id_buffer, ID_AS_STRING_MAX,cte->cm.parameter,
                                    ID_AS_STRING_MAX);
                target_id = atoi(id_buffer);
            }

            if(!target_id) {
                s_write(&(cte->transmission), ERROR_ILLEGAL_COMMAND,
                        sizeof(ERROR_ILLEGAL_COMMAND));
            } else {
                struct client_table_entry* candidate = get_client_by_id(target_id);
                if(candidate != NULL) {
                    int load_status = loaddata(candidate,filename);
                    if(load_status == ERROR_CODE_NO_FILE) {
                        s_write(&(cte->transmission), ERROR_NO_FILE, sizeof(ERROR_NO_FILE));
                    } else if(load_status == ERROR_CODE_READ_FAILED) {
                        s_write(&(cte->transmission), ERROR_READ_FAILED, sizeof(ERROR_READ_FAILED));
                    }
                } else {
                    s_write(&(cte->transmission), ERROR_NO_CLIENT, sizeof(ERROR_NO_CLIENT));
                }
            }
        }

        else if(cte->cm.code == CODE_PRINTAVGDIFF) {
            print_avg_diff(cte);
        }

        else if(cte->cm.code == CODE_LISTDUMPS) {
            listdumps(cte);
        }

        else if(cte->cm.code == CODE_QUERYCSAC) {
            if(strlen(cte->cm.parameter) < 3) {
                s_write(&(cte->transmission), ERROR_NO_COMMAND, sizeof(ERROR_NO_COMMAND));
                return 1;
            }
            client_query_csac(cte, cte->cm.parameter);
        } else if(cte->cm.code == CODE_PRINTCFD) {
            print_cfd(cte, cte->cm.id_parameter);
        }

        else {
            t_print("No action made for this part of the protocol\n");
        }
    }
    return 1;
}

/* Setups the clients structure and initializes data */
void setup_session(int session_fd, struct client_table_entry *new_client)
{
    /* Setting the IP adress */
    char ip[INET_ADDRSTRLEN];
    get_ip_str(session_fd, ip);

    /* Setting the PID */
    new_client->pid = getpid();
    new_client->timestamp = time(NULL);
    strncpy(new_client->ip, ip, INET_ADDRSTRLEN);

    /* Initializing structure, zeroing just to be sure */
    new_client->client_id = 0;
    new_client->transmission.session_fd = session_fd;

    /* Zeroing out filters */
    new_client->fs.rdf.moved = 0;
    new_client->fs.rdf.was_moved = 0;

    new_client->marked_for_kick = 0;
    new_client->ready = 0;

    /* Setting timeout */
    struct timeval timeout = {UNIDENTIFIED_TIMEOUT, 0};
    if(!set_timeout(new_client, timeout)) {
        t_print("Failed to set timeout for client\n");
    }

    memset(&new_client->transmission.iobuffer, '0', IO_BUFFER_SIZE*sizeof(char));
    memset(&new_client->cm.parameter, '0', MAX_PARAMETER_SIZE*sizeof(char));

    /*
    * Entering child process main loop
    * (Outer) breaks if server closes.
    * (Inner) Breaks (disconnects the client) if
    * respond < 0
    */
    while(!done) {
        if(!respond(new_client)) {
            break;
        }
    }
}