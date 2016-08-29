#include "filters.h"

#define ALARM_MOVED "[ ALARM ] Client %d was moved!\n"
#define ALARM_RETURNED "[ ALARM ] Client %d has returned!\n"


/** @brief Checks if a sensor has been marked as moved
 *
 * Iterates through client_list and checks for clients marked
 * as moved. Raises alarm.
 *
 * @return Void
 */
static void min_max_result(void);

static void min_max_result(void)
{
    struct client_table_entry* client_list_iterate;
    struct client_table_entry* safe;

    list_for_each_entry_safe(client_list_iterate, safe,&client_list->list, list) {
        if(client_list_iterate->client_id > 0) {
            if(client_list_iterate->moved == 1) {
                client_list_iterate->was_moved = 1;
                client_list_iterate->moved = 0;
                t_print(ALARM_MOVED, client_list_iterate->client_id);
            } else {
                if(client_list_iterate->was_moved) {
                    client_list_iterate->was_moved = 0;
                    t_print(ALARM_RETURNED, client_list_iterate->client_id);
                }
            }
        }
    }
}

void min_max(void)
{
    struct client_table_entry* client_list_iterate;
    struct client_table_entry* safe;

    list_for_each_entry_safe(client_list_iterate, safe,&client_list->list, list) {
        if(client_list_iterate->nmea.lat_current > client_list_iterate->nmea.lat_high) {
            client_list_iterate->moved = 1;
            client_list_iterate->nmea.lat_disturbed = HIGH;
        }
        else if(client_list_iterate->nmea.lat_current < client_list_iterate->nmea.lat_low) {
            client_list_iterate->moved = 1;
            client_list_iterate->nmea.lat_disturbed = LOW;
        }
        else {
            client_list_iterate->nmea.lat_disturbed = SAFE;
        }

        if(client_list_iterate->nmea.lon_current > client_list_iterate->nmea.lon_high) {
            client_list_iterate->moved = 1;
            client_list_iterate->nmea.lon_disturbed = HIGH;
        }
        else if(client_list_iterate->nmea.lon_current < client_list_iterate->nmea.lon_low) {
            client_list_iterate->moved = 1;
            client_list_iterate->nmea.lon_disturbed = LOW;
        }
        else {
            client_list_iterate->nmea.lon_disturbed = SAFE;
        }

        if(client_list_iterate->nmea.alt_current > client_list_iterate->nmea.alt_high) {
            client_list_iterate->moved = 1;
            client_list_iterate->nmea.alt_disturbed = HIGH;
        }
        else if(client_list_iterate->nmea.alt_current < client_list_iterate->nmea.alt_low) {
            client_list_iterate->moved = 1;
            client_list_iterate->nmea.alt_disturbed = LOW;
        }
        else {
            client_list_iterate->nmea.alt_disturbed = SAFE;
        }

        if(client_list_iterate->nmea.speed_current > client_list_iterate->nmea.speed_high) {
            client_list_iterate->moved = 1;
            client_list_iterate->nmea.speed_disturbed = HIGH;
        }
        else if(client_list_iterate->nmea.speed_current < client_list_iterate->nmea.speed_low) {
            client_list_iterate->moved = 1;
            client_list_iterate->nmea.speed_disturbed = LOW;
        }
        else {
            client_list_iterate->nmea.speed_disturbed = SAFE;
        }
    }

    min_max_result();
}