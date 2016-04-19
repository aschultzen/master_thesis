#include "actions.h"

/* GENERAL */
#define CLIENT_TABLE_LABEL "CLIENT TABLE\n"
#define NEW_LINE "\n"
#define PRINT_LOCATION_HEADER "      CURRENT        MIN          MAX          AVG\n"
#define DUMPDATA_HEADER "CURRENT        MIN           MAX      AVERAGE     AVG_DIFF      TOTAL      DISTURBED\n"
#define PRINT_AVG_DIFF_HEADER "ID     LAT        LON       ALT       SPEED\n"
#define DATADUMP_EXTENSION ".bin"
#define DATADUMP_HUMAN_EXTENSION ".txt"

/* ERRORS */

#define ERROR_APPEND_TOO_LONG "ERROR: TEXT TO APPEND TOO LONG\n"
#define ERROR_NO_SENSORS_CONNECTED "NO SENSORS CONNECTED\n"
#define ERROR_FCLOSE "Failed to close file, out of space?\n"
#define ERROR_FWRITE "Failed to write to file, aborting.\n"
#define ERROR_FREAD "Failed to read file, aborting.\n"
#define ERROR_FOPEN "Failed to open file, aborting.\n"
#define ERROR_UPDATE_WARMUP_ILLEGAL "Warm-up time value has to be greater than 0!\n"

/* HELP */
#define HELP "\n"\
" COMMAND      | SHORT | PARAM     | DESCRIPTION\n"\
"----------------------------------------------------------------------------------\n"\
" HELP         | ?     | NONE      | Prints this table\n"\
"----------------------------------------------------------------------------------\n"\
" IDENTIFY     | ID    | ID        | ID is set as the connected clients ID (you)\n"\
"----------------------------------------------------------------------------------\n"\
" DISCONNECT   | EXIT  | NONE      | Disconnects\n"\
"----------------------------------------------------------------------------------\n"\
" PRINTCLIENTS | PC    | NONE      | Prints an overview of connected clients\n"\
"----------------------------------------------------------------------------------\n"\
" PRINTSERVER  | PS    | NONE      | Prints server state and config\n"\
"----------------------------------------------------------------------------------\n"\
" PRINTTIME    |       | ID        | Prints time solved from <CLIENT ID>\n"\
"----------------------------------------------------------------------------------\n"\
" PRINTAVGDIFF | PAD   | NONE      | Prints all average diffs for all clients\n"\
"----------------------------------------------------------------------------------\n"\
" PRINTLOC     | PL    | ID        | Prints all average diffs for all clients\n"\
"----------------------------------------------------------------------------------\n"\
" LISTDATA     | LSD   | NONE      | Lists all dump files in server directory\n"\
"----------------------------------------------------------------------------------\n"\
" DUMPDATA     | DD    | ID & FILE | Dumps NMEA data of ID into FILE\n"\
"----------------------------------------------------------------------------------\n"\
" LOADDATA     | LD    | ID & FILE | Loads NMEA of FILE into sensor ID\n"\
"----------------------------------------------------------------------------------\n"\


/* SIZES */
#define DUMPDATA_TIME_SIZE 13
#define MAX_APPEND_LENGTH 20

void set_warmup(struct client_table_entry *client, int new_value)
{
    if(new_value > 0) {
        s_conf->warm_up_seconds = new_value;
    } else {
        s_write(&(client->transmission), ERROR_UPDATE_WARMUP_ILLEGAL, sizeof(ERROR_UPDATE_WARMUP_ILLEGAL));
    }

}

void kick_client(struct client_table_entry* client)
{
    sem_wait(&(s_synch->client_list_mutex));
    sem_wait(&(s_synch->ready_mutex));
    client->marked_for_kick = 1;
    sem_post(&(s_synch->ready_mutex));
    sem_post(&(s_synch->client_list_mutex));
}

/* Prints client X's solved time back to monitor */
void print_client_time(struct client_table_entry *monitor, struct client_table_entry* client)
{
    int buffsize = 100;
    char buffer[buffsize];
    memset(&buffer, 0, buffsize);

    substring_extractor(RMC_TIME_START,RMC_TIME_START + 1,',',buffer, buffsize,client->nmea.raw_rmc, strlen(client->nmea.raw_rmc));
    s_write(&(monitor->transmission), buffer, 12);
    s_write(&(monitor->transmission), "\n", 1);
}

/* Prints a formatted string containing info about connected clients to monitor */
void print_clients(struct client_table_entry *monitor)
{
    char buffer [1000];
    int snprintf_status = 0;
    char *c_type = "SENSOR";
    char *modifier = "";
    int time_left = 0;

    struct client_table_entry* client_list_iterate;
    s_write(&(monitor->transmission), CLIENT_TABLE_LABEL, sizeof(CLIENT_TABLE_LABEL));
    s_write(&(monitor->transmission), HORIZONTAL_BAR, sizeof(HORIZONTAL_BAR));
    list_for_each_entry(client_list_iterate,&client_list->list, list) {

        if(client_list_iterate->client_type == MONITOR) {
            c_type = "MONITOR";
        } else {
            c_type = "SENSOR";
        }

        if(client_list_iterate->client_type == SENSOR) {
            double elapsed_warmup = difftime(time(NULL), client_list_iterate->warmup_started);
            time_left = s_conf->warm_up_seconds - elapsed_warmup;
        } else {
            time_left = 0;
        }

        if(monitor->client_id == client_list_iterate->client_id) {
            modifier = BOLD_GRN_BLK;
        } else {
            modifier = RESET;
        }
        snprintf_status = snprintf( buffer, 1000,
                                    "%sPID: %d, " \
                                    "IP:%s, " \
                                    "TOUCH: %d, " \
                                    "TYPE: %s, " \
                                    "ID: %d " \
                                    "WARMUP LEFT: %d%s\n",
                                    modifier,
                                    client_list_iterate->pid,
                                    client_list_iterate->ip,
                                    (int)difftime(time(NULL),client_list_iterate->timestamp),
                                    c_type,
                                    client_list_iterate->client_id,
                                    time_left,
                                    RESET);

        s_write(&(monitor->transmission), buffer, snprintf_status);
    }
    s_write(&(monitor->transmission), HORIZONTAL_BAR, sizeof(HORIZONTAL_BAR));
}

/*
* Prints a string containing simple description
* of the different implemented commands back
* to the monitor.
*/
void print_help(struct client_table_entry *monitor)
{
    s_write(&(monitor->transmission), HELP, sizeof(HELP));
}

/*
* Prints MAX, MIN, CURRENT and AVERAGE position
* for client X back to the monitor
*/
void print_location(struct client_table_entry *monitor, struct client_table_entry* client)
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

    nc = client->nmea;
    s_write(&(monitor->transmission), PRINT_LOCATION_HEADER, sizeof(PRINT_LOCATION_HEADER));

    /*Determining colors*/
    if(!nc.lat_disturbed) {
        lat_modifier = BOLD_GRN_BLK;
    } else if(nc.lat_disturbed > 0) {
        lat_modifier = BOLD_RED_BLK;
    } else {
        lat_modifier = BOLD_CYN_BLK;
    }

    if(!nc.lon_disturbed) {
        lon_modifier = BOLD_GRN_BLK;
    } else if(nc.lon_disturbed > 0) {
        lon_modifier = BOLD_RED_BLK;
    } else {
        lon_modifier = BOLD_CYN_BLK;
    }

    if(!nc.alt_disturbed) {
        alt_modifier = BOLD_GRN_BLK;
    } else if(nc.alt_disturbed > 0) {
        alt_modifier = BOLD_RED_BLK;
    } else {
        alt_modifier = BOLD_CYN_BLK;
    }

    if(!nc.speed_disturbed) {
        speed_modifier = BOLD_GRN_BLK;
    } else if(nc.speed_disturbed > 0) {
        speed_modifier = BOLD_RED_BLK;
    } else {
        speed_modifier = BOLD_CYN_BLK;
    }

    snprintf_status = snprintf( buffer, 1000,
                                "LAT: %s%f%s  %s%f%s  %s%f%s %f\n" \
                                "LON: %s%f%s  %s%f%s  %s%f%s %f\n" \
                                "ALT: %s %f%s  %s %f%s  %s %f%s  %f\n" \
                                "SPD: %s   %f%s  %s   %f%s  %s   %f%s    %f\n",
                                lat_modifier,nc.lat_current,reset, low_modifier,nc.lat_low,reset, high_modifier,nc.lat_high,reset,nc.lat_average,
                                lon_modifier, nc.lon_current,reset, low_modifier,nc.lon_low,reset, high_modifier,nc.lon_high,reset,nc.lon_average,
                                alt_modifier, nc.alt_current,reset, low_modifier,nc.alt_low,reset, high_modifier,nc.alt_high,reset,nc.alt_average,
                                speed_modifier, nc.speed_current,reset, low_modifier,nc.speed_low,reset, high_modifier,nc.speed_high,reset,nc.speed_average);
    s_write(&(monitor->transmission), buffer, snprintf_status);
}

/*
* Prints the difference between the calculated
* average values for location and the current value
*/
void print_avg_diff(struct client_table_entry *client)
{
    char buffer [1000];
    int snprintf_status = 0;
    struct nmea_container nc;

    if(s_data->number_of_sensors > 0) {
        s_write(&(client->transmission), PRINT_AVG_DIFF_HEADER, sizeof(PRINT_AVG_DIFF_HEADER));
        struct client_table_entry* client_list_iterate;
        list_for_each_entry(client_list_iterate,&client_list->list, list) {
            if(client_list_iterate->client_id > 0) {
                nc = client_list_iterate->nmea;
                snprintf_status = snprintf( buffer, 1000, "%d   %f  %f  %f  %f\n",
                                            client_list_iterate->client_id, nc.lat_avg_diff, nc.lon_avg_diff, nc.alt_avg_diff, nc.speed_avg_diff);
                s_write(&(client->transmission), buffer, snprintf_status);
            }
        }
    } else {
        s_write(&(client->transmission), ERROR_NO_SENSORS_CONNECTED, sizeof(ERROR_NO_SENSORS_CONNECTED));
    }
}


/* Restart WARMUP procedure */
void restart_warmup(struct client_table_entry* client)
{
    client->warmup = 1;
    client->warmup_started = time(NULL);
    client->ready = 0;
    t_print("Sensor %d warmup restarted\n", client->client_id);
}

/* Dumps data location data for client X into a file */
int datadump(struct client_table_entry* client, char *filename, int dump_human_read)
{
    FILE *bin_file;
    char bin_name[strlen(filename) + strlen(DATADUMP_EXTENSION)];
    strcpy(bin_name, filename);
    strcat(bin_name, DATADUMP_EXTENSION);

    bin_file=fopen(bin_name, "wb");

    if(!bin_file) {
        t_print(ERROR_FOPEN);
        return 0;
    }

    if(!fwrite(&client->nmea, sizeof(struct nmea_container), 1, bin_file)){
        t_print(ERROR_FWRITE);
        return 0;
    }

    if(fclose(bin_file)) {
        t_print(ERROR_FCLOSE);
    }

    if(dump_human_read){
        /* Dumping humanly readable data */
        FILE *h_dump;
        char h_name[strlen(filename) + strlen(DATADUMP_HUMAN_EXTENSION)];
        strcpy(h_name, filename);
        strcat(h_name, DATADUMP_HUMAN_EXTENSION);

        h_dump = fopen(h_name, "wb");

        fprintf(h_dump, "Sensor Server dumpfile created for client %d\n", client->client_id);

        int inner_counter = 0;
        int outer_counter = 0;
        double *data = &client->nmea.lat_current;
 
        fprintf(h_dump,DUMPDATA_HEADER);
        while(outer_counter < 4) {
            while(inner_counter < 7) {
                fprintf(h_dump, "%f  ",*data);
                data++;
                inner_counter++;
            }
            fprintf(h_dump, "%s", "\n");
            inner_counter = 0;
            outer_counter++;
        }

        if(fclose(h_dump)) {
            t_print(ERROR_FCLOSE);
        }
    }
    return 1;
}

int listdumps(struct client_table_entry* monitor)
{
  DIR *dp;
  struct dirent *ep;

  dp = opendir ("./");
  if(dp != NULL){
      while ( (ep = readdir(dp)) ){
        if(strstr(ep->d_name,DATADUMP_EXTENSION) != NULL){
            s_write(&(monitor->transmission),ep->d_name, strlen(ep->d_name));
            s_write(&(monitor->transmission),NEW_LINE, sizeof(NEW_LINE));
        }
      }
      closedir (dp);
    }else{
    return 0;
  }

  return 1;
}

int loaddata(struct client_table_entry* target, char *filename)
{
    FILE *dump_file;
    int file_len = 0;


    dump_file=fopen(filename, "rb");

    if(!dump_file) {
        t_print(ERROR_FOPEN);
        return ERROR_CODE_NO_FILE;
    }

    /* Checking file length */
    fseek(dump_file, 0, SEEK_END);
    file_len=ftell(dump_file);
    fseek(dump_file, 0, SEEK_SET);

    int f_s = fread( &target->nmea,1,sizeof(struct nmea_container), dump_file);

    t_print("Read %d/%d bytes successfully from %s\n", f_s, file_len,filename);

    if(f_s == 0){
        t_print(ERROR_FREAD);
        return ERROR_CODE_READ_FAILED;
    }

    if(fclose(dump_file)) {
        t_print(ERROR_FCLOSE);
    }

    return 1;
}