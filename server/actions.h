#ifndef ACTIONS_H
#define ACTIONS_H

#include "sensor_server_commons.h"
#include "sensor_server.h"

/* GENERAL */
#define HORIZONTAL_BAR "=============================================================\n"
#define CLIENT_TABLE_LABEL "CLIENT TABLE\n"
#define SERVER_TABLE_LABEL "SERVER DATA\n"
#define NEW_LINE "\n"
#define PRINT_LOCATION_HEADER "      CURRENT        MIN          MAX          AVG\n"
#define PRINT_AVG_DIFF_HEADER "ID     LAT        LON       ALT       SPEED\n"
#define DATADUMP_EXTENSION ".dump"

/* ERRORS */
#define ERROR_NO_CLIENT "ERROR: NO SUCH CLIENT\n"
#define ERROR_APPEND_TOO_LONG "ERROR: TEXT TO APPEND TOO LONG\n"
#define ERROR_NO_SENSORS_CONNECTED "NO SENSORS CONNECTED\n"
#define ERROR_FCLOSE "dumpdata(): Failed to close file, out of space?\n"
#define ERROR_FWRITE "dumpdata(): Failed to write to file, aborting.\n"
#define ERROR_FOPEN "dumpdata(): Failed to open file, aborting.\n"

/* HELP */
#define HELP  "\n"\
          "   COMMAND     PARAMETER                      DESCRIPTION\n"\
          "------------ ------------ -------------------------------------------------\n"\
          "  IDENTIFY INTEGER    PARAM is set as the connected clients ID (you)\n"\
          "        ID > 0 is treated as sensor, ID < 0 as monitor\n"\
          "---------------------------------------------------------------------------\n"\
          "  EXIT   NONE     Disconnects\n"\
          "---------------------------------------------------------------------------\n"\
          "  DISCONNECT   NONE     See EXIT\n"\
          "---------------------------------------------------------------------------\n"\
          "  PRINTCLIENTS NONE     Prints an overview of connected clients\n"\
          "---------------------------------------------------------------------------\n"\
          "  PRINTSERVER  NONE     Prints server state and config\n"\
          "---------------------------------------------------------------------------\n"\
          "  PRINTTIME  INTEGER    Prints time solved from <CLIENT ID>\n"\
          "---------------------------------------------------------------------------\n"\
          "  DUMPDATA    INTEGER    Dumps all location related data to file\n"\
          "---------------------------------------------------------------------------\n"\
          "  PRINTAVGDIFF  NONE    Prints all average diffs for all clients\n"\
          "---------------------------------------------------------------------------\n"\

/* SIZES */
#define DUMPDATA_TIME_SIZE 13
#define MAX_APPEND_LENGTH 20
#define MAX_FILENAME_SIZE 30

void kick_client(struct transmission_s *tsm, struct client_table_entry* candidate);
void print_client_time(struct transmission_s *tsm, struct client_table_entry* candidate);
void print_clients(struct client_table_entry *cte);
void print_server_data(struct client_table_entry *cte, struct server_data *s_data);
void print_help(struct transmission_s *tsm);
void print_location(struct transmission_s *tsm, struct client_table_entry* candidate);
void print_avg_diff(struct client_table_entry *cte);
void restart_warmup(struct client_table_entry* target, struct transmission_s *tsm);
int dumpdata(struct client_table_entry* target, struct transmission_s *tsm, char *filename_append);

#endif /* !ACTIONS_H */