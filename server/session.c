#include "session.h"

/* 
* Used by spawned client processes to "mark" that their NMEA
* data is ready for processing. Works as a barrier in a way.
*/
static int nmea_ready()
{
    s_synch->ready_counter++;
    if(s_synch->ready_counter == s_data->number_of_sensors)
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

/* Check if a client is still warming up */
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

static void kick_client(struct transmission_s *tsm, int client_id)
{
    struct client_table_entry* kick_cand = get_client_by_id(client_id);
    if(kick_cand != NULL){
        sem_wait(&(s_synch->client_list_mutex));
            sem_wait(&(s_synch->ready_mutex));
                kick_cand->marked_for_kick = 1;
                s_write(tsm, PROTOCOL_OK, sizeof(PROTOCOL_OK));
            sem_post(&(s_synch->ready_mutex));
        sem_post(&(s_synch->client_list_mutex));
    }else{
        s_write(tsm, ERROR_NO_CLIENT, sizeof(ERROR_NO_CLIENT));
    }
}

/* Sends a formatted string containing info about connected clients */
static void print_clients(struct client_table_entry *cte)
{
    char buffer [1000];
    int snprintf_status = 0;
    char *c_type = "SENSOR";
    char *modifier = "";

    struct client_table_entry* client_list_iterate;
    s_write(&(cte->transmission), NEW_LINE, sizeof(NEW_LINE));
    s_write(&(cte->transmission), CLIENT_TABLE_LABEL, sizeof(CLIENT_TABLE_LABEL));
    s_write(&(cte->transmission), HORIZONTAL_BAR, sizeof(HORIZONTAL_BAR));
    list_for_each_entry(client_list_iterate,&client_list->list, list) {

        if(client_list_iterate->client_type == MONITOR) {
            c_type = "MONITOR";
        } else {
            c_type = "SENSOR";
        }

        if(cte->client_id == client_list_iterate->client_id){
            modifier = BOLD_GRN_BLK;
        }else{
            modifier = RESET;
        }
        snprintf_status = snprintf( buffer, 1000, "%sPID: %d, IP:%s, TOUCH: %d, TYPE: %s, ID: %d\n" RESET,
                                    modifier,
                                    client_list_iterate->pid,
                                    client_list_iterate->ip,
                                    (int)difftime(time(NULL),client_list_iterate->timestamp),
                                    c_type,
                                    client_list_iterate->client_id);
       
        s_write(&(cte->transmission), buffer, snprintf_status);
    }
    s_write(&(cte->transmission), HORIZONTAL_BAR, sizeof(HORIZONTAL_BAR));
}

/* Sends a formatted string containing server info */
static void print_server_data(struct client_table_entry *cte, struct server_data *s_data)
{
    char buffer [1000];
    int snprintf_status = 0;
    struct tm *loctime;
    loctime = localtime (&s_data->started);

    s_write(&(cte->transmission), NEW_LINE, sizeof(NEW_LINE));
    s_write(&(cte->transmission), SERVER_TABLE_LABEL, sizeof(SERVER_TABLE_LABEL));
    s_write(&(cte->transmission), HORIZONTAL_BAR, sizeof(HORIZONTAL_BAR));

    snprintf_status = snprintf( buffer, 1000,
                                "PID: %d\nNumber of clients: %d\nNumber of sensors: %d\nMax clients: %d\nStarted: %sVersion: %s\n",
                                s_data->pid,
                                s_data->number_of_clients,
                                s_data->number_of_sensors,
                                cfg->max_clients,
                                asctime (loctime),
                                s_data->version);

    s_write(&(cte->transmission), buffer, snprintf_status);
    s_write(&(cte->transmission), HORIZONTAL_BAR, sizeof(HORIZONTAL_BAR));
}

static void print_help(struct transmission_s *tsm)
{
    s_write(tsm, HELP, sizeof(HELP));
    s_write(tsm, PROTOCOL_OK, sizeof(PROTOCOL_OK));
}

static void print_location(struct transmission_s *tsm, int client_id)
{
    char buffer [1000];
    int snprintf_status = 0;

    char *lat_modifier;
    char *lon_modifier;
    char *alt_modifier;

    char *high_modifier = BOLD_BLK_RED;
    char *low_modifier = BOLD_WHT_CYN;

    char *reset = RESET;

    struct nmea_container nc;
    struct client_table_entry* candidate = get_client_by_id(client_id);
    if(candidate != NULL){
        nc = candidate->nmea;
        s_write(tsm, PRINT_LOCATION_HEADER, sizeof(PRINT_LOCATION_HEADER));

        /*Determining colors*/
        if(!nc.lat_disturbed){
            lat_modifier = BOLD_GRN_BLK;
        }else if(nc.lat_disturbed > 0){
            lat_modifier = BOLD_RED_BLK;
        }else{
            lat_modifier = BOLD_CYN_BLK;
        }

        if(!nc.lon_disturbed){
            lon_modifier = BOLD_GRN_BLK;
        }else if(nc.lon_disturbed > 0){
            lon_modifier = BOLD_RED_BLK;
        }else{
            lon_modifier = BOLD_CYN_BLK;
        }

        if(!nc.alt_disturbed){
            alt_modifier = BOLD_GRN_BLK;
        }else if(nc.alt_disturbed > 0){
            alt_modifier = BOLD_RED_BLK;
        }else{
            alt_modifier = BOLD_CYN_BLK;
        }

        snprintf_status = snprintf( buffer, 1000, "LAT: %s%f%s  %s%f%s  %s%f%s\nLON: %s%f%s  %s%f%s  %s%f%s\nALT: %s %f%s  %s %f%s  %s %f%s\n",
                                    lat_modifier,nc.lat_current,reset, low_modifier,nc.lat_low,reset, high_modifier,nc.lat_high,reset,
                                    lon_modifier, nc.lon_current,reset, low_modifier,nc.lon_low,reset, high_modifier,nc.lon_high,reset,
                                    alt_modifier, nc.alt_current,reset, low_modifier,nc.alt_low,reset, high_modifier,nc.alt_high,reset);
    s_write(tsm, buffer, snprintf_status);
    }else{
        s_write(tsm, ERROR_NO_CLIENT, sizeof(ERROR_NO_CLIENT));
    }    
}

/* Restart WARMUP procedure */
static void warmup(struct transmission_s *tsm, int client_id)
{
 struct client_table_entry* candidate = get_client_by_id(client_id);
    if(candidate != NULL){
        candidate->warmup = 1;
        candidate->warmup_started = time(NULL);
        t_print("Sensor %d warmup restarted!\n", client_id);
        s_write(tsm, PROTOCOL_OK, sizeof(PROTOCOL_OK));
    }else{
        s_write(tsm, ERROR_NO_CLIENT, sizeof(ERROR_NO_CLIENT));
    }    
}

/*
* Explanation:
* ------------
* Parses input from clients. Return value indicates status.
* Uses the command_code struct to convey parameter and command code. 
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

    /* PRINT_LOCATION */
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_PRINT_LOCATION ) == (cte->transmission.iobuffer)) {
        int length = (strlen(cte->transmission.iobuffer) - strlen(PROTOCOL_PRINT_LOCATION) );
        memcpy(cte->cm.parameter, (cte->transmission.iobuffer)+(strlen(PROTOCOL_PRINT_LOCATION)*(sizeof(char))), length);
        cte->cm.code = CODE_PRINT_LOCATION;
        return 1;
    } 

    /* WARMUP */
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_WARMUP ) == (cte->transmission.iobuffer)) {
        int length = (strlen(cte->transmission.iobuffer) - strlen(PROTOCOL_WARMUP) );
        memcpy(cte->cm.parameter, (cte->transmission.iobuffer)+(strlen(PROTOCOL_WARMUP)*(sizeof(char))), length);
        cte->cm.code = CODE_WARMUP;
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

    /* HELP */
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_HELP ) == (cte->transmission.iobuffer)) {
        cte->cm.code = CODE_HELP;
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

    if(cte->marked_for_kick){
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

        if(cte->cm.code == CODE_HELP) {
            print_help(&(cte->transmission));
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
                sem_wait(&(s_synch->client_list_mutex));
                    s_data->number_of_sensors++;
                sem_post(&(s_synch->client_list_mutex));
            }
            cte->client_id = id;
            t_print("[%s] ID set to: %d\n", cte->ip,cte->client_id);
            s_write(&(cte->transmission), PROTOCOL_OK, sizeof(PROTOCOL_OK));
            return 0;
        }

        if(cte->cm.code == CODE_PRINT_LOCATION) {
            int id = 0;

            if(sscanf(cte->cm.parameter, "%d", &id) == -1) {
                s_write(&(cte->transmission), ERROR_ILLEGAL_COMMAND, sizeof(ERROR_ILLEGAL_COMMAND));
                return 0;
            }
            print_location(&(cte->transmission), id);
        }

        if(cte->cm.code == CODE_WARMUP) {
            int id = 0;

            if(sscanf(cte->cm.parameter, "%d", &id) == -1) {
                s_write(&(cte->transmission), ERROR_ILLEGAL_COMMAND, sizeof(ERROR_ILLEGAL_COMMAND));
                return 0;
            }
            warmup(&(cte->transmission), id);
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
                kick_client(&(cte->transmission),id);
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
    cte->nmea.lat_low = 9999.999999;
    cte->nmea.lon_low = 9999.999999;
    cte->nmea.alt_low = 999.999999;

    /* Setting the high values */
    cte->nmea.lat_high = -9999.999999;
    cte->nmea.lon_high = -9999.999999;
    cte->nmea.alt_high = -999.999999;

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
    new_client->marked_for_kick = 0;

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