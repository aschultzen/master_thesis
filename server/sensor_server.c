#include "sensor_server.h"
#include "serial.h"

/* Server data and stats */
struct server_data *s_data;

/* Shared synchro elements */
struct server_synchro *s_synch;

/* Used by sig handlers */
volatile sig_atomic_t done;

/* Pointer to shared memory containing the client list */
struct client_table_entry *client_list;

/* Used by server to show connected clients. */
static void show_list() __attribute__ ((unused));
static void show_list() {
    if(! list_empty(&client_list->list) ) {
        struct client_table_entry* client_list_iterate;
        printf("\n");
        t_print("CONNECTED CLIENTS\n");
        t_print("========================================\n");
        list_for_each_entry(client_list_iterate, &client_list->list, list) {
            t_print("ID: %d, PID: %d, IP:%s TYPE %d\n",
            client_list_iterate->client_id, 
            client_list_iterate->pid, 
            client_list_iterate->ip, 
            client_list_iterate->client_type);
        }
        t_print("========================================\n\n");
    } else {
        t_print("No clients connected.\n");
    }
}

/* Removes a client with the given PID */
static void remove_client(pid_t pid)
{      
    struct client_table_entry* client_list_iterate;
    struct client_table_entry* temp_remove;

    sem_wait(&(s_synch->client_list_mutex));
    list_for_each_entry_safe(client_list_iterate, temp_remove,&client_list->list, list) {
        if(client_list_iterate->pid == pid) {
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
            remove_client(pid);
            t_print("Process %d reaped. Status: %d\n", pid, status);
        }
    }
}

/* SIGTERM/INT Handler */
static void handle_sig(int signum)
{
    if(signum == 15) {
        t_print("[%d] SIGTERM received!\n", getpid());
    }
    if(signum == 2) {
        t_print("[%d] SIGINT received!\n", getpid());
    }
    t_print("Stopping server...\n", getpid());
    done = 1;
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

    /* Loading config */
    struct config cfg;
    int load_config_status = load_config(&cfg, CONFIG_FILE_PATH);

    /* Falling back to default if load_config fails */
    if(load_config_status == 0){
        t_print("Config loaded!\n");
        client_list = mmap(NULL, (cfg.config_server_max_connections * sizeof(struct client_table_entry)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    }else{
        t_print("Failed to load config! Config file corrupt or missing entries.\n");
        client_list = mmap(NULL, (MAX_CLIENTS * sizeof(struct client_table_entry)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    }
    INIT_LIST_HEAD(&client_list->list);

    /* Create and initialize shared memory for server data */
    s_data = mmap(NULL, sizeof(struct server_data), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    s_data->max_clients = cfg.config_server_max_connections;
    bcopy(PROGRAM_VERSION, s_data->version,4);
    s_data->pid = getpid();
    s_data->started = time(NULL);

    /* Init shared semaphores and sync elements */
    s_synch = mmap(NULL, sizeof(struct server_synchro), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(&(s_synch->ready_mutex), 1, 1);
    sem_init(&(s_synch->client_list_mutex), 1, 1);

    if( &(s_synch->ready_mutex) == SEM_FAILED || &(s_synch->client_list_mutex) == SEM_FAILED)
    {
      t_print("Unable to create semaphores, exiting...\n");
      sem_unlink(CLIENT_LIST_SEM_NAME);
      sem_unlink(READY_SEM_NAME);
      exit(1);
    }

    /* Initialize socket */
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0) {
        die(62,"ERROR opening socket\n");
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
        t_print("%d: ERROR on binding\n", 80);
        exit(1);
    }

    /* Marking the connection for listening*/
    listen(server_sockfd,SOMAXCONN);

    int session_fd = 0;
    t_print("Server is running. Accepting connections.\n");
    while (!done) {
        t_print("Waiting for connections...\n");
        session_fd = accept(server_sockfd,0,0);
        if (session_fd==-1) {
            if (errno==EINTR) continue;
            die(90,"failed to accept connection (errno=%d)",errno);
        }
        if(s_data->number_of_clients == MAX_CLIENTS){
            write(session_fd, ERROR_MAX_CLIENTS_REACHED, sizeof(ERROR_MAX_CLIENTS_REACHED));
            close(session_fd);
        }else{
            struct client_table_entry *new_client = create_client(client_list);
            pid_t pid=fork();
            if (pid==-1) {
                die(94, "failed to create child process (errno=%d)",errno);
                //WHAT HAPPENS WITH THE LIST WHEN FORK FAILS? DEAL WITH IT.
            } else if (pid==0) {
                close(server_sockfd);
                setup_session(session_fd, new_client);
                close(session_fd);
                t_print("[%d] Disconnected\n", getpid());
                _exit(0);
            } else {
                t_print("Connection accepted\n");
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
    t_print("Server STOPPED!\n", getpid());
}

static int usage(char *argv[])
{
    char description[] = {"Required argument:\n\t -p <PORT NUMBER>\n\n"};
    printf("Usage: %s [ARGS]\n\n", argv[0]);
    printf("Sensor_server: Server part of GPS Jamming/Spoofing system\n\n");
    printf("%s\n", description);
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
        printf("Missing parameters!\n");
    }

    t_print("Sensor server starting...\n", getpid());
    start_server(atoi(port_number));
    exit(0);
}