#include "sensor_server.h"

/* Server data and stats */
struct server_data *s_data;

/* Shared synchro elements */
struct server_synchro *s_synch;

/* Used by sig handlers */
volatile sig_atomic_t done;

/* Pointer to shared memory containing the client list */
struct client_table_entry *client_list;

/* Pointer to shared memory containing config */
struct config *cfg;

struct client_table_entry* get_client_by_id(int id)
{
    struct client_table_entry* client_list_iterate;
    struct client_table_entry* temp;
    int found = 0;

    sem_wait(&(s_synch->client_list_mutex));
    list_for_each_entry_safe(client_list_iterate, temp, &client_list->list, list) {
        if(client_list_iterate->client_id == id) {
            found = 1;
            break;
        }
    }
    sem_post(&(s_synch->client_list_mutex));
    if(found){
        return client_list_iterate;
    }
    else{
        return NULL;
    }
}

/* Removes a client with the given PID */
static void remove_client_by_pid(pid_t pid)
{
    struct client_table_entry* client_list_iterate;
    struct client_table_entry* temp_remove;

    sem_wait(&(s_synch->client_list_mutex));
    list_for_each_entry_safe(client_list_iterate, temp_remove,&client_list->list, list) {
        if(client_list_iterate->pid == pid) {
            if(client_list_iterate->client_id > 0){
                s_data->number_of_sensors--;
            }
            list_del(&client_list_iterate->list);
        }
    }
    s_data->number_of_clients--;
    sem_post(&(s_synch->client_list_mutex));
}

/* Removes a client with the given ID */
void remove_client_by_id(int id)
{
    struct client_table_entry* client_list_iterate;
    struct client_table_entry* temp_remove;

    sem_wait(&(s_synch->client_list_mutex));
    list_for_each_entry_safe(client_list_iterate, temp_remove,&client_list->list, list) {
        if(client_list_iterate->client_id == id) {
            list_del(&client_list_iterate->list);
        }
    }
    s_data->number_of_clients--;
    sem_post(&(s_synch->client_list_mutex));
}

/* Creates an entry in the client list structure and returns a pointer to it*/
static struct client_table_entry* create_client(struct client_table_entry* ptr)
{
    sem_wait(&(s_synch->client_list_mutex));
    s_data->number_of_clients++;
    struct client_table_entry* tmp;
    tmp = (client_list + s_data->number_of_clients);
    list_add_tail( &(tmp->list), &(ptr->list) );
    sem_post(&(s_synch->client_list_mutex));

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
            t_print(PROCESS_REAPED, pid, status);
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
    done = 1;
}

/* Setting up the config structure specific for the server */
static void initialize_config(struct config_map_entry *conf_map, struct config *cfg, int entries){
    conf_map[0].entry_name = CONFIG_SERVER_MAX_CONNECTIONS;
    conf_map[0].modifier = FORMAT_INT;
    conf_map[0].destination = &cfg->max_clients;

    conf_map[1].entry_name = CONFIG_SERVER_WARM_UP;
    conf_map[1].modifier = FORMAT_INT;
    conf_map[1].destination = &cfg->warm_up_seconds;
}

/*
* Main loop for the server.
* Forks everytime a client connects and calls setup_session()
*/
static void start_server(int port_number)
{
    /* Initializing variables */
    int server_sockfd;
    struct sockaddr_in serv_addr;
    done = 0;

    /* Initializing config structure */
    cfg = mmap(NULL, sizeof(struct config), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    initialize_config(conf_map, cfg, CONFIG_ENTRIES);

    /* Loading config */
    int load_config_status = load_config(conf_map, CONFIG_FILE_PATH, CONFIG_ENTRIES);

    /* Falling back to default if load_config fails */
    if(load_config_status) {
        t_print(CONFIG_LOADED);
        client_list = mmap(NULL, (cfg->max_clients * sizeof(struct client_table_entry)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    } else {
        t_print(ERROR_CONFIG_LOAD_FAILED);
        exit(0);
    }

    INIT_LIST_HEAD(&client_list->list);

    /* Create and initialize shared memory for server data */
    s_data = mmap(NULL, sizeof(struct server_data), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    bcopy(PROGRAM_VERSION, s_data->version,4);
    s_data->pid = getpid();
    s_data->started = time(NULL);

    /* Init shared semaphores and sync elements */
    s_synch = mmap(NULL, sizeof(struct server_synchro), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(&(s_synch->ready_mutex), 1, 1);
    sem_init(&(s_synch->client_list_mutex), 1, 1);

    if( &(s_synch->ready_mutex) == SEM_FAILED || &(s_synch->client_list_mutex) == SEM_FAILED) {
        t_print(ERROR_SEMAPHORE_CREATION_FAILED);
        sem_close(&(s_synch->ready_mutex));
        sem_close(&(s_synch->client_list_mutex));
        exit(1);
    }

    /* Initialize socket */
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0) {
        die(62,ERROR_SOCKET_OPEN_FAILED);
    }

    /* Registering the SIGINT handler */
    struct sigaction sigint_action;
    memset(&sigint_action, 0, sizeof(struct sigaction));
    sigint_action.sa_handler = handle_sig;
    sigaction(SIGINT, &sigint_action, NULL);

    /* Registering the SIGTERM handler */
    struct sigaction sigterm_action;
    memset(&sigterm_action, 0, sizeof(struct sigaction));
    sigterm_action.sa_handler = handle_sig;
    sigaction(SIGTERM, &sigterm_action, NULL);

    /* Registering the SIGCHLD handler */
    struct sigaction child_action;
    child_action.sa_handler = &handle_sigchld;
    sigemptyset(&child_action.sa_mask);
    child_action.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &child_action, 0) == -1) {
        perror(0);
        exit(1);
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
    while (!done) {
        t_print(WAITING_FOR_CONNECTIONS);
        session_fd = accept(server_sockfd,0,0);
        if (session_fd==-1) {
            if (errno==EINTR) continue;
                t_print(ERROR_CONNECTION_ACCEPT,errno);
        }
        if(s_data->number_of_clients == cfg->max_clients) {
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
                if(new_client->marked_for_kick){
                    t_print(CLIENT_KICKED, getpid());
                }
                t_print(CLIENT_DISCONNECTED, getpid());
                _exit(0);
            } else {
                t_print(CON_ACCEPTED);
                close(session_fd);
            }
        }
    }
    /* Freeing and closing */
    munmap(client_list, sizeof(struct client_table_entry));
    munmap(s_data, sizeof(struct server_data));
    sem_close(&(s_synch->ready_mutex));
    sem_close(&(s_synch->client_list_mutex));
    munmap(s_synch, sizeof(struct server_synchro));
    close(server_sockfd);
    t_print(SERVER_STOPPED);
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