#include "session.h"

static int nmea_ready()
{
    s_synch->ready_counter++;
    if(s_synch->ready_counter == s_data->number_of_clients)
    {
        /* Zeroing out the counter, we are ready */
        s_synch->ready_counter = 0;
        return 1;
    }
    else 
    {
        return 0;
    }
}

/* Extract position data from NMEA */
static void extract_pos(struct client_table_entry *cte)
{
    int buffsize = 100;
    char buffer[buffsize];

    /* Extracting latitude */
    word_extractor(LATITUDE_START,LATITUDE_START + 1,',',buffer, buffsize,cte->nmea.raw_rmc, strlen(cte->nmea.raw_rmc));
    cte->nmea.lat_current = atof(buffer);

    /* Extracting longitude */
    word_extractor(LONGITUDE_START,LONGITUDE_START + 1,',',buffer, buffsize,cte->nmea.raw_rmc, strlen(cte->nmea.raw_rmc));
    cte->nmea.lon_current = atof(buffer);

    /* Extracting altitude */
    word_extractor(ALTITUDE_START,ALTITUDE_START + 1,',',buffer, buffsize,cte->nmea.raw_gga, strlen(cte->nmea.raw_gga));
    cte->nmea.alt_current = atof(buffer);
}

static void check_warm_up(struct client_table_entry *cte)
{
    if(cte->warmup_started){
        double elapsed = difftime(time(NULL), cte->warmup_started);
        double percent = (elapsed / cfg->warm_up_seconds) * 100;

        if((int)percent % 10 == 0){
            t_print("Client %d Warming up, %d%%\n", cte->client_id, (int)percent);
        }

        if(elapsed >= cfg->warm_up_seconds){
            t_print("Client %d, warm-up finished!\n", cte->client_id);
            cte->warmup = 0;
        }
    }
    else
    {
        cte->warmup_started = time(NULL);
    }
}

/* Updating "extreme" values */
static void warm_up(struct client_table_entry *cte)
{
    /* Updating latitude */
    if(cte->nmea.lat_current > cte->nmea.lat_high){
        cte->nmea.lat_high = cte->nmea.lat_current;
    }

    if(cte->nmea.lat_current < cte->nmea.lat_low){
        cte->nmea.lat_low = cte->nmea.lat_current;
    }

    /* Updating longitude */
    if(cte->nmea.lon_current > cte->nmea.lon_high){
        cte->nmea.lon_high = cte->nmea.lon_current;
    }

    if(cte->nmea.lon_current < cte->nmea.lon_low){
        cte->nmea.lon_low = cte->nmea.lon_current;
    }

    /* Updating altitude */
    if(cte->nmea.alt_current > cte->nmea.alt_high){
        cte->nmea.alt_high = cte->nmea.alt_current;
    }

    if(cte->nmea.alt_current < cte->nmea.alt_low){
        cte->nmea.alt_low = cte->nmea.alt_current;
    }
}

/* Sends a formatted string containing info about connected clients */
static void print_clients(struct client_table_entry *cte)
{
    char buffer [1000];
    int snprintf_status = 0;
    char *c_type = "SENSOR";

    struct client_table_entry* client_list_iterate;
    s_write(&(cte->transmission), "\n", 1);
    s_write(&(cte->transmission), "CLIENT TABLE\n", 13);
    s_write(&(cte->transmission), "=============================================================\n", 63);
    list_for_each_entry(client_list_iterate,&client_list->list, list) {

        if(client_list_iterate->client_type == MONITOR) {
            c_type = "MONITOR";
        } else {
            c_type = "SENSOR";
        }

        snprintf_status = snprintf( buffer, 1000, "PID: %d, IP:%s, TOUCH: %d, TYPE: %s, ID: %d\n",
                                    client_list_iterate->pid,
                                    client_list_iterate->ip,
                                    (int)difftime(time(NULL),client_list_iterate->timestamp),
                                    c_type,
                                    client_list_iterate->client_id);
        s_write(&(cte->transmission), buffer, snprintf_status);
    }
    s_write(&(cte->transmission), "=============================================================\n", 63);
}

/* Sends a formatted string containing server info */
static void print_server_data(struct client_table_entry *cte, struct server_data *s_data)
{
    char buffer [1000];
    int snprintf_status = 0;
    struct tm *loctime;
    loctime = localtime (&s_data->started);

    s_write(&(cte->transmission), "\n", 1);
    s_write(&(cte->transmission), "SERVER DATA\n", 12);
    s_write(&(cte->transmission), "=============================================================\n", 63);

    snprintf_status = snprintf( buffer, 1000,
                                "PID: %d\nNumber of clients: %d\nMax clients: %d\nStarted: %sVersion: %s\n",
                                s_data->pid,
                                s_data->number_of_clients,
                                cfg->max_clients,
                                asctime (loctime),
                                s_data->version);

    s_write(&(cte->transmission), buffer, snprintf_status);
    s_write(&(cte->transmission), "=============================================================\n", 63);
}

/*
* Explanation:
* ------------
* Parses input from clients over IP network. Return value indicates status.
* Uses the command_code struct to convey parameter and command code. 
* The purpose of the parser was to make the server/client code less
* cluttered and to make future protocol implementations easier. 
* 
* Return values:
* ------------  
* Returns -1 if size is wrong
* Returns 0 if protocol is not followed
* Returns 1 if all is ok
*/

int parse_input(struct client_table_entry *cte)
{
    /* INPUT TO BIG */
    if(strlen(cte->transmission.iobuffer) > (MAX_PARAMETER_SIZE + MAX_COMMAND_SIZE) + 2) {
        return -1;
    }

    /* INPUT TO SMALL */
    if(strlen(cte->transmission.iobuffer) < (MIN_PARAMETER_SIZE + MIN_COMMAND_SIZE) + 2) {
        return -1;
    }

    /* ZEROING COMMAND CODE */
    cte->cm.code = 0;

    /* IDENTIFY */
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_IDENTIFY ) == (cte->transmission.iobuffer)) {
        int length = (strlen(cte->transmission.iobuffer) - strlen(PROTOCOL_IDENTIFY) );
        memcpy(cte->cm.parameter, (cte->transmission.iobuffer)+(strlen(PROTOCOL_IDENTIFY)*(sizeof(char))), length);
        cte->cm.code = CODE_IDENTIFY;
        return 1;
    }

    /* PRINTCLIENTS */
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_PRINTCLIENTS ) == (cte->transmission.iobuffer)) {
        cte->cm.code = CODE_PRINTCLIENTS;
        return 1;
    }

    /* PRINTSERVER */
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_PRINTSERVER ) == (cte->transmission.iobuffer)) {
        cte->cm.code = CODE_PRINTSERVER;
        return 1;
    }

    /* KICK */
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_KICK ) == (cte->transmission.iobuffer)) {
        int length = (strlen(cte->transmission.iobuffer) - strlen(PROTOCOL_KICK) );
        memcpy(cte->cm.parameter, (cte->transmission.iobuffer)+(strlen(PROTOCOL_KICK)*(sizeof(char))), length);
        cte->cm.code = CODE_KICK;
        t_print("Told to kick %s", cte->cm.parameter);
        return 1;
    }

    /* NMEA */
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_NMEA ) == (cte->transmission.iobuffer)) {
        cte->cm.code = CODE_NMEA;
        /* Fetch RMC */
        char *rmc_start = strstr(cte->transmission.iobuffer, RMC);
        char *gga_start = strstr(cte->transmission.iobuffer, GGA);
        memcpy(cte->nmea.raw_rmc, rmc_start, gga_start - rmc_start);
        memcpy(cte->nmea.raw_gga, gga_start, ( strlen(cte->transmission.iobuffer) - (rmc_start - cte->transmission.iobuffer) - (gga_start - rmc_start)));
        return 1;
    }    

    /* EXIT */
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_EXIT ) == (cte->transmission.iobuffer)) {
        cte->cm.code = CODE_DISCONNECT;
        return 1;
    }

    /* DISCONNECT */
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_DISCONNECT ) == (cte->transmission.iobuffer)) {
        cte->cm.code = CODE_DISCONNECT;
        return 1;
    }    

    return 0;
}

/* Responds to client action */
static int respond(struct client_table_entry *cte)
{
    /* Only print ">" if client is monitor */
    if(cte->client_id < 0){
        s_write(&(cte->transmission), ">", 1);  
    }

    int read_status = s_read(&(cte->transmission)); /* Blocking */
    if(read_status == -1) {
        t_print("Read failed or interrupted!\n");
        return -1;
    }

    int parse_status = parse_input(cte);

    if(parse_status == -1) {
        s_write(&(cte->transmission), ERROR_ILLEGAL_MESSAGE_SIZE,
                sizeof(ERROR_ILLEGAL_MESSAGE_SIZE));
    }
    if(parse_status == 0) {
        s_write(&(cte->transmission), ERROR_ILLEGAL_COMMAND,
                sizeof(ERROR_ILLEGAL_COMMAND));
    }
    if(parse_status == 1) {
        if(cte->cm.code == CODE_DISCONNECT) {
            t_print("Client %d requested DISCONNECT.\n", cte->client_id);
            s_write(&(cte->transmission), PROTOCOL_OK, sizeof(PROTOCOL_OK));
            s_write(&(cte->transmission), PROTOCOL_GOODBYE, sizeof(PROTOCOL_GOODBYE));
            return -1;
        }
        if(cte->cm.code == CODE_IDENTIFY) {
            int id = 0;

            if(sscanf(cte->cm.parameter, "%d", &id) == -1) {
                s_write(&(cte->transmission), ERROR_ILLEGAL_COMMAND, sizeof(ERROR_ILLEGAL_COMMAND));
                return 0;
            }

            struct client_table_entry* client_list_iterate;
            list_for_each_entry(client_list_iterate, &client_list->list, list) {
                if(client_list_iterate->client_id == id) {
                    cte->client_id = 0;
                    t_print("[%s] bounced! ID %d already in use.\n", cte->ip,id);
                    s_write(&(cte->transmission), "ID in use!\n", 11);
                    return -1;
                }
            }

            if(id < 0) {
                cte->client_type = MONITOR;
            } else {
                cte->client_type = SENSOR;
            }
            cte->client_id = id;
            t_print("[%s] ID set to: %d\n", cte->ip,cte->client_id);
            s_write(&(cte->transmission), PROTOCOL_OK, sizeof(PROTOCOL_OK));
            return 0;
        }

        if(cte->client_id == 0) {
            s_write(&(cte->transmission), ERROR_NO_ID, sizeof(ERROR_NO_ID));
            return 0;
        }

        if(cte->cm.code == CODE_PRINTCLIENTS) {
            print_clients(cte);
        }

        if(cte->cm.code == CODE_PRINTSERVER) {
            print_server_data(cte, s_data);
        }

        if(cte->cm.code == CODE_KICK) {
            int id = atoi(cte->cm.parameter);
            if(!id){
                s_write(&(cte->transmission), "ILLEGAL KICK REQUEST\n", 22);
            }else{
               struct client_table_entry* kick_cand = get_client_by_id(id);
               if(kick_cand != NULL){
                    close(kick_cand->transmission.session_fd);
               }
               else{
                s_write(&(cte->transmission), "NO SUCH CLIENT\n\n", 15);
               }
            }
        }

        if(cte->cm.code == CODE_NMEA) {
            int rmc_checksum = calc_nmea_checksum(cte->nmea.raw_rmc);
            int gga_checksum = calc_nmea_checksum(cte->nmea.raw_gga);
            if(rmc_checksum == 0 && gga_checksum == 0) {
                cte->timestamp = time(NULL);
                cte->checksum_passed = 1;
                extract_pos(cte);
                if(cte->warmup){
                    check_warm_up(cte);
                    warm_up(cte);
                }
            } else {
                cte->checksum_passed = 0;
                t_print("RMC and GGA received, checksum failed!\n");
            }

            /* If the client is finished with warming up */
        
            /* 
            * NOTE! This means that no data will be analyzed
            * before all the sensors are ready
            */
            if(!cte->warmup){
                sem_wait(&(s_synch->ready_mutex));

                if(nmea_ready()){
                    analyze();
                }else{
                    t_print("Not ready!\n");
                }
                sem_post(&(s_synch->ready_mutex));
                check_result();
            }
        }
    }
    return 0;
}

/* 
* Used to set extremes values to their opposite.
* This way, the comparison check in warm_up() is "true" 
* even tough they are checked for the first time 
*/
static void init_nmea(struct client_table_entry *cte)
{

    /* Setting low values */
    cte->nmea.lat_low = 9999.9999;
    cte->nmea.lon_low = 9999.9999;
    cte->nmea.alt_low = 9999.9999;

    /* Setting the high values */
    cte->nmea.lat_high = -9999.9999;
    cte->nmea.lon_high = -9999.9999;
    cte->nmea.alt_high = -9999.9999;

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

    /* Initializing structure */
    new_client->heartbeat_timeout.tv_sec = CLIENT_TIMEOUT + 1000; //remove 1000 when testing is done!
    new_client->heartbeat_timeout.tv_usec = 0;
    new_client->client_id = 0;
    new_client->transmission.session_fd = session_fd;
    new_client->moved = 0;

    /* Marked for warm up */
    new_client->warmup = 1;
    new_client->warmup_started = 0;

    init_nmea(new_client);

    memset(&new_client->transmission.iobuffer, 0, IO_BUFFER_SIZE*sizeof(char));
    memset(&new_client->cm.parameter, 0, MAX_PARAMETER_SIZE*sizeof(char));

    /* Setting socket timeout to default value */
    /* This doesn't always work for some reason, race condition? :/ */
    if (setsockopt (new_client->transmission.session_fd, SOL_SOCKET,
                    SO_RCVTIMEO, (char *)&new_client->heartbeat_timeout, sizeof(struct timeval)) < 0) {
        die(36,"setsockopt failed\n");
    }

    /*
    * Entering child process main loop
    * (Outer) breaks if server closes.
    * (Inner) Breaks (disconnects the client) if
    * respond < 0
    */
    while(!done) {
        if(respond(new_client) < 0) {
            break;
        }
    }
}