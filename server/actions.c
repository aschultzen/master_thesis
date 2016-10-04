#include "actions.h"

/* GENERAL */
#define CLIENT_TABLE_LABEL "CLIENT TABLE\n"
#define NEW_LINE "\n"
#define PRINT_LOCATION_HEADER "      CURRENT        MIN          MAX          AVG\n"
#define DUMPDATA_HEADER "CURRENT        MIN           MAX      AVERAGE     AVG_DIFF      TOTAL      DISTURBED\n"
#define PRINT_AVG_DIFF_HEADER "ID     LAT        LON       ALT       SPEED\n"
#define DATADUMP_EXTENSION ".bin"
#define DATADUMP_HUMAN_EXTENSION ".txt"
#define RDF_HEADER "\nREF_DEV_FILTER DATA\n"
#define CSAC_SCRIPT_COMMAND "python query_csac.py "

/* ERRORS */
#define ERROR_APPEND_TOO_LONG "ERROR: TEXT TO APPEND TOO LONG\n"
#define ERROR_NO_SENSORS_CONNECTED "NO SENSORS CONNECTED\n"
#define ERROR_FCLOSE "Failed to close file, out of space?\n"
#define ERROR_FWRITE "Failed to write to file, aborting.\n"
#define ERROR_FREAD "Failed to read file, aborting.\n"
#define ERROR_FOPEN "Failed to open file, aborting.\n"
#define ERROR_UPDATE_WARMUP_ILLEGAL "Warm-up time value has to be greater than 0!\n"
#define ERROR_CSAC_FAILED "Communication with CSAC failed!\n"

/* LOAD_REF_DEV_DATA */
#define REF_DEV_FILENAME "ref_dev_sensor"
#define ALT_REF "alt_ref:"
#define LON_REF "lon_ref:"
#define LAT_REF "lat_ref:"
#define SPEED_REF "speed_ref:"
#define ALT_DEV "alt_dev:"
#define LON_DEV "lon_dev:"
#define LAT_DEV "lat_dev:"
#define SPEED_DEV "speed_dev:"
#define LOAD_REF_DEV_DATA_ENTRIES 8

/* HELP */
#define HELP "\n"\
" COMMAND      | SHORT | PARAM     | DESCRIPTION\n"\
"--------------------------------------------------------------------------------\n"\
" HELP         | ?     | NONE      | Prints this table\n"\
"--------------------------------------------------------------------------------\n"\
" IDENTIFY     | ID    | ID        | Your ID is set to PARAM ID\n"\
"--------------------------------------------------------------------------------\n"\
" DISCONNECT   | EXIT  | NONE      | Disconnects\n"\
"--------------------------------------------------------------------------------\n"\
" PRINTCLIENTS | PC    | NONE      | Prints an overview of connected clients\n"\
"--------------------------------------------------------------------------------\n"\
" PRINTSERVER  | PS    | NONE      | Prints server state and config\n"\
"--------------------------------------------------------------------------------\n"\
" PRINTTIME    |       | ID        | Prints time solved from <CLIENT ID>\n"\
"--------------------------------------------------------------------------------\n"\
" PRINTAVGDIFF | PAD   | NONE      | Prints all average diffs for all clients\n"\
"--------------------------------------------------------------------------------\n"\
" PRINTLOC     | PL    | ID        | Prints all average diffs for all clients\n"\
"--------------------------------------------------------------------------------\n"\
" LISTDATA     | LSD   | NONE      | Lists all dump files in server directory\n"\
"--------------------------------------------------------------------------------\n"\
" DUMPDATA     | DD    | ID & FILE | Dumps NMEA data of ID into FILE\n"\
"--------------------------------------------------------------------------------\n"\
" LOADDATA     | LD    | ID & FILE | Loads NMEA of FILE into sensor ID\n"\
"--------------------------------------------------------------------------------\n"\
" QUERYCSAC    | QC    | COMMAND   | Queries the CSAC with parameter COMMAND\n"\
"--------------------------------------------------------------------------------\n"\
" LOADRFDATA   | LRFD  | ID        | Loads REF_DEV_FILTER data into clint<ID>\n"\
"--------------------------------------------------------------------------------\n"\
" PRINTCFD     | PFD   |           | Prints CSAC filter data\n"\
"--------------------------------------------------------------------------------\n"\

/* SIZES */
#define DUMPDATA_TIME_SIZE 13
#define MAX_APPEND_LENGTH 20

void set_warmup(struct client_table_entry *client, int new_value)
{
    if(new_value > 0) {
        s_conf->warm_up_seconds = new_value;
    } else {
        s_write(&(client->transmission), ERROR_UPDATE_WARMUP_ILLEGAL,
                sizeof(ERROR_UPDATE_WARMUP_ILLEGAL));
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
void print_client_time(struct client_table_entry *monitor,
                       struct client_table_entry* client)
{
    int buffsize = 100;
    char buffer[buffsize];
    memset(&buffer, 0, buffsize);

    substring_extractor(RMC_TIME_START,RMC_TIME_START + 1,',',buffer, buffsize,
                        client->nmea.raw_rmc, strlen(client->nmea.raw_rmc));
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
    s_write(&(monitor->transmission), CLIENT_TABLE_LABEL,
            sizeof(CLIENT_TABLE_LABEL));
    s_write(&(monitor->transmission), HORIZONTAL_BAR, sizeof(HORIZONTAL_BAR));
    list_for_each_entry(client_list_iterate,&client_list->list, list) {

        if(client_list_iterate->client_type == MONITOR) {
            c_type = "MONITOR";
        } else {
            c_type = "SENSOR";
        }

        if(client_list_iterate->client_type == SENSOR) {
            double elapsed_warmup = difftime(time(NULL),
                                             client_list_iterate->fs.mmf.warmup_started);
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
void print_location(struct client_table_entry *monitor,
                    struct client_table_entry* client)
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
    s_write(&(monitor->transmission), PRINT_LOCATION_HEADER,
            sizeof(PRINT_LOCATION_HEADER));

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
                                lat_modifier,nc.lat_current,reset, low_modifier,nc.lat_low,reset, high_modifier,
                                nc.lat_high,reset,nc.lat_average,
                                lon_modifier, nc.lon_current,reset, low_modifier,nc.lon_low,reset,
                                high_modifier,nc.lon_high,reset,nc.lon_average,
                                alt_modifier, nc.alt_current,reset, low_modifier,nc.alt_low,reset,
                                high_modifier,nc.alt_high,reset,nc.alt_average,
                                speed_modifier, nc.speed_current,reset, low_modifier,nc.speed_low,reset,
                                high_modifier,nc.speed_high,reset,nc.speed_average);
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
        s_write(&(client->transmission), PRINT_AVG_DIFF_HEADER,
                sizeof(PRINT_AVG_DIFF_HEADER));
        struct client_table_entry* client_list_iterate;
        list_for_each_entry(client_list_iterate,&client_list->list, list) {
            if(client_list_iterate->client_id > 0) {
                nc = client_list_iterate->nmea;
                snprintf_status = snprintf( buffer, 1000, "%d   %f  %f  %f  %f\n",
                                            client_list_iterate->client_id, nc.lat_avg_diff, nc.lon_avg_diff,
                                            nc.alt_avg_diff, nc.speed_avg_diff);
                s_write(&(client->transmission), buffer, snprintf_status);
            }
        }
    } else {
        s_write(&(client->transmission), ERROR_NO_SENSORS_CONNECTED,
                sizeof(ERROR_NO_SENSORS_CONNECTED));
    }
}

static int get_pfd_string(char *buffer, int buf_len)
{
    memset(buffer, '\0',buf_len);
    int snprintf_status = snprintf( buffer, 1000,
                                    "Phase:                     %lf\n\n" \
                                    "T current:                 %lf\n" \
                                    "T current (smooth):        %lf\n" \
                                    "T previous (smooth):       %lf\n" \
                                    "T today (smooth):          %lf\n" \
                                    "T yesterday (smooth):      %lf\n\n" \
                                    "Steer current:             %lf\n" \
                                    "Steer current (smooth):    %lf\n" \
                                    "Steer previous (smooth):   %lf\n\n" \
                                    "Steer today (smooth):      %lf\n" \
                                    "Steer yesterday (smooth):  %lf\n\n" \
                                    "Steer prediction:          %lf\n\n" \
                                    "MJD today:                 %lf\n" \
                                    "Days passed since startup: %d\n\n" \
                                    "Discipline status:         %d\n" \
                                    "Fast timing filter status  %d\n" \
                                    "Freq corr. filter status   %d\n\n",
                                    cfd->phase_current,
                                    cfd->t_current,
                                    cfd->t_smooth_current,
                                    cfd->t_smooth_previous,
                                    cfd->t_smooth_today,
                                    cfd->t_smooth_yesterday,
                                    cfd->steer_current,
                                    cfd->steer_smooth_current,
                                    cfd->steer_smooth_previous,
                                    cfd->steer_smooth_today,
                                    cfd->steer_smooth_yesterday,
                                    cfd->steer_prediction,
                                    cfd->today_mjd,
                                    cfd->days_passed,
                                    cfd->discok,
                                    cfd->ftf_status,
                                    cfd->fqf_status);
    return snprintf_status;
}

void print_cfd(struct client_table_entry *monitor, int update_count)
{
    int buf_len = 1000;
    char buffer [buf_len];
    int counter = 0;

    if(update_count == 0) {
        update_count = 1;
    }

    while(counter < update_count) {
        get_pfd_string(buffer, buf_len);
        s_write(&(monitor->transmission), buffer, strlen(buffer));
        counter++;
        sleep(1);
    }
}

int dump_cfd(char *path)
{
    int buf_len = 1000;
    char buffer[buf_len];

    /* Formating string with CSAC filter data */
    get_pfd_string(buffer, buf_len);

    /* Opening and writing to file */
    FILE *cfd_file;
    cfd_file = fopen(path, "w+");

    if(!cfd_file) {
        t_print("dump_cfd: %s: %s",ERROR_FOPEN, path);
        return 0;
    }

    if(!fprintf(cfd_file,"%s", buffer) ) {
        t_print(ERROR_FWRITE);
        return 0;
    }

    if(fclose(cfd_file)) {
        t_print(ERROR_FCLOSE);
    }
    return 1;
}


/* Restart WARMUP procedure */
void restart_warmup(struct client_table_entry* client)
{
    client->fs.mmf.warmup = 1;
    client->fs.mmf.warmup_started = time(NULL);
    client->ready = 0;
    t_print("Sensor %d warmup restarted\n", client->client_id);
}

/* Dumps data location data for client X into a file */
int datadump(struct client_table_entry* client, char *filename,
             int dump_human_read)
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

    if(!fwrite(&client->nmea, sizeof(struct nmea_container), 1, bin_file)) {
        t_print(ERROR_FWRITE);
        return 0;
    }

    if(fclose(bin_file)) {
        t_print(ERROR_FCLOSE);
    }

    if(dump_human_read) {
        /* Dumping humanly readable data */
        FILE *h_dump;
        char h_name[strlen(filename) + strlen(DATADUMP_HUMAN_EXTENSION)];
        strcpy(h_name, filename);
        strcat(h_name, DATADUMP_HUMAN_EXTENSION);

        h_dump = fopen(h_name, "wb");

        fprintf(h_dump, "Sensor Server dumpfile created for client %d\n",
                client->client_id);

        /*
        * Dumping all from NMEA container
        * after raw_rmc and including speed_disturbed
        */
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
            fprintf(h_dump, "%f", *data);
            inner_counter = 0;
            outer_counter++;
        }

        /*
        * Dumping ref_dev_data
        */
        fprintf(h_dump,DUMPDATA_HEADER);
        inner_counter = 0;
        double *rdf = &client->fs.rdf.rdd.alt_ref;
        while(inner_counter < 8) {
            fprintf(h_dump, "%lf \n",*rdf);
            rdf++;
            inner_counter++;
        }

        if(fclose(h_dump)) {
            t_print(ERROR_FCLOSE);
        }
    }
    return 1;
}

/* Print list of dumped data */
int listdumps(struct client_table_entry* monitor)
{
    DIR *dp;
    struct dirent *ep;

    dp = opendir ("./");
    if(dp != NULL) {
        while ( (ep = readdir(dp)) ) {
            if(strstr(ep->d_name,DATADUMP_EXTENSION) != NULL) {
                s_write(&(monitor->transmission),ep->d_name, strlen(ep->d_name));
                s_write(&(monitor->transmission),NEW_LINE, sizeof(NEW_LINE));
            }
        }
        closedir (dp);
    } else {
        return 0;
    }

    return 1;
}

/* Load dumped data into the client */
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

    if(f_s == 0) {
        t_print(ERROR_FREAD);
        return ERROR_CODE_READ_FAILED;
    }

    if(fclose(dump_file)) {
        t_print(ERROR_FCLOSE);
    }

    return 1;
}

int query_csac(char *query, char *buffer)
{
    /* Building command */
    int command_size = MAX_PARAMETER_SIZE + sizeof(CSAC_SCRIPT_COMMAND);
    char command[command_size];
    memset(command,'\0', command_size);
    strcat(command, CSAC_SCRIPT_COMMAND);
    strcat(command, query);

    /* Acquiring lock*/
    sem_wait(&(s_synch->csac_mutex));

    /* Running command */
    if(!run_command(command, buffer)) {
        /* Releasing lock */
        sem_post(&(s_synch->csac_mutex));
        return 0;
    }

    /* Releasing lock */
    sem_post(&(s_synch->csac_mutex));
    return 1;
}


int client_query_csac(struct client_table_entry *monitor, char *query)
{
    char buffer[MAX_PARAMETER_SIZE];
    memset(buffer, '\0', MAX_PARAMETER_SIZE);

    if(!query_csac(query, buffer)) {
        return 0;
    }

    if(!s_write(&(monitor->transmission), buffer, strlen(buffer))) {
        return 0;
    }
    return 1;
}

/*
* Load ref_dev data into the client struct.
* Re-using the config loader.
* This whole function needs some work! Magic numbers beware.
*/
int load_ref_def_data(struct client_table_entry* target)
{
    /* Request lock */
    sem_wait(&(s_synch->client_list_mutex));
    sem_wait(&(s_synch->ready_mutex));
    struct config_map_entry conf_map[LOAD_REF_DEV_DATA_ENTRIES];

    int filename_length = strlen(REF_DEV_FILENAME) + 10;
    char filename[filename_length];
    memset(filename,'\0' ,filename_length);
    strcpy(filename, REF_DEV_FILENAME);

    /* Way overkill for int to string, but still. */
    char id[10];
    memset(id,'\0' ,10);
    sprintf(id, "%d", target->client_id);
    strcat(filename, id);

    conf_map[0].entry_name = ALT_REF;
    conf_map[0].modifier = FORMAT_DOUBLE;
    conf_map[0].destination = &target->fs.rdf.rdd.alt_ref;

    conf_map[1].entry_name = LON_REF;
    conf_map[1].modifier = FORMAT_DOUBLE;
    conf_map[1].destination = &target->fs.rdf.rdd.lon_ref;

    conf_map[2].entry_name = LAT_REF;
    conf_map[2].modifier = FORMAT_DOUBLE;
    conf_map[2].destination = &target->fs.rdf.rdd.lat_ref;

    conf_map[3].entry_name = SPEED_REF;
    conf_map[3].modifier = FORMAT_DOUBLE;
    conf_map[3].destination = &target->fs.rdf.rdd.speed_ref;

    conf_map[4].entry_name = ALT_DEV;
    conf_map[4].modifier = FORMAT_DOUBLE;
    conf_map[4].destination = &target->fs.rdf.rdd.alt_dev;

    conf_map[5].entry_name = LON_DEV;
    conf_map[5].modifier = FORMAT_DOUBLE;
    conf_map[5].destination = &target->fs.rdf.rdd.lon_dev;

    conf_map[6].entry_name = LAT_DEV;
    conf_map[6].modifier = FORMAT_DOUBLE;
    conf_map[6].destination = &target->fs.rdf.rdd.lat_dev;

    conf_map[7].entry_name = SPEED_DEV;
    conf_map[7].modifier = FORMAT_DOUBLE;
    conf_map[7].destination = &target->fs.rdf.rdd.speed_dev;

    t_print("Loading filter data from: %s\n", filename);

    int load_config_status = load_config(conf_map, filename,
                                         LOAD_REF_DEV_DATA_ENTRIES);

    /* releasing lock */
    sem_post(&(s_synch->ready_mutex));
    sem_post(&(s_synch->client_list_mutex));
    return load_config_status;
}
