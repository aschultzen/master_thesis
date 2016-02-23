#include "session.h"



/* Sends a formatted string containing info about connected clients */
static void print_clients(struct client_table_entry *cte){
    char buffer [1000];
    int snprintf_status = 0;
    char *c_type = "SENSOR";

    struct client_table_entry* client_list_iterate;
    s_write(cte, "\n", 1);
    s_write(cte, "CLIENT TABLE\n", 13);
    s_write(cte, "=============================================================\n", 63);
    list_for_each_entry(client_list_iterate,&client_list->list, list) {
        
        if(client_list_iterate->client_type == MONITOR){
            c_type = "MONITOR";
        }else{
           c_type = "SENSOR"; 
        }

        snprintf_status = snprintf( buffer, 1000, "PID: %d, IP:%s, TOUCH: %d, TYPE: %s, ID: %d\n", 
        client_list_iterate->pid, 
        client_list_iterate->ip, 
        (int)difftime(time(NULL),client_list_iterate->timestamp), 
        c_type,
        client_list_iterate->client_id);
        s_write(cte, buffer, snprintf_status);
    }
    s_write(cte, "=============================================================\n", 63);
}

/* Sends a formatted string containing server info */
static void print_server_data(struct client_table_entry *cte, struct server_data *s_data){
    char buffer [1000];
    int snprintf_status = 0;

    struct tm *loctime;
    loctime = localtime (&s_data->started);

    s_write(cte, "\n", 1);
    s_write(cte, "SERVER DATA\n", 12);
    s_write(cte, "=============================================================\n", 63);
        
    snprintf_status = snprintf( buffer, 1000, 
        "PID: %d\nNumber of clients: %d\nMax clients: %d\nStarted: %sVersion: %s\n", 
        s_data->pid, 
        s_data->number_of_clients,
        s_data->max_clients,
        asctime (loctime), 
        s_data->version);

    s_write(cte, buffer, snprintf_status);
    s_write(cte, "=============================================================\n", 63);
}

/* Responds to client action */
static int respond(struct client_table_entry *cte)
{
    s_write(cte, ">", 1);
    int read_status = s_read(cte); /* Blocking */
    if(read_status == -1) {
        t_print("Read failed or interrupted!\n");
        return -1;
    }

    int parse_status = parse_input(cte);

    if(parse_status == -1) {
        s_write(cte, ERROR_ILLEGAL_MESSAGE_SIZE,
                sizeof(ERROR_ILLEGAL_MESSAGE_SIZE));
    }
    if(parse_status == 0) {
        s_write(cte, ERROR_ILLEGAL_COMMAND,
                sizeof(ERROR_ILLEGAL_COMMAND));
    }
    if(parse_status == 1) {
        if(cte->cm.code == CODE_DISCONNECT) {
            t_print("Client %d requested DISCONNECT.\n", cte->client_id);
            s_write(cte, PROTOCOL_OK, sizeof(PROTOCOL_OK));
            s_write(cte, PROTOCOL_GOODBYE, sizeof(PROTOCOL_GOODBYE));
            return -1;
        }
        if(cte->cm.code == CODE_IDENTIFY) {
            int id = 0;

            if(sscanf(cte->cm.parameter, "%d", &id) == -1) {
                s_write(cte, ERROR_ILLEGAL_COMMAND, sizeof(ERROR_ILLEGAL_COMMAND));
                return 0;
            }

            struct client_table_entry* client_list_iterate;
            list_for_each_entry(client_list_iterate, &client_list->list, list) {
                if(client_list_iterate->client_id == id) {
                    cte->client_id = 0;
                    t_print("[%s] bounced! ID %d already in use.\n", cte->ip,id);
                    s_write(cte, "ID in use!\n", 11);
                    return -1;
                }
            }

            if(id < 0){
                cte->client_type = MONITOR;
            }
            else{
                cte->client_type = SENSOR;
            }
            cte->client_id = id;
            t_print("[%s] ID set to: %d\n", cte->ip,cte->client_id);
            s_write(cte, PROTOCOL_OK, sizeof(PROTOCOL_OK));
            return 0;
        }

        if(cte->client_id == 0) {
            s_write(cte, ERROR_NO_ID, sizeof(ERROR_NO_ID));
            return 0;
        }

        if(cte->cm.code == CODE_PRINTCLIENTS) {
            print_clients(cte);
        }

        if(cte->cm.code == CODE_PRINTSERVER) {
            print_server_data(cte, s_data);
        }
    }
    return 0;
}

/* Setups the clients structure and initializes data */
void setup_session(int session_fd, struct client_table_entry *new_client)
{
    /* Setting the IP adress */
    char ip[INET_ADDRSTRLEN];
    get_ip_str(session_fd, ip);

    /* Setting the PID */
    new_client->pid = getpid();
    new_client->timestamp = time(NULL);
    strncpy(new_client->ip, ip, INET_ADDRSTRLEN);

    /* Initializing structure */
    new_client->heartbeat_timeout.tv_sec = CLIENT_TIMEOUT + 1000; //remove 1000 when testing is done!
    new_client->heartbeat_timeout.tv_usec = 0;
    new_client->client_id = 0;
    new_client->session_fd = session_fd;

    memset(&new_client->iobuffer, 0, IO_BUFFER_SIZE*sizeof(char));
    memset(&new_client->cm.parameter, 0, MAX_PARAMETER_SIZE*sizeof(char));

    /* Setting socket timeout to default value */
    /* This doesn't always work for some reason, race condition? :/ */
    if (setsockopt (new_client->session_fd, SOL_SOCKET,
                    SO_RCVTIMEO, (char *)&new_client->heartbeat_timeout, sizeof(struct timeval)) < 0) {
        die(36,"setsockopt failed\n");
    }

    /*
    * Entering child process main loop
    * (Outer) breaks if server closes.
    * (Inner) Breaks (disconnects the client) if
    * respond < 0
    */
    while(!done) {
        if(respond(new_client) < 0) {
            break;
        }
    }
}