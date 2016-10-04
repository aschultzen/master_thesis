#include "filters.h"

#define ALARM_MMF "[ ALARM ] Client %d triggered MIN_MAX!\n"
#define ALARM_MMF_RETURNED "[ ALARM ] Client %d MIN_MAX returned!\n"
#define ALARM_RDF "[ ALARM ] Client %d triggered REF_DEV!\n"
#define ALARM_RDF_RETURNED "[ ALARM ] Client %d REF_DEV returned!\n"

#define LOG_FILE "server_log"
#define LOG_STRING_LENGTH 100
#define MJD_LENGTH 15


static int log_alarm(int client_id, char *alarm)
{
    /* allocating memory for string */;
    char log_string[LOG_STRING_LENGTH];
    memset(log_string, '\0', LOG_STRING_LENGTH);

    /* Formatting alarm */
    char alarm_buffer[strlen(alarm) + ID_AS_STRING_MAX];
    memset(alarm_buffer, '\0', strlen(alarm) + ID_AS_STRING_MAX);
    snprintf(alarm_buffer, strlen(alarm) + ID_AS_STRING_MAX, alarm, client_id);

    /* Formatting output*/
    snprintf(log_string, LOG_STRING_LENGTH, " %s", alarm_buffer);

    /* Logging */
    return log_to_file(s_conf->log_path, log_string, 1);
}


void raise_alarm(void)
{
    struct client_table_entry* iterator;
    struct client_table_entry* safe;

    list_for_each_entry_safe(iterator, safe,&client_list->list, list) {
        if(iterator->client_id > 0) {
            /* Checking MIN-MAX */
            if(iterator->fs.mmf.moved == 1) {
                iterator->fs.mmf.was_moved = 1;
                iterator->fs.mmf.moved = 0;
                if(s_conf->logging) {
                    log_alarm(iterator->client_id, ALARM_MMF);
                }
                t_print(ALARM_MMF, iterator->client_id);
            } else {
                if(iterator->fs.mmf.was_moved) {
                    iterator->fs.mmf.was_moved = 0;
                    if(s_conf->logging) {
                        log_alarm(iterator->client_id, ALARM_MMF_RETURNED);
                    }
                    t_print(ALARM_MMF_RETURNED, iterator->client_id);
                }
            }

            /* Checking REF-DEV */
            if(iterator->fs.rdf.moved == 1) {
                iterator->fs.rdf.was_moved = 1;
                iterator->fs.rdf.moved = 0;
                if(s_conf->logging) {
                    log_alarm(iterator->client_id, ALARM_RDF);
                }
                t_print(ALARM_RDF, iterator->client_id);

            } else {
                if(iterator->fs.rdf.was_moved) {
                    iterator->fs.rdf.was_moved = 0;
                    if(s_conf->logging) {
                        log_alarm(iterator->client_id, ALARM_RDF_RETURNED);
                    }
                    t_print(ALARM_RDF_RETURNED, iterator->client_id);
                }
            }
        }
    }
}

void ref_dev_filter(void)
{
    struct client_table_entry* iterator;
    struct client_table_entry* safe;

    list_for_each_entry_safe(iterator, safe,&client_list->list, list) {

        if(iterator->nmea.lat_current > iterator->fs.rdf.rdd.lat_ref +
                iterator->fs.rdf.rdd.lat_dev) {
            iterator->fs.rdf.moved = 1;
            iterator->fs.rdf.dv.lat_disturbed = HIGH;
            printf("HIGH : %lf / %lf\n", iterator->nmea.lat_current,
                   iterator->fs.rdf.rdd.lat_ref + iterator->fs.rdf.rdd.lat_dev);
        } else if(iterator->nmea.lat_current < iterator->fs.rdf.rdd.lat_ref -
                  iterator->fs.rdf.rdd.lat_dev) {
            iterator->fs.rdf.moved = 1;
            iterator->fs.rdf.dv.lat_disturbed = LOW;
            printf("LOW : %lf / %lf\n", iterator->nmea.lat_current,
                   iterator->fs.rdf.rdd.lat_ref - iterator->fs.rdf.rdd.lat_dev);
        } else {
            iterator->fs.rdf.dv.lat_disturbed = SAFE;
        }

        if(iterator->nmea.alt_current > iterator->fs.rdf.rdd.alt_ref +
                iterator->fs.rdf.rdd.alt_dev) {
            iterator->fs.rdf.moved = 1;
            iterator->fs.rdf.dv.alt_disturbed = HIGH;
            printf("HIGH : %lf / %lf\n", iterator->nmea.alt_current,
                   iterator->fs.rdf.rdd.alt_ref + iterator->fs.rdf.rdd.alt_dev);
        } else if(iterator->nmea.alt_current < iterator->fs.rdf.rdd.alt_ref -
                  iterator->fs.rdf.rdd.alt_dev) {
            iterator->fs.rdf.moved = 1;
            iterator->fs.rdf.dv.alt_disturbed = LOW;
            printf("LOW : %lf / %lf\n", iterator->nmea.alt_current,
                   iterator->fs.rdf.rdd.alt_ref - iterator->fs.rdf.rdd.alt_dev);
        } else {
            iterator->fs.rdf.dv.alt_disturbed = SAFE;
        }

        if(iterator->nmea.lon_current > iterator->fs.rdf.rdd.lon_ref +
                iterator->fs.rdf.rdd.lon_dev) {
            iterator->fs.rdf.moved = 1;
            iterator->fs.rdf.dv.lon_disturbed = HIGH;
            printf("HIGH : %lf / %lf\n", iterator->nmea.lon_current,
                   iterator->fs.rdf.rdd.lon_ref + iterator->fs.rdf.rdd.lon_dev);
        } else if(iterator->nmea.lon_current < iterator->fs.rdf.rdd.lon_ref -
                  iterator->fs.rdf.rdd.lon_dev) {
            iterator->fs.rdf.moved = 1;
            iterator->fs.rdf.dv.lon_disturbed = LOW;
            printf("LOW : %lf / %lf\n", iterator->nmea.lon_current,
                   iterator->fs.rdf.rdd.lon_ref - iterator->fs.rdf.rdd.lon_dev);
        } else {
            iterator->fs.rdf.dv.lon_disturbed = SAFE;
        }

        if(iterator->nmea.speed_current > iterator->fs.rdf.rdd.speed_ref +
                iterator->fs.rdf.rdd.speed_dev) {
            iterator->fs.rdf.moved = 1;
            iterator->fs.rdf.dv.speed_disturbed = HIGH;
            printf("HIGH : %lf / %lf\n", iterator->nmea.speed_current,
                   iterator->fs.rdf.rdd.speed_ref + iterator->fs.rdf.rdd.speed_dev);
        } else if(iterator->nmea.speed_current < iterator->fs.rdf.rdd.speed_ref -
                  iterator->fs.rdf.rdd.speed_dev) {
            iterator->fs.rdf.moved = 1;
            iterator->fs.rdf.dv.speed_disturbed = LOW;
            printf("HIGH : %lf / %lf\n", iterator->nmea.speed_current,
                   iterator->fs.rdf.rdd.speed_ref - iterator->fs.rdf.rdd.speed_dev);
        } else {
            iterator->fs.rdf.dv.speed_disturbed = SAFE;
        }
    }
}

void min_max_filter(void)
{
    struct client_table_entry* iterator;
    struct client_table_entry* safe;

    list_for_each_entry_safe(iterator, safe,&client_list->list, list) {
        if(iterator->nmea.lat_current > iterator->nmea.lat_high) {
            iterator->fs.mmf.moved = 1;
            iterator->fs.mmf.dv.lat_disturbed = HIGH;
        } else if(iterator->nmea.lat_current < iterator->nmea.lat_low) {
            iterator->fs.mmf.moved = 1;
            iterator->fs.mmf.dv.lat_disturbed = LOW;
        } else {
            iterator->fs.mmf.dv.lat_disturbed = SAFE;
        }

        if(iterator->nmea.lon_current > iterator->nmea.lon_high) {
            iterator->fs.mmf.moved = 1;
            iterator->fs.mmf.dv.lon_disturbed = HIGH;
        } else if(iterator->nmea.lon_current < iterator->nmea.lon_low) {
            iterator->fs.mmf.moved = 1;
            iterator->fs.mmf.dv.lon_disturbed = LOW;
        } else {
            iterator->fs.mmf.dv.lon_disturbed = SAFE;
        }

        if(iterator->nmea.alt_current > iterator->nmea.alt_high) {
            iterator->fs.mmf.moved = 1;
            iterator->fs.mmf.dv.alt_disturbed = HIGH;
        } else if(iterator->nmea.alt_current < iterator->nmea.alt_low) {
            iterator->fs.mmf.moved = 1;
            iterator->fs.mmf.dv.alt_disturbed = LOW;
        } else {
            iterator->fs.mmf.dv.alt_disturbed = SAFE;
        }

        if(iterator->nmea.speed_current > iterator->nmea.speed_high) {
            iterator->fs.mmf.moved = 1;
            iterator->fs.mmf.dv.speed_disturbed = HIGH;
        } else if(iterator->nmea.speed_current < iterator->nmea.speed_low) {
            iterator->fs.mmf.moved = 1;
            iterator->fs.mmf.dv.speed_disturbed = LOW;
        } else {
            iterator->fs.mmf.dv.speed_disturbed = SAFE;
        }
    }
}