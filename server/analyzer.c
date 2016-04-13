#include "analyzer.h"

#define ALARM_MOVED "[ ALARM ] Client %d was moved!\n"
#define ALARM_RETURNED "[ ALARM ] Client %d has returned!\n"



/** @brief Checks if a sensor has been marked as moved
 *
 * Iterates through client_list and checks for clients marked
 * as moved. Raises alarm.
 *
 * @return Void
 */
static void check_moved_result(void);

/** @brief Checks for any "moving" SENSORS
 *
 * Iterates through client_list
 * and checks if anyone's current position (LAT, LON, ALT, SPEED)
 * is within the ranges recorded during warm-up. If it is, the 
 * dimension's disturbed value is set to SAFE (no change), 
 * LOW (lower then the lowest recorded) or HIGH (higher than recorded).
 * Unless SAFE, moved is set to 1. The moved variable is used by 
 * check_moved_result() to raise an alarm. 
 *
 * @return Void
 */
static void check_moved(void);


static void check_moved_result(void)
{
	struct client_table_entry* client_list_iterate;
    struct client_table_entry* safe;

	list_for_each_entry_safe(client_list_iterate, safe,&client_list->list, list) {
		if(client_list_iterate->client_id > 0){
			if(client_list_iterate->moved == 1){
				client_list_iterate->was_moved = 1;
				client_list_iterate->moved = 0;
				t_print(ALARM_MOVED, client_list_iterate->client_id);
			}else{
				if(client_list_iterate->was_moved){
					client_list_iterate->was_moved = 0;
					t_print(ALARM_RETURNED, client_list_iterate->client_id);
				}
			}
		}
    }	
}

/** @brief Checks for any "moving" SENSORS
 *
 * Iterates through client_list
 * and checks if anyone's current position (LAT, LON, ALT, SPEED)
 * is within the ranges recorded during warm-up. If it is, the 
 * dimension's disturbed value is set to SAFE (no change), 
 * LOW (lower then the lowest recorded) or HIGH (higher than recorded).
 * Unless SAFE, moved is set to 1. The moved variable is used by 
 * check_moved_result() to raise an alarm. 
 *
 * @return Void
 */
static void check_moved(void)
{
	struct client_table_entry* client_list_iterate;
    struct client_table_entry* safe;

	list_for_each_entry_safe(client_list_iterate, safe,&client_list->list, list) {
	    if(client_list_iterate->nmea.lat_current > client_list_iterate->nmea.lat_high){
	        client_list_iterate->moved = 1;
	        client_list_iterate->nmea.lat_disturbed = HIGH;
	    }
	    else if(client_list_iterate->nmea.lat_current < client_list_iterate->nmea.lat_low){
	        client_list_iterate->moved = 1;
	        client_list_iterate->nmea.lat_disturbed = LOW;
	    }
	    else{
			client_list_iterate->nmea.lat_disturbed = SAFE;
	    }

	    if(client_list_iterate->nmea.lon_current > client_list_iterate->nmea.lon_high){
	        client_list_iterate->moved = 1;
	        client_list_iterate->nmea.lon_disturbed = HIGH;
	    }
	    else if(client_list_iterate->nmea.lon_current < client_list_iterate->nmea.lon_low){
	        client_list_iterate->moved = 1;
	        client_list_iterate->nmea.lon_disturbed = LOW;
	    }
	    else{
	    	client_list_iterate->nmea.lon_disturbed = SAFE;
	    }

	    if(client_list_iterate->nmea.alt_current > client_list_iterate->nmea.alt_high){
	        client_list_iterate->moved = 1;
	        client_list_iterate->nmea.alt_disturbed = HIGH;
	    }
	    else if(client_list_iterate->nmea.alt_current < client_list_iterate->nmea.alt_low){
	        client_list_iterate->moved = 1;
	        client_list_iterate->nmea.alt_disturbed = LOW;
	    }
	    else{
	    	client_list_iterate->nmea.alt_disturbed = SAFE;
	    }

	    if(client_list_iterate->nmea.speed_current > client_list_iterate->nmea.speed_high){
	        client_list_iterate->moved = 1;
	        client_list_iterate->nmea.speed_disturbed = HIGH;
	    }
	    else if(client_list_iterate->nmea.speed_current < client_list_iterate->nmea.speed_low){
	        client_list_iterate->moved = 1;
	        client_list_iterate->nmea.speed_disturbed = LOW;
	    }
	    else{
	    	client_list_iterate->nmea.speed_disturbed = SAFE;
	    }		
    }

    check_moved_result();
}

/* Analyzes data from all the clients */
void analyze(void){
	check_moved();
}