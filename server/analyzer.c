#include "analyzer.h"

static void check_moved_result()
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

static void check_moved()
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
    }

    check_moved_result();
}

/* Analyzes data from all the clients */
void analyze(){
	check_moved();
	//check_telemetry();
}