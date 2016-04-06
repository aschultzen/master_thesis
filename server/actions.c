#include "actions.h"

void kick_client(struct transmission_s *tsm, struct client_table_entry* candidate)
{
    sem_wait(&(s_synch->client_list_mutex));
    sem_wait(&(s_synch->ready_mutex));
    candidate->marked_for_kick = 1;
    sem_post(&(s_synch->ready_mutex));
    sem_post(&(s_synch->client_list_mutex));
}

/* Prints client X's solved time back to monitor */
void print_client_time(struct transmission_s *tsm, struct client_table_entry* candidate)
{
    int buffsize = 100;
    char buffer[buffsize];
    memset(&buffer, 0, buffsize);

    word_extractor(RMC_TIME_START,RMC_TIME_START + 1,',',buffer, buffsize,candidate->nmea.raw_rmc, strlen(candidate->nmea.raw_rmc));
    s_write(tsm, buffer, 12);
    s_write(tsm, "\n", 1);
}

/* Prints a formatted string containing info about connected clients to monitor */
void print_clients(struct client_table_entry *cte)
{
    char buffer [1000];
    int snprintf_status = 0;
    char *c_type = "SENSOR";
    char *modifier = "";

    struct client_table_entry* client_list_iterate;
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
        snprintf_status = snprintf( buffer, 1000, "%sPID: %d, IP:%s, TOUCH: %d, TYPE: %s, ID: %d%s\n",
                                    modifier,
                                    client_list_iterate->pid,
                                    client_list_iterate->ip,
                                    (int)difftime(time(NULL),client_list_iterate->timestamp),
                                    c_type,
                                    client_list_iterate->client_id, RESET);
       
        s_write(&(cte->transmission), buffer, snprintf_status);
    }
    s_write(&(cte->transmission), HORIZONTAL_BAR, sizeof(HORIZONTAL_BAR));
}

/* Prints a formatted string containing server info to monitor */
void print_server_data(struct client_table_entry *cte, struct server_data *s_data)
{
    char buffer [1000];
    int snprintf_status = 0;
    struct tm *loctime;
    loctime = localtime (&s_data->started);

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

/* 
* Prints a string containing simple description
* of the different implemented commands back
* to the monitor.
*/
void print_help(struct transmission_s *tsm)
{
    s_write(tsm, HELP, sizeof(HELP));
    s_write(tsm, PROTOCOL_OK, sizeof(PROTOCOL_OK));
}

/* 
* Prints MAX, MIN, CURRENT and AVERAGE position
* for client X back to the monitor
*/
void print_location(struct transmission_s *tsm, struct client_table_entry* candidate)
{
    char buffer [1000];
    int snprintf_status = 0;

    char *lat_modifier;
    char *lon_modifier;
    char *alt_modifier;
    char *speed_modifier;

    char *high_modifier = BOLD_BLK_RED;
    char *low_modifier = BOLD_WHT_CYN;

    char *reset = RESET;

    struct nmea_container nc;

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

    if(!nc.speed_disturbed){
        speed_modifier = BOLD_GRN_BLK;
    }else if(nc.speed_disturbed > 0){
        speed_modifier = BOLD_RED_BLK;
    }else{
        speed_modifier = BOLD_CYN_BLK;
    }

    snprintf_status = snprintf( buffer, 1000, "LAT: %s%f%s  %s%f%s  %s%f%s %f\nLON: %s%f%s  %s%f%s  %s%f%s %f\nALT: %s %f%s  %s %f%s  %s %f%s  %f\nSPD: %s   %f%s  %s   %f%s  %s   %f%s    %f\n",
                                lat_modifier,nc.lat_current,reset, low_modifier,nc.lat_low,reset, high_modifier,nc.lat_high,reset,nc.lat_average,
                                lon_modifier, nc.lon_current,reset, low_modifier,nc.lon_low,reset, high_modifier,nc.lon_high,reset,nc.lon_average, 
                                alt_modifier, nc.alt_current,reset, low_modifier,nc.alt_low,reset, high_modifier,nc.alt_high,reset,nc.alt_average,
                                speed_modifier, nc.speed_current,reset, low_modifier,nc.speed_low,reset, high_modifier,nc.speed_high,reset,nc.speed_average);
    s_write(tsm, buffer, snprintf_status); 
}

/* 
* Prints the difference between the calculated
* average values for location and the current value
*/
void print_avg_diff(struct client_table_entry *cte)
{
    char buffer [1000];
    int snprintf_status = 0;
    struct nmea_container nc;

    if(s_data->number_of_sensors > 0){
        s_write(&(cte->transmission), PRINT_AVG_DIFF_HEADER, sizeof(PRINT_AVG_DIFF_HEADER));
        struct client_table_entry* client_list_iterate;
        list_for_each_entry(client_list_iterate,&client_list->list, list) {
            if(client_list_iterate->client_id > 0){
                nc = client_list_iterate->nmea;
                snprintf_status = snprintf( buffer, 1000, "%d   %f  %f  %f  %f\n", 
                client_list_iterate->client_id, nc.lat_avg_diff, nc.lon_avg_diff, nc.alt_avg_diff, nc.speed_avg_diff);
                s_write(&(cte->transmission), buffer, snprintf_status);
            }
        }
    }else{
        s_write(&(cte->transmission), ERROR_NO_SENSORS_CONNECTED, sizeof(ERROR_NO_SENSORS_CONNECTED));
    }
}


/* Restart WARMUP procedure */
void restart_warmup(struct client_table_entry* target, struct transmission_s *tsm)
{
    target->warmup = 1;
    target->warmup_started = time(NULL);
    target->ready = 0;
    t_print("Sensor %d warmup restarted\n", target->client_id);
}

/* Dumps data location data for client X into a file */
int dumpdata(struct client_table_entry* target, struct transmission_s *tsm, char *filename)
{
	FILE *dumpfile;

    int full_filename_size = strlen(filename) + strlen(DATADUMP_EXTENSION);
    char full_filename[full_filename_size];
    memset(full_filename,'0', full_filename_size);

	/* No name specified, generate one instead */
	if(strlen(filename) == 0){
		int autoname_size = sizeof(DATADUMP_EXTENSION) + DUMPDATA_TIME_SIZE + ID_AS_STRING_MAX + 2;
		char autoname[autoname_size];
	    memset(autoname,'0',autoname_size);

	    char time_buffer[100];
	    time_t rawtime;
	    struct tm *info;
	    time(&rawtime);
	    info = gmtime(&rawtime);
	    strftime(time_buffer,80,"%d%m%y-%H%M%S", info);

	    char id_as_string[ID_AS_STRING_MAX];
	    sprintf(id_as_string, "%d", target->client_id);

	    strcat(autoname, id_as_string);
	    strcat(autoname, "_");
	    strcat(autoname, time_buffer);
	    strcat(autoname, DATADUMP_EXTENSION);
        *full_filename = *autoname;
		dumpfile=fopen(autoname, "wb");
        if(!dumpfile){
            t_print(ERROR_FOPEN);
            return 0;
        }
	}
	else{
        strcat(full_filename, filename);
        strcat(full_filename, DATADUMP_EXTENSION);
		dumpfile=fopen(filename, "wb");
        if(!dumpfile){
            t_print(ERROR_FOPEN);
            return 0;
        }
	}
    
    if(!fwrite(&target->nmea, sizeof(struct nmea_container), 1, dumpfile)){
        t_print(ERROR_FWRITE);
        return 0;
    }

    if(fclose(dumpfile)){
        t_print(ERROR_FCLOSE);
    }

    return 1;
}

