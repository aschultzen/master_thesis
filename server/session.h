#ifndef SESSION_H
#define SESSION_H

#include "sensor_server_commons.h"
#include "analyzer.h"

/* GENERAL */
#define HORIZONTAL_BAR "=============================================================\n"
#define CLIENT_TABLE_LABEL "CLIENT TABLE\n"
#define SERVER_TABLE_LABEL "SERVER DATA\n"
#define NEW_LINE "\n"
#define PRINT_LOCATION_HEADER "      CURRENT        MIN          MAX\n"

/* ERRORS */
#define ERROR_NO_CLIENT "ERROR: NO SUCH CLIENT\n"

/* HELP */
#define HELP	"\n"\
				"   COMMAND     PARAMETER                      DESCRIPTION\n"\
 				"------------ ------------ -------------------------------------------------\n"\
  				"  IDENTIFY	INTEGER	   PARAM is set as the connected clients ID (you)\n"\
  				"			   ID > 0 is treated as sensor, ID < 0 as monitor\n"\
  				"---------------------------------------------------------------------------\n"\
  				"  EXIT		NONE	   Disconnects\n"\
  				"---------------------------------------------------------------------------\n"\
  				"  DISCONNECT 	NONE	   See EXIT\n"\
  				"---------------------------------------------------------------------------\n"\
  				"  PRINTCLIENTS	NONE	   Prints an overview of connected clients\n"\
  				"---------------------------------------------------------------------------\n"\
  				"  PRINTSERVER	NONE	   Prints server state and config\n"\
  				"---------------------------------------------------------------------------\n"\
  				"  GETTIME	INTEGER	   Prints time solved from <CLIENT ID>\n"\
  				"---------------------------------------------------------------------------\n"\


void setup_session(int session_fd, struct client_table_entry *new_client);

#endif /* !SESSION_H */