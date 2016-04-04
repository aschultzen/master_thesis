#include "session.h"

/* 
* Used by spawned client processes to "mark" that their NMEA
* data is ready for processing. Works as a barrier in a way.
*/
static int nmea_ready()
{
    struct client_table_entry* client_list_iterate;
    struct client_table_entry* temp;
    int ready = 0;

    list_for_each_entry_safe(client_list_iterate, temp, &client_list->list, list) {
        if(client_list_iterate->ready == 1) {
             ready++;
        }
    }
    if(ready == s_data->number_of_sensors){
        return 1;
    }
    else{
        return 0;
    }
}

/* Extract position data from NMEA */
static void extract_nmea_data(struct client_table_entry *cte)
{
    int buffsize = 100;
    char buffer[buffsize];
    memset(&buffer, 0, buffsize);

    /* Extracting latitude */
    word_extractor(LATITUDE_START,LATITUDE_START + 1,',',buffer, buffsize,cte->nmea.raw_rmc, strlen(cte->nmea.raw_rmc));
    cte->nmea.lat_current = atof(buffer);

    /* Extracting longitude */
    word_extractor(LONGITUDE_START,LONGITUDE_START + 1,',',buffer, buffsize,cte->nmea.raw_rmc, strlen(cte->nmea.raw_rmc));
    cte->nmea.lon_current = atof(buffer);

    /* Extracting altitude */
    word_extractor(ALTITUDE_START,ALTITUDE_START + 1,',',buffer, buffsize,cte->nmea.raw_gga, strlen(cte->nmea.raw_gga));
    cte->nmea.alt_current = atof(buffer);

    /* Extracting speed */
    word_extractor(SPEED_START,SPEED_START + 1,',',buffer, buffsize,cte->nmea.raw_rmc, strlen(cte->nmea.raw_rmc));
    cte->nmea.speed_current = atof(buffer);
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

    /* Updating speed */
    if(cte->nmea.speed_current > cte->nmea.speed_high){
        cte->nmea.speed_high = cte->nmea.speed_current;
    }

    if(cte->nmea.speed_current < cte->nmea.speed_low){
        cte->nmea.speed_low = cte->nmea.speed_current;
    }
}

/*
* Parses input from clients. Return value indicates status.
* Uses the command_code struct to convey parameter and command code. 
*
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

    /* NMEA */
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_NMEA ) == (cte->transmission.iobuffer)) {
        cte->cm.code = CODE_NMEA;
        return 1;
    }  

    /* IDENTIFY */
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_IDENTIFY ) == (cte->transmission.iobuffer)) {
        int length = (strlen(cte->transmission.iobuffer) - strlen(PROTOCOL_IDENTIFY) );
        memcpy(cte->cm.parameter, (cte->transmission.iobuffer)+(strlen(PROTOCOL_IDENTIFY)*(sizeof(char))), length);
        cte->cm.code = CODE_IDENTIFY;
        return 1;
    }

    /* IDENTIFY SHORT */
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_IDENTIFY_SHORT ) == (cte->transmission.iobuffer)) {
        int length = (strlen(cte->transmission.iobuffer) - strlen(PROTOCOL_IDENTIFY_SHORT) );
        memcpy(cte->cm.parameter, (cte->transmission.iobuffer)+(strlen(PROTOCOL_IDENTIFY_SHORT)*(sizeof(char))), length);
        cte->cm.code = CODE_IDENTIFY;
        return 1;
    }

    /* DUMPDATA */
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_DUMPDATA ) == (cte->transmission.iobuffer)) {
        int length = (strlen(cte->transmission.iobuffer) - strlen(PROTOCOL_DUMPDATA) );
        memcpy(cte->cm.parameter, (cte->transmission.iobuffer)+(strlen(PROTOCOL_DUMPDATA)*(sizeof(char))), length);
        cte->cm.code = CODE_DUMPDATA;
        return 1;
    }

    /* DUMPDATA_SHORT */
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_DUMPDATA_SHORT ) == (cte->transmission.iobuffer)) {
        int length = (strlen(cte->transmission.iobuffer) - strlen(PROTOCOL_DUMPDATA_SHORT) );
        memcpy(cte->cm.parameter, (cte->transmission.iobuffer)+(strlen(PROTOCOL_DUMPDATA_SHORT)*(sizeof(char))), length);
        cte->cm.code = CODE_DUMPDATA;
        return 1;
    }

    /* PRINT_LOCATION */
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_PRINT_LOCATION ) == (cte->transmission.iobuffer)) {
        int length = (strlen(cte->transmission.iobuffer) - strlen(PROTOCOL_PRINT_LOCATION) );
        memcpy(cte->cm.parameter, (cte->transmission.iobuffer)+(strlen(PROTOCOL_PRINT_LOCATION)*(sizeof(char))), length);
        cte->cm.code = CODE_PRINT_LOCATION;
        return 1;
    } 

    /* PRINT_LOCATION_SHORT */
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_PRINT_LOCATION_SHORT ) == (cte->transmission.iobuffer)) {
        int length = (strlen(cte->transmission.iobuffer) - strlen(PROTOCOL_PRINT_LOCATION_SHORT) );
        memcpy(cte->cm.parameter, (cte->transmission.iobuffer)+(strlen(PROTOCOL_PRINT_LOCATION_SHORT)*(sizeof(char))), length);
        cte->cm.code = CODE_PRINT_LOCATION;
        return 1;
    } 

    /* PRINTTIME */
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_PRINTTIME ) == (cte->transmission.iobuffer)) {
        int length = (strlen(cte->transmission.iobuffer) - strlen(PROTOCOL_PRINTTIME) );
        memcpy(cte->cm.parameter, (cte->transmission.iobuffer)+(strlen(PROTOCOL_PRINTTIME)*(sizeof(char))), length);
        cte->cm.code = CODE_PRINTTIME;
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
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_PRINTCLIENTS ) == (cte->transmission.iobuffer) || 
        strstr((char*)cte->transmission.iobuffer, PROTOCOL_PRINTCLIENTS_SHORT ) == (cte->transmission.iobuffer)) {
        cte->cm.code = CODE_PRINTCLIENTS;
        return 1;
    }

    /* PRINTSERVER */
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_PRINTSERVER ) == (cte->transmission.iobuffer) || 
        strstr((char*)cte->transmission.iobuffer, PROTOCOL_PRINTSERVER_SHORT ) == (cte->transmission.iobuffer)) {
        cte->cm.code = CODE_PRINTSERVER;
        return 1;
    }

    /* KICK */
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_KICK ) == (cte->transmission.iobuffer)) {
        int length = (strlen(cte->transmission.iobuffer) - strlen(PROTOCOL_KICK) );
        memcpy(cte->cm.parameter, (cte->transmission.iobuffer)+(strlen(PROTOCOL_KICK)*(sizeof(char))), length);
        cte->cm.code = CODE_KICK;
        return 1;
    }
  
    /* EXIT */
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_EXIT ) == (cte->transmission.iobuffer)) {
        cte->cm.code = CODE_DISCONNECT;
        return 1;
    }

    /* DISCONNECT */
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_DISCONNECT ) == (cte->transmission.iobuffer) || 
        strstr((char*)cte->transmission.iobuffer, PROTOCOL_DISCONNECT_SHORT ) == (cte->transmission.iobuffer)) {
        cte->cm.code = CODE_DISCONNECT;
        return 1;
    } 

    /* HELP */
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_HELP ) == (cte->transmission.iobuffer)) {
        cte->cm.code = CODE_HELP;
        return 1;
    } 

    /* PRINTAVGDIFF */
    if(strstr((char*)cte->transmission.iobuffer, PROTOCOL_PRINTAVGDIFF ) == (cte->transmission.iobuffer) || 
        strstr((char*)cte->transmission.iobuffer, PROTOCOL_PRINTAVGDIFF_SHORT ) == (cte->transmission.iobuffer)) {
        cte->cm.code = CODE_PRINTAVGDIFF;
        return 1;
    } 


    return 0;
}

/* Responds to client action */
static int respond(struct client_table_entry *cte)
{
    bzero(cte->cm.parameter, MAX_PARAMETER_SIZE);
    /* Only print ">" if client is monitor */
    if(cte->client_id < 0){
        s_write(&(cte->transmission), ">", 1);  
    }

    int read_status = s_read(&(cte->transmission)); /* Blocking */
    if(read_status == -1) {
        t_print("[ CLIENT % ] Read failed or interrupted!\n", cte->client_id);
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
    /* PARSING OK, CONTINUING */
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

        if(cte->client_id == 0) {
            s_write(&(cte->transmission), ERROR_NO_ID, sizeof(ERROR_NO_ID));
            return 0;
        }

        if(cte->cm.code == CODE_NMEA) {
            /* Fetching data from buffer */
            char *rmc_start = strstr(cte->transmission.iobuffer, RMC);
            char *gga_start = strstr(cte->transmission.iobuffer, GGA);
            memcpy(cte->nmea.raw_rmc, rmc_start, gga_start - rmc_start);
            memcpy(cte->nmea.raw_gga, gga_start, ( strlen(cte->transmission.iobuffer) - (rmc_start - cte->transmission.iobuffer) - (gga_start - rmc_start)));
            
            /* Checking NMEA checksum */
            int rmc_checksum = calculate_nmea_checksum(cte->nmea.raw_rmc);
            int gga_checksum = calculate_nmea_checksum(cte->nmea.raw_gga);
            if(rmc_checksum == 0 && gga_checksum == 0) {
                cte->timestamp = time(NULL);
                cte->nmea.checksum_passed = 1;
                extract_nmea_data(cte);
                calculate_nmea_average(cte);
                calculate_nmea_diff(cte);
                if(cte->warmup){
                    check_warm_up(cte);
                    warm_up(cte);
                }else{
                    cte->ready = 1;
                    sem_wait(&(s_synch->ready_mutex));
                    /* If all is ready, analyze */
                    if(nmea_ready()){
                        analyze();
                    }else{
                        sem_post(&(s_synch->ready_mutex));
                        return 0;
                    }
                    sem_post(&(s_synch->ready_mutex));
                }
            } else {
                cte->nmea.checksum_passed = 0;
                t_print("RMC and GGA received, checksum failed!\n");
            }
            return 0;
        }

        if(cte->cm.code == CODE_PRINT_LOCATION) {
            int target_id = 0;

            if(sscanf(cte->cm.parameter, "%d", &target_id) == -1) {
                s_write(&(cte->transmission), ERROR_ILLEGAL_COMMAND, sizeof(ERROR_ILLEGAL_COMMAND));
                return 0;
            }
            print_location(&(cte->transmission), target_id);
            return 0;
        }

        if(cte->cm.code == CODE_WARMUP) {
            int target_id = 0;

            if(sscanf(cte->cm.parameter, "%d", &target_id) == -1) {
                s_write(&(cte->transmission), ERROR_ILLEGAL_COMMAND, sizeof(ERROR_ILLEGAL_COMMAND));
                return 0;
            }

            if(target_id > 0){
                struct client_table_entry* candidate = get_client_by_id(target_id);
                if(candidate != NULL){
                    restart_warmup(candidate, &(cte->transmission));
                }
                else{
                    s_write(&(cte->transmission), ERROR_NO_CLIENT, sizeof(ERROR_NO_CLIENT));
                }
            }
            else{
                s_write(&(cte->transmission), ERROR_WARMUP_NOT_SENSOR, sizeof(ERROR_WARMUP_NOT_SENSOR));
            }

            return 0;
        }        

        if(cte->cm.code == CODE_PRINTCLIENTS) {
            print_clients(cte);
            return 0;
        }

        if(cte->cm.code == CODE_PRINTSERVER) {
            print_server_data(cte, s_data);
            return 0;
        }

        if(cte->cm.code == CODE_PRINTTIME) {
            int target_id = atoi(cte->cm.parameter);
            if(!target_id){
                s_write(&(cte->transmission), ERROR_NO_CLIENT, sizeof(ERROR_NO_CLIENT));
            }else{
                print_client_time(&(cte->transmission), target_id);
            }
            return 0;    
        }

        if(cte->cm.code == CODE_KICK) {
            int target_id = atoi(cte->cm.parameter);
            if(!target_id){
                s_write(&(cte->transmission), ERROR_ILLEGAL_KICK, sizeof(ERROR_ILLEGAL_KICK));
            }else{
                kick_client(&(cte->transmission),target_id);
            }
            return 0;
        }

        if(cte->cm.code == CODE_DUMPDATA) {
            int append_buffer_size = MAX_APPEND_LENGTH;
            char append_text[append_buffer_size];
            int target_id;
            char id_buffer[ID_AS_STRING_MAX];
            bzero(id_buffer, ID_AS_STRING_MAX);
            bzero(append_text, append_buffer_size);

            word_extractor(2,3, ' ', append_text, append_buffer_size,cte->cm.parameter, MAX_APPEND_LENGTH);

            if(strlen(append_text) == 0){
                target_id = atoi(cte->cm.parameter);
            }
            else{
                word_extractor(1,2, ' ', id_buffer, ID_AS_STRING_MAX,cte->cm.parameter, ID_AS_STRING_MAX);
                target_id = atoi(id_buffer);
            }

            if(!target_id){
                s_write(&(cte->transmission), ERROR_ILLEGAL_COMMAND, sizeof(ERROR_ILLEGAL_COMMAND));
            }else{
                struct client_table_entry* candidate = get_client_by_id(target_id);
                if(candidate != NULL){
                    dumpdata(candidate, &(cte->transmission), append_text);
                }
                else{
                    s_write(&(cte->transmission), ERROR_NO_CLIENT, sizeof(ERROR_NO_CLIENT));
                }
            }
        }

        if(cte->cm.code == CODE_PRINTAVGDIFF) {
            print_avg_diff(cte);
            return 0;
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
    new_client->was_moved = 0;
    new_client->marked_for_kick = 0;
    new_client->dumploc = 0;
    new_client->ready = 0;

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