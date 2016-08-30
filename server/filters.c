#include "filters.h"

#define ALARM_MOVED "[ ALARM ] Client %d was moved!\n"
#define ALARM_RETURNED "[ ALARM ] Client %d has returned!\n"

void raise_alarm(void)
{
    struct client_table_entry* iterator;
    struct client_table_entry* safe;

    list_for_each_entry_safe(iterator, safe,&client_list->list, list) {
        if(iterator->client_id > 0) {
            if(iterator->moved == 1) {
                iterator->was_moved = 1;
                iterator->moved = 0;
                t_print(ALARM_MOVED, iterator->client_id);
            } else {
                if(iterator->was_moved) {
                    iterator->was_moved = 0;
                    t_print(ALARM_RETURNED, iterator->client_id);
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
        if(iterator->nmea.lat_current > s_conf->rdd.lat_ref + s_conf->rdd.lat_dev) {
            iterator->moved = 1;
            iterator->nmea.lat_disturbed = HIGH;
        }
        else if(iterator->nmea.lat_current < s_conf->rdd.lat_ref - s_conf->rdd.lat_dev) {
            iterator->moved = 1;
            iterator->nmea.lat_disturbed = LOW;
        }
        else {
            iterator->nmea.lat_disturbed = SAFE;
        }

        if(iterator->nmea.alt_current > s_conf->rdd.alt_ref + s_conf->rdd.alt_dev) {
            iterator->moved = 1;
            iterator->nmea.alt_disturbed = HIGH;
        }
        else if(iterator->nmea.alt_current < s_conf->rdd.alt_ref - s_conf->rdd.alt_dev) {
            iterator->moved = 1;
            iterator->nmea.alt_disturbed = LOW;
        }
        else {
            iterator->nmea.alt_disturbed = SAFE;
        }

        if(iterator->nmea.lon_current > s_conf->rdd.lon_ref + s_conf->rdd.lon_dev) {
            iterator->moved = 1;
            iterator->nmea.lon_disturbed = HIGH;
        }
        else if(iterator->nmea.lon_current < s_conf->rdd.lon_ref - s_conf->rdd.lon_dev) {
            iterator->moved = 1;
            iterator->nmea.lon_disturbed = LOW;
        }
        else {
            iterator->nmea.lon_disturbed = SAFE;
        }

        if(iterator->nmea.speed_current > s_conf->rdd.speed_ref + s_conf->rdd.speed_dev) {
            iterator->moved = 1;
            iterator->nmea.speed_disturbed = HIGH;
        }
        else if(iterator->nmea.speed_current < s_conf->rdd.speed_ref - s_conf->rdd.speed_dev) {
            iterator->moved = 1;
            iterator->nmea.speed_disturbed = LOW;
        }
        else {
            iterator->nmea.speed_disturbed = SAFE;
        }
    }
}

void min_max_filter(void)
{
    struct client_table_entry* iterator;
    struct client_table_entry* safe;

    list_for_each_entry_safe(iterator, safe,&client_list->list, list) {
        if(iterator->nmea.lat_current > iterator->nmea.lat_high) {
            iterator->moved = 1;
            iterator->nmea.lat_disturbed = HIGH;
        }
        else if(iterator->nmea.lat_current < iterator->nmea.lat_low) {
            iterator->moved = 1;
            iterator->nmea.lat_disturbed = LOW;
        }
        else {
            iterator->nmea.lat_disturbed = SAFE;
        }

        if(iterator->nmea.lon_current > iterator->nmea.lon_high) {
            iterator->moved = 1;
            iterator->nmea.lon_disturbed = HIGH;
        }
        else if(iterator->nmea.lon_current < iterator->nmea.lon_low) {
            iterator->moved = 1;
            iterator->nmea.lon_disturbed = LOW;
        }
        else {
            iterator->nmea.lon_disturbed = SAFE;
        }

        if(iterator->nmea.alt_current > iterator->nmea.alt_high) {
            iterator->moved = 1;
            iterator->nmea.alt_disturbed = HIGH;
        }
        else if(iterator->nmea.alt_current < iterator->nmea.alt_low) {
            iterator->moved = 1;
            iterator->nmea.alt_disturbed = LOW;
        }
        else {
            iterator->nmea.alt_disturbed = SAFE;
        }

        if(iterator->nmea.speed_current > iterator->nmea.speed_high) {
            iterator->moved = 1;
            iterator->nmea.speed_disturbed = HIGH;
        }
        else if(iterator->nmea.speed_current < iterator->nmea.speed_low) {
            iterator->moved = 1;
            iterator->nmea.speed_disturbed = LOW;
        }
        else {
            iterator->nmea.speed_disturbed = SAFE;
        }
    }
}