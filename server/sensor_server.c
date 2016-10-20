#include "sensor_server.h"

/* VERSION */
#define PROGRAM_VERSION "0.8c"

/* ERRORS */
#define ERROR_MAX_CLIENTS_REACHED "CONNECTION REJECTED: MAXIMUM NUMBER OF CLIENTS REACHED\n"
#define ERROR_CONFIG_LOAD_FAILED "CONFIG LOAD FAILED: CONFIG FILE CORRUPTED\n"
#define ERROR_SEMAPHORE_CREATION_FAILED "SEMAPHORE CREATION FAILED\n"
#define ERROR_SOCKET_OPEN_FAILED "ERROR: FAILED TO OPEN SOCKET\n"
#define ERROR_SOCKET_BINDING "ERROR: FAILED TO BIND ON %d\n"
#define ERROR_CONNECTION_ACCEPT "ERROR: FAILED TO ACCEPT CONNECTION (%d)\n"
#define ERROR_FAILED_FORK "ERROR: FORK FAILED (%d)\n"
#define ERROR_MISSING_PARAMS "MISSING PARAMETERS!\n"

/* GENERAL STRINGS */
#define PROCESS_REAPED "Process %d reaped. Status: %d Signum: %d\n"
#define SIGTERM_RECEIVED "[%d] SIGTERM received!\n"
#define SIGINT_RECEIVED "[%d] SIGINT received!\n"
#define STOPPING_SERVER "Stopping server...\n"
#define CONFIG_LOADED "Config loaded!\n"
#define SERVER_RUNNING "Server is running. Accepting connections.\n"
#define WAITING_FOR_CONNECTIONS "Waiting for connections...\n"
#define CON_ACCEPTED "Connection accepted\n"
#define CLIENT_DISCONNECTED "Client [%d] at [%s] disconnected\n"
#define SERVER_STOPPED "Server STOPPED!\n"
#define SERVER_STARTING "Sensor server starting...\n"
#define CLIENT_KICKED "Client was kicked\n"

/* USAGE() STRINGS */
#define USAGE_DESCRIPTION "Required argument:\n\t -p <PORT NUMBER>\n\n"
#define USAGE_PROGRAM_INTRO "Sensor_server: Server part of GPS Jamming/Spoofing system\n\n"
#define USAGE_USAGE "Usage: %s [ARGS]\n\n"

/* CONFIG CONSTANTS*/
#define CONFIG_FILE_PATH "config.ini"
#define CONFIG_SERVER_MAX_CONNECTIONS "max_clients:"
#define CONFIG_SERVER_WARM_UP "warm_up:"
#define CONFIG_SERVER_HUMANLY_READABLE "humanly_readable_dumpdata:"
#define CONFIG_CSAC_PATH "csac_serial_interface:"
#define CONFIG_LOGGING "logging:"
#define CONFIG_LOG_PATH "log_path:"
#define CONFIG_CSAC_LOG_PATH "csac_log_path:"
#define CONFIG_CSAC_LOGGING "csac_logging:"
#define SERVER_CONFIG_ENTRIES 8

/* Server data and stats */
struct server_data *s_data;

/* Shared synchro elements */
struct server_synchro *s_synch;

/* Used by sig handlers */
//volatile sig_atomic_t done;

/* Pointer to shared memory containing the client list */
struct client_table_entry *client_list;

/* Pointer to client list map */
struct client_table_entry **client_list_map;

/* Pointer to shared memory containing config */
struct server_config *s_conf;

/* Pointer to shared CSAC_filter data */
struct csac_filter_data *cfd;

static void remove_client_by_pid(pid_t pid);
void remove_client_by_id(int id);
static struct client_table_entry* create_client(struct client_table_entry* ptr);
static void handle_sigchld(int signum);
static void handle_sig(int signum);
static void initialize_config(struct config_map_entry *conf_map,
                              struct server_config *s_conf);
static int start_server(int port_number);
static int usage(char *argv[]);
static void setup_session(int session_fd, struct client_table_entry *new_client);
static int release_mem_piece(struct client_table_entry* release_me);

int set_timeout(struct client_table_entry *target,
                       struct timeval h_timeout)
{
    /* setsockopt return -1 on error and 0 on success */
    target->heartbeat_timeout = h_timeout;
    if (setsockopt (target->transmission.session_fd, SOL_SOCKET,
                    SO_RCVTIMEO, (char *)&target->heartbeat_timeout, sizeof(struct timeval)) < 0) {
        t_print("an error: %s\n", strerror(errno));
        return 0;
    }
    return 1;
}

/* Prints a formatted string containing server info to monitor */
void print_server_data(struct client_table_entry *monitor)
{
    char buffer [1000];
    int snprintf_status = 0;
    struct tm *loctime_started;
    loctime_started = localtime (&s_data->started);

    s_write(&(monitor->transmission), SERVER_TABLE_LABEL,
            sizeof(SERVER_TABLE_LABEL));
    s_write(&(monitor->transmission), HORIZONTAL_BAR, sizeof(HORIZONTAL_BAR));

    snprintf_status = snprintf( buffer, 1000,
                                "PID: %d\n" \
                                "Number of clients: %d\n" \
                                "Number of sensors: %d\n" \
                                "Max clients: %d\n" \
                                "Sensor Warm-up time: %ds\n" \
                                "Dump humanly readable data: %d\n" \
                                "Started: %s" \
                                "Version: %s\n",
                                s_data->pid,
                                s_data->number_of_clients,
                                s_data->number_of_sensors,
                                s_conf->max_clients,
                                s_conf->warm_up_seconds,
                                s_conf->human_readable_dumpdata,
                                asctime (loctime_started),
                                s_data->version);

    s_write(&(monitor->transmission), buffer, snprintf_status);
    s_write(&(monitor->transmission), HORIZONTAL_BAR, sizeof(HORIZONTAL_BAR));
}

struct client_table_entry* get_client_by_id(int id)
{
    struct client_table_entry* cli;
    struct client_table_entry* temp;
    int found = 0;

    sem_wait(&(s_synch->client_list_sem));
    list_for_each_entry_safe(cli, temp, &client_list->list, list) {
        if(cli->client_id == id) {
            found = 1;
            break;
        }
    }
    sem_post(&(s_synch->client_list_sem));
    if(found) {
        return cli;
    } else {
        return NULL;
    }
}

/* Removes a client with the given PID */
static void remove_client_by_pid(pid_t pid)
{
    struct client_table_entry* cli;
    struct client_table_entry* temp_remove;

    sem_wait(&(s_synch->client_list_sem));
    list_for_each_entry_safe(cli, temp_remove,&client_list->list,
                             list) {
        if(cli->pid == pid) {
            /* Decrementing sensor count */
            if(cli->client_id > 0) {
                s_data->number_of_sensors--;
            }
            t_print(CLIENT_DISCONNECTED, cli->client_id ,cli->ip);
            list_del(&cli->list);
            release_mem_piece(cli);
        }
    }
    /* Decrementing total client count */
    s_data->number_of_clients--;
    sem_post(&(s_synch->client_list_sem));
}

/* Removes a client with the given ID */
void remove_client_by_id(int id)
{
    struct client_table_entry* cli;
    struct client_table_entry* temp_remove;

    sem_wait(&(s_synch->client_list_sem));
    list_for_each_entry_safe(cli, temp_remove,&client_list->list,
                             list) {
        if(cli->client_id == id) {
            list_del(&cli->list);
            release_mem_piece(cli);
            s_data->number_of_clients--;
        }
    }
    sem_post(&(s_synch->client_list_sem));
}

static int release_mem_piece(struct client_table_entry* release_me)
{
    int i;
    for(i = 1; i < s_conf->max_clients; i++){
        if(client_list_map[i] == NULL){
            client_list_map[i] = release_me;
            return 1;
        }
        i++;
    }
    return 0;
}

static struct client_table_entry* get_mem_piece()
{
    int i;
    for(i = 1; i < s_conf->max_clients; i++){
        if(client_list_map[i] != NULL){
            struct client_table_entry *tmp = client_list_map[i];
            client_list_map[i] = NULL;
            return tmp;
        }
        i++;
    }
    return NULL;
}

/* Creates an entry in the client list structure and returns a pointer to it*/
static struct client_table_entry* create_client(struct client_table_entry* ptr)
{
    sem_wait(&(s_synch->client_list_sem));
    s_data->number_of_clients++;
    struct client_table_entry* tmp;
    tmp = get_mem_piece();
    list_add_tail( &(tmp->list), &(ptr->list) );
    sem_post(&(s_synch->client_list_sem));
    return tmp;
}

/* SIGCHLD Handler */
static void handle_sigchld(int signum)
{
    pid_t pid;
    int   status;
    while ((pid = waitpid(-1, &status, WNOHANG)) != -1) {
        if(pid == 0) {
            break;
        }

        if(pid > 0) {
            remove_client_by_pid(pid);
        }
    }
}

/* SIGTERM/INT Handler */
static void handle_sig(int signum)
{
    if(signum == 15) {
        t_print(SIGTERM_RECEIVED, getpid());
    }
    if(signum == 2) {
        t_print(SIGINT_RECEIVED, getpid());
    }
    t_print(STOPPING_SERVER, getpid());
    s_synch->done = 1;
}

/* Setting up the config structure specific for the server */
static void initialize_config(struct config_map_entry *conf_map,
                              struct server_config *s_conf)
{
    conf_map[0].entry_name = CONFIG_SERVER_MAX_CONNECTIONS;
    conf_map[0].modifier = FORMAT_INT;
    conf_map[0].destination = &s_conf->max_clients;

    conf_map[1].entry_name = CONFIG_SERVER_WARM_UP;
    conf_map[1].modifier = FORMAT_INT;
    conf_map[1].destination = &s_conf->warm_up_seconds;

    conf_map[2].entry_name = CONFIG_SERVER_HUMANLY_READABLE;
    conf_map[2].modifier = FORMAT_INT;
    conf_map[2].destination = &s_conf->human_readable_dumpdata;

    conf_map[3].entry_name = CONFIG_CSAC_PATH;
    conf_map[3].modifier = FORMAT_STRING;
    conf_map[3].destination = &s_conf->csac_path;

    conf_map[4].entry_name = CONFIG_LOGGING;
    conf_map[4].modifier = FORMAT_INT;
    conf_map[4].destination = &s_conf->logging;

    conf_map[5].entry_name = CONFIG_LOG_PATH;
    conf_map[5].modifier = FORMAT_STRING;
    conf_map[5].destination = &s_conf->log_path;

    conf_map[6].entry_name = CONFIG_CSAC_LOG_PATH;
    conf_map[6].modifier = FORMAT_STRING;
    conf_map[6].destination = &s_conf->csac_log_path;

    conf_map[7].entry_name = CONFIG_CSAC_LOGGING;
    conf_map[7].modifier = FORMAT_INT;
    conf_map[7].destination = &s_conf->csac_logging;
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

    /* Initializing structure, zeroing just to be sure */
    new_client->client_id = 0;
    new_client->transmission.session_fd = session_fd;

    /* Zeroing out filters */
    new_client->fs.rdf.moved = 0;
    new_client->fs.rdf.was_moved = 0;

    new_client->marked_for_kick = 0;
    new_client->ready = 0;

    /* Setting timeout */
    struct timeval timeout = {UNIDENTIFIED_TIMEOUT, 0};
    if(!set_timeout(new_client, timeout)) {
        t_print("Failed to set timeout for client\n");
    }

    memset(&new_client->transmission.iobuffer, '0', IO_BUFFER_SIZE*sizeof(char));
    memset(&new_client->cm.parameter, '0', MAX_PARAMETER_SIZE*sizeof(char));

    /*
    * Entering child process main loop
    * (Outer) breaks if server closes.
    * (Inner) Breaks (disconnects the client) if
    * respond < 0
    */
    while(!s_synch->done) {
        if(!respond(new_client)) {
            break;
        }
    }
}

/*
* Main loop for the server.
* Forks everytime a client connects and calls setup_session()
*/
static int start_server(int port_number)
{
    /* Initializing variables */
    int server_sockfd;
    struct sockaddr_in serv_addr;
    struct config_map_entry conf_map[SERVER_CONFIG_ENTRIES];

    /* Initializing config structure */
    s_conf = mmap(NULL, sizeof(struct server_config), PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    initialize_config(conf_map, s_conf);

    /* Loading config */
    int load_config_status = load_config(conf_map, CONFIG_FILE_PATH,
                                         SERVER_CONFIG_ENTRIES);

    /* Falling back to default if load_config fails */
    if(load_config_status) {
        t_print(CONFIG_LOADED);
        client_list = mmap(NULL,
                           ( (s_conf->max_clients + 1) * sizeof(struct client_table_entry)),
                           PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
        if(client_list == MAP_FAILED){
            t_print("Failed to allocate memory for the client list!\n");
        }
    } else {
        t_print(ERROR_CONFIG_LOAD_FAILED);
        exit(0);
    }

    client_list_map = malloc((s_conf->max_clients + 1) 
        * sizeof(struct client_table_entry*));
    int i;

    /* Skip the first entry for some reason */
    for(i = 1; i < s_conf->max_clients; i++){
        client_list_map[i] = client_list + i;
    }

    INIT_LIST_HEAD(&client_list->list);

    /* Create and initialize shared memory for server data */
    s_data = mmap(NULL, sizeof(struct server_data), PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(s_data == MAP_FAILED){
            t_print("Failed to allocate memory for the server data!\n");
    }
    
    bcopy(PROGRAM_VERSION, s_data->version,4);
    s_data->pid = getpid();
    s_data->started = time(NULL);

    /* Init shared semaphores and sync elements */
    s_synch = mmap(NULL, sizeof(struct server_synchro), PROT_READ | PROT_WRITE,
                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if(s_synch == MAP_FAILED){
            t_print("Failed to allocate memory for the semaphores!\n");
    }

    sem_init(&(s_synch->ready_sem), 1, 1);
    sem_init(&(s_synch->client_list_sem), 1, 1);
    sem_init(&(s_synch->csac_sem), 1, 1);

    /* Init pointer to shared CSAC_filter data */
    cfd = mmap(NULL, sizeof(struct csac_filter_data), PROT_READ | PROT_WRITE,
               MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if(cfd == MAP_FAILED){
            t_print("Failed to allocate memory for the CSAC filter data!\n");
    }

    if( &(s_synch->ready_sem) == SEM_FAILED
            || &(s_synch->client_list_sem) == SEM_FAILED) {
        t_print(ERROR_SEMAPHORE_CREATION_FAILED);
        sem_close(&(s_synch->ready_sem));
        sem_close(&(s_synch->client_list_sem));
        sem_close(&(s_synch->csac_sem));
        exit(1);
    }

    
    pid_t f_pid;
    f_pid = fork();
    if(f_pid == 0) {
        t_print("Forked out CSAC filter [%d]\n", getpid());
        start_csac_filter(cfd);
        _exit(0);
    }

    /* Waiting for filter to start */
    sleep(1);
    if(s_synch->done)
        return 1;

    /* Registering the SIGINT handler */
    struct sigaction sigint_action;
    memset(&sigint_action, 0, sizeof(struct sigaction));
    sigint_action.sa_handler = handle_sig;
    sigaction(SIGINT, &sigint_action, NULL);
    if (sigaction(SIGCHLD, &sigint_action, 0) == -1) {
        perror(0);
        exit(1);
    }

    /* Registering the SIGTERM handler */
    struct sigaction sigterm_action;
    memset(&sigterm_action, 0, sizeof(struct sigaction));
    sigterm_action.sa_handler = handle_sig;
    sigaction(SIGTERM, &sigterm_action, NULL);
    if (sigaction(SIGCHLD, &sigterm_action, 0) == -1) {
        perror(0);
        exit(1);
    }

    /* Registering the SIGCHLD handler */
    struct sigaction child_action;
    child_action.sa_handler = &handle_sigchld;
    sigemptyset(&child_action.sa_mask);
    child_action.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &child_action, 0) == -1) {
        perror(0);
        exit(1);
    }

    /* Initialize socket */
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0) {
        die(62,ERROR_SOCKET_OPEN_FAILED);
    }

    /*
    * Initializing the server address struct:
    * AF_INET = IPV4 Internet protocol
    * INADDR_ANY = Accept connections to all IPs of the machine
    * htons(port_number) = Endianess: network to host long(port number).
    */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port_number);

    /*
    * Assigns the address (serv_addr) to the socket
    * referred to by server_sockfd.
    */
    if (bind(server_sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0) {
        t_print(ERROR_SOCKET_BINDING, port_number);
        exit(1);
    }

    /* Marking the connection for listening*/
    listen(server_sockfd,SOMAXCONN);

    int session_fd = 0;
    t_print(SERVER_RUNNING);
    while (!s_synch->done) {
        fprintf(stderr, "Server done = %d\n", s_synch->done);
        t_print(WAITING_FOR_CONNECTIONS);
        session_fd = accept(server_sockfd,0,0);
        if (session_fd==-1) {
            if (errno==EINTR) continue;
            t_print(ERROR_CONNECTION_ACCEPT,errno);
        }
        if(s_data->number_of_clients == s_conf->max_clients) {
            write(session_fd, ERROR_MAX_CLIENTS_REACHED, sizeof(ERROR_MAX_CLIENTS_REACHED));
            close(session_fd);
        } else {
            struct client_table_entry *new_client = create_client(client_list);
            pid_t pid=fork();
            if (pid==-1) {
                t_print(ERROR_FAILED_FORK, errno);
                /* WHAT HAPPENS WITH THE LIST WHEN FORK FAILS? DEAL WITH IT.*/
            } else if (pid==0) {
                close(server_sockfd);
                setup_session(session_fd, new_client);
                close(session_fd);
                if(new_client->marked_for_kick) {
                    t_print(CLIENT_KICKED, getpid());
                }
                _exit(0);
            } else {
                t_print(CON_ACCEPTED);
                close(session_fd);
            }
        }
    }

    /* Destroying semaphores */
    sem_destroy(&(s_synch->csac_sem));
    sem_destroy(&(s_synch->ready_sem));
    sem_destroy(&(s_synch->client_list_sem));

    /* Freeing */
    munmap(client_list, sizeof(struct client_table_entry));
    munmap(s_data, sizeof(struct server_data));
    munmap(cfd, sizeof(struct csac_filter_data));
    munmap(s_synch, sizeof(struct server_synchro));
    free(client_list_map);

    /* Closing server FD */
    close(server_sockfd);
    t_print(SERVER_STOPPED);
    return 1;
}

static int usage(char *argv[])
{
    printf(USAGE_USAGE, argv[0]);
    printf(USAGE_PROGRAM_INTRO);
    printf(USAGE_DESCRIPTION);
    return 0;
}

int main(int argc, char *argv[])
{
    char *port_number = NULL;

    /* getopt silent mode set */
    opterr = 0;

    if(argc < 3) {
        usage(argv);
        return 0;
    }

    while (1) {
        char c;

        c = getopt (argc, argv, "p:");
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'p':
            port_number = optarg;
            break;
        }
    }

    if(port_number == NULL) {
        printf(ERROR_MISSING_PARAMS);
    }

    t_print(SERVER_STARTING);
    start_server(atoi(port_number));
    exit(0);
}
