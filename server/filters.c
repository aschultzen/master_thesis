#include "filters.h"

#define ALARM_RDF "[ ALARM ] Sensor %d triggered KRL filter!\n"
#define ALARM_RDF_RETURNED "[ ALARM ] Sensor %d cleared KRL filter!\n"

#define LOG_FILE "server_log"
#define LOG_STRING_LENGTH 100
#define MJD_LENGTH 15

/** @brief Checks for any "moving" SENSORS
*
* Checks solved position against known position.
* Known position loaded from the config file.
* @return Void
*/
static void krl_filter(void);

/** @brief Checks if a sensor has been marked as moved
 *
 * Iterates through client_list and checks for clients marked
 * as moved. Raises alarm.
 *
 * @return Void
 */
static void raise_alarm(void);

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
    return log_to_file(s_conf->log_path, log_string, 2);
}


void raise_alarm(void)
{
    struct client_table_entry* iterator;
    struct client_table_entry* safe;

    list_for_each_entry_safe(iterator, safe,&client_list->list, list) {
        if(iterator->client_id > 0) {
            /* Checking krl_filter */
            if(iterator->fs.rdf.moved == 1) {
                iterator->fs.rdf.was_moved = 1;
                iterator->fs.rdf.moved = 0;
                if(s_conf->logging) {
                    log_alarm(iterator->client_id, ALARM_RDF);
                }
            } else {
                if(iterator->fs.rdf.was_moved) {
                    iterator->fs.rdf.was_moved = 0;
                    if(s_conf->logging) {
                        log_alarm(iterator->client_id, ALARM_RDF_RETURNED);
                    }
                }
            }
        }
    }
}

void krl_filter(void)
{
    struct client_table_entry* iterator;
    struct client_table_entry* safe;

    list_for_each_entry_safe(iterator, safe,&client_list->list, list) {

        if(iterator->nmea.lat_current > iterator->fs.rdf.rdd.lat_ref +
                iterator->fs.rdf.rdd.lat_dev) {
            iterator->fs.rdf.moved = 1;
            iterator->fs.rdf.dv.lat_disturbed = HIGH;
        } else if(iterator->nmea.lat_current < iterator->fs.rdf.rdd.lat_ref -
                  iterator->fs.rdf.rdd.lat_dev) {
            iterator->fs.rdf.moved = 1;
            iterator->fs.rdf.dv.lat_disturbed = LOW;
        } else {
            iterator->fs.rdf.dv.lat_disturbed = SAFE;
        }

        if(iterator->nmea.alt_current > iterator->fs.rdf.rdd.alt_ref +
                iterator->fs.rdf.rdd.alt_dev) {
            iterator->fs.rdf.moved = 1;
            iterator->fs.rdf.dv.alt_disturbed = HIGH;
        } else if(iterator->nmea.alt_current < iterator->fs.rdf.rdd.alt_ref -
                  iterator->fs.rdf.rdd.alt_dev) {
            iterator->fs.rdf.moved = 1;
            iterator->fs.rdf.dv.alt_disturbed = LOW;
        } else {
            iterator->fs.rdf.dv.alt_disturbed = SAFE;
        }

        if(iterator->nmea.lon_current > iterator->fs.rdf.rdd.lon_ref +
                iterator->fs.rdf.rdd.lon_dev) {
            iterator->fs.rdf.moved = 1;
            iterator->fs.rdf.dv.lon_disturbed = HIGH;
        } else if(iterator->nmea.lon_current < iterator->fs.rdf.rdd.lon_ref -
                  iterator->fs.rdf.rdd.lon_dev) {
            iterator->fs.rdf.moved = 1;
            iterator->fs.rdf.dv.lon_disturbed = LOW;
        } else {
            iterator->fs.rdf.dv.lon_disturbed = SAFE;
        }

        if(iterator->nmea.speed_current > iterator->fs.rdf.rdd.speed_ref +
                iterator->fs.rdf.rdd.speed_dev) {
            iterator->fs.rdf.moved = 1;
            iterator->fs.rdf.dv.speed_disturbed = HIGH;
        } else if(iterator->nmea.speed_current < iterator->fs.rdf.rdd.speed_ref -
                  iterator->fs.rdf.rdd.speed_dev) {
            iterator->fs.rdf.moved = 1;
            iterator->fs.rdf.dv.speed_disturbed = LOW;
        } else {
            iterator->fs.rdf.dv.speed_disturbed = SAFE;
        }
    }
}

void apply_filters()
{
    krl_filter();
    raise_alarm();
}