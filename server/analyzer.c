#include "analyzer.h"

void check_result()
{
	struct client_table_entry* client_list_iterate;
    struct client_table_entry* safe;

	list_for_each_entry_safe(client_list_iterate, safe,&client_list->list, list) {
		if(client_list_iterate->moved == 1){
			t_print(ALARM, client_list_iterate->client_id);
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
	    }

	    if(client_list_iterate->nmea.lat_current < client_list_iterate->nmea.lat_low){
	        client_list_iterate->moved = 1;
	    }

	    if(client_list_iterate->nmea.lon_current > client_list_iterate->nmea.lon_high){
	        client_list_iterate->moved = 1;
	    }

	    if(client_list_iterate->nmea.lon_current < client_list_iterate->nmea.lon_low){
	        client_list_iterate->moved = 1;
	    }

	    if(client_list_iterate->nmea.alt_current > client_list_iterate->nmea.alt_high){
	        client_list_iterate->moved = 1;
	    }

	    if(client_list_iterate->nmea.alt_current < client_list_iterate->nmea.alt_low){
	        client_list_iterate->moved = 1;
	    }	
    }
}

/* Analyzes data from all the clients */
void analyze(){
	check_moved();
}