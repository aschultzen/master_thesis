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

    /* NMEA */
    if(strstr((char*)incoming, PROTOCOL_NMEA ) == (incoming)) {
        cte->cm.code = CODE_NMEA;
    }  

    /* IDENTIFY */
    else if(strstr((char*)incoming, PROTOCOL_IDENTIFY ) == (incoming)) {
        int length = (strlen(incoming) - strlen(PROTOCOL_IDENTIFY) );
        memcpy(cte->cm.parameter, (incoming)+(strlen(PROTOCOL_IDENTIFY)*(sizeof(char))), length);
        cte->cm.code = CODE_IDENTIFY;
    }

    /* IDENTIFY SHORT */
    else if(strstr((char*)incoming, PROTOCOL_IDENTIFY_SHORT ) == (incoming)) {
        int length = (strlen(incoming) - strlen(PROTOCOL_IDENTIFY_SHORT) );
        memcpy(cte->cm.parameter, (incoming)+(strlen(PROTOCOL_IDENTIFY_SHORT)*(sizeof(char))), length);
        cte->cm.code = CODE_IDENTIFY;
    }

    /* DUMPDATA */
    else if(strstr((char*)incoming, PROTOCOL_DUMPDATA ) == (incoming)) {
        int length = (strlen(incoming) - strlen(PROTOCOL_DUMPDATA) );
        memcpy(cte->cm.parameter, (incoming)+(strlen(PROTOCOL_DUMPDATA)*(sizeof(char))), length);
        cte->cm.code = CODE_DUMPDATA;
    }

    /* DUMPDATA_SHORT */
    else if(strstr((char*)incoming, PROTOCOL_DUMPDATA_SHORT ) == (incoming)) {
        int length = (strlen(incoming) - strlen(PROTOCOL_DUMPDATA_SHORT) );
        memcpy(cte->cm.parameter, (incoming)+(strlen(PROTOCOL_DUMPDATA_SHORT)*(sizeof(char))), length);
        cte->cm.code = CODE_DUMPDATA;
    }

    /* PRINT_LOCATION */
    else if(strstr((char*)incoming, PROTOCOL_PRINT_LOCATION ) == (incoming)) {
        int length = (strlen(incoming) - strlen(PROTOCOL_PRINT_LOCATION) );
        memcpy(cte->cm.parameter, (incoming)+(strlen(PROTOCOL_PRINT_LOCATION)*(sizeof(char))), length);
        cte->cm.code = CODE_PRINT_LOCATION;
    } 

    /* PRINT_LOCATION_SHORT */
    else if(strstr((char*)incoming, PROTOCOL_PRINT_LOCATION_SHORT ) == (incoming)) {
        int length = (strlen(incoming) - strlen(PROTOCOL_PRINT_LOCATION_SHORT) );
        memcpy(cte->cm.parameter, (incoming)+(strlen(PROTOCOL_PRINT_LOCATION_SHORT)*(sizeof(char))), length);
        cte->cm.code = CODE_PRINT_LOCATION;
    } 

    /* PRINTTIME */
    else if(strstr((char*)incoming, PROTOCOL_PRINTTIME ) == (incoming)) {
        int length = (strlen(incoming) - strlen(PROTOCOL_PRINTTIME) );
        memcpy(cte->cm.parameter, (incoming)+(strlen(PROTOCOL_PRINTTIME)*(sizeof(char))), length);
        cte->cm.code = CODE_PRINTTIME;
    } 

    /* WARMUP */
    else if(strstr((char*)incoming, PROTOCOL_WARMUP ) == (incoming)) {
        int length = (strlen(incoming) - strlen(PROTOCOL_WARMUP) );
        memcpy(cte->cm.parameter, (incoming)+(strlen(PROTOCOL_WARMUP)*(sizeof(char))), length);
        cte->cm.code = CODE_WARMUP;
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
        memcpy(cte->cm.parameter, (incoming)+(strlen(PROTOCOL_KICK)*(sizeof(char))), length);
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
    else if(strstr((char*)incoming, PROTOCOL_HELP ) == (incoming)) {
        cte->cm.code = CODE_HELP;
    } 

    /* PRINTAVGDIFF */
    else if(strstr((char*)incoming, PROTOCOL_PRINTAVGDIFF ) == (incoming) || 
        strstr((char*)incoming, PROTOCOL_PRINTAVGDIFF_SHORT ) == (incoming)) {
        cte->cm.code = CODE_PRINTAVGDIFF;
    }

    else{
        return 0;
    }
    return 1;
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
        t_print("[ CLIENT %d ] Read failed or interrupted!\n", cte->client_id);
        return 0;
    }

    if(cte->marked_for_kick){
        return 0;
    }

    int parse_status = parse_input(cte);

    if(parse_status == -1) {
        s_write(&(cte->transmission), ERROR_ILLEGAL_MESSAGE_SIZE,
                sizeof(ERROR_ILLEGAL_MESSAGE_SIZE));
    }
    else if(parse_status == 0) {
        s_write(&(cte->transmission), ERROR_ILLEGAL_COMMAND,
                sizeof(ERROR_ILLEGAL_COMMAND));
    }
    /* PARSING OK, CONTINUING */
    else{
        s_write(&(cte->transmission), PROTOCOL_OK, sizeof(PROTOCOL_OK));
            
        if(cte->cm.code == CODE_DISCONNECT) {
            t_print("Client %d requested DISCONNECT.\n", cte->client_id);
            s_write(&(cte->transmission), PROTOCOL_GOODBYE, sizeof(PROTOCOL_GOODBYE));
            return 0;
        }

        else if(cte->cm.code == CODE_HELP) {
            print_help(&(cte->transmission));
        }

        else if(cte->cm.code == CODE_IDENTIFY) {
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
                    return 0;
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
            return 1;
        }

        else if(cte->client_id == 0) {
            s_write(&(cte->transmission), ERROR_NO_ID, sizeof(ERROR_NO_ID));
        }

        else if(cte->cm.code == CODE_NMEA) {
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
                        return 1;
                    }
                    sem_post(&(s_synch->ready_mutex));
                }
            } else {
                cte->nmea.checksum_passed = 0;
                t_print("RMC and GGA received, checksum failed!\n");
            }
        }

        else if(cte->cm.code == CODE_PRINT_LOCATION) {
            int target_id = 0;

            if(sscanf(cte->cm.parameter, "%d", &target_id) == -1) {
                s_write(&(cte->transmission), ERROR_ILLEGAL_COMMAND, sizeof(ERROR_ILLEGAL_COMMAND));
            }
            else{
                struct client_table_entry* candidate = get_client_by_id(target_id);
                if(candidate == NULL){
                    s_write(&(cte->transmission), ERROR_NO_CLIENT, sizeof(ERROR_NO_CLIENT));
                }else{
                    print_location(&(cte->transmission), candidate);   
                }    
            }
        }

        else if(cte->cm.code == CODE_WARMUP) {
            int target_id = 0;

            if(sscanf(cte->cm.parameter, "%d", &target_id) == -1) {
                s_write(&(cte->transmission), ERROR_ILLEGAL_COMMAND, sizeof(ERROR_ILLEGAL_COMMAND));
            }else{
                if(target_id > 0){
                    struct client_table_entry* candidate = get_client_by_id(target_id);
                    if(candidate != NULL){
                        restart_warmup(candidate, &(cte->transmission));
                    }else{
                        s_write(&(cte->transmission), ERROR_NO_CLIENT, sizeof(ERROR_NO_CLIENT));
                    }
                }else{
                    s_write(&(cte->transmission), ERROR_WARMUP_NOT_SENSOR, sizeof(ERROR_WARMUP_NOT_SENSOR));
                }
            }
        }        

        else if(cte->cm.code == CODE_PRINTCLIENTS) {
            print_clients(cte);
        }

        else if(cte->cm.code == CODE_PRINTSERVER) {
            print_server_data(cte, s_data);
        }

        else if(cte->cm.code == CODE_PRINTTIME) {
            int target_id = atoi(cte->cm.parameter);
            if(!target_id){
                s_write(&(cte->transmission), ERROR_NO_CLIENT, sizeof(ERROR_NO_CLIENT));
            }else{
                struct client_table_entry* candidate = get_client_by_id(target_id);
                if(candidate != NULL){
                    print_client_time(&(cte->transmission), candidate);
                }else{
                    s_write(&(cte->transmission), ERROR_NO_CLIENT, sizeof(ERROR_NO_CLIENT));
                } 
            }   
        }

        else if(cte->cm.code == CODE_KICK) {
            int target_id = atoi(cte->cm.parameter);
            if(!target_id){
                s_write(&(cte->transmission), ERROR_NO_CLIENT, sizeof(ERROR_NO_CLIENT));
            }else{
                struct client_table_entry* candidate = get_client_by_id(target_id);
                if(candidate == NULL){
                    s_write(&(cte->transmission), ERROR_NO_CLIENT, sizeof(ERROR_NO_CLIENT)); 
                }
                else{
                    kick_client(&(cte->transmission),candidate);  
                }
            }
        }

        else if(cte->cm.code == CODE_DUMPDATA) {
            int filename_buffer_size = MAX_FILENAME_SIZE;
            char filename[filename_buffer_size];
            int target_id;
            char id_buffer[ID_AS_STRING_MAX];
            bzero(id_buffer, ID_AS_STRING_MAX);
            bzero(filename, filename_buffer_size);

            word_extractor(2,3, ' ', filename, filename_buffer_size,cte->cm.parameter, MAX_FILENAME_SIZE);

            if(strlen(filename) == 0){
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
                    if(!dumpdata(candidate, &(cte->transmission), filename)){
                        s_write(&(candidate->transmission), ERROR_DUMPDATA_FAILED, sizeof(ERROR_DUMPDATA_FAILED));
                    }else{
                        s_write(&(candidate->transmission), PROTOCOL_OK, sizeof(PROTOCOL_OK));
                    }
                }
                else{
                    s_write(&(cte->transmission), ERROR_NO_CLIENT, sizeof(ERROR_NO_CLIENT));
                }
            }
        }

        else if(cte->cm.code == CODE_PRINTAVGDIFF) {
            print_avg_diff(cte);
        }
        else{
            t_print("No action made for this part of the protocol\n");
        }
    }
    return 1;
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

    memset(&new_client->transmission.iobuffer, '0', IO_BUFFER_SIZE*sizeof(char));
    memset(&new_client->cm.parameter, '0', MAX_PARAMETER_SIZE*sizeof(char));

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
        if(!respond(new_client)) {
            break;
        }
    }
}