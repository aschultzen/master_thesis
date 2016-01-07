#include "sensor_server.h"
#include "serial.h"

static struct client_table_entry client_list;

/* Declaration of shared memory variables */
static char *serial_display_connections;
static char *serial_display_message;

volatile sig_atomic_t done = 0;

void show_list()
{
    if(! list_empty(&client_list.list) ){
        struct client_table_entry* client_list_iterate;
        printf("\n");
        t_print("CONNECTED CLIENTS\n");
        t_print("========================================\n");
        list_for_each_entry(client_list_iterate, &client_list.list, list) {
            t_print("ID: %d, PID: %d, IP:%s\n", client_list_iterate->id, client_list_iterate->pid, client_list_iterate->ip);
        }
        t_print("========================================\n\n");
    }else{
        t_print("No clients connected yet..\n");
    }
}

void remove_from_list(pid_t pid)
{
    struct client_table_entry* client_list_iterate;
    struct client_table_entry* temp_remove;
    list_for_each_entry_safe(client_list_iterate, temp_remove,&client_list.list, list) {
        if(client_list_iterate->pid == pid){
            list_del(&client_list_iterate->list);
            free(client_list_iterate);
        }
    }
}

static void append_to_list(struct client_table_entry* ptr, pid_t i_pid, char *ip)           
{
  struct client_table_entry* tmp;
  tmp = (struct client_table_entry*)malloc(sizeof(struct client_table_entry));
  //tmp = mmap(NULL, 
  //          (struct client_table_entry*)malloc(sizeof(struct client_table_entry)), PROT_READ | PROT_WRITE,
  //          MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if(!tmp) {
    perror("malloc");
    exit(1);
  }
  tmp->id = 1;
  tmp->pid = i_pid;
  strncpy(tmp->ip, ip, INET_ADDRSTRLEN);
  list_add_tail( &(tmp->list), &(ptr->list) );
}

/* SIGCHLD Handler */
void handle_sigchld(int signum)
{
    pid_t pid;
    int   status;
    while ((pid = waitpid(-1, &status, WNOHANG)) != -1)
    {
        if(pid > 0){
            t_print("SIGCHLD HANDLER: Removing PROC %d\n", pid);
            remove_from_list(pid);
        }
    }
}

/* SIGTERM/INT Handler */
void handle_sig(int signum)
{
    if(signum == 15) {
        t_print("[%d] SIGTERM received!\n", getpid());
    }
    if(signum == 2) {
        t_print("[%d] SIGINT received!\n", getpid());
    }
    t_print("%d: Stopping server...\n", getpid());
    done = 1;
}

int respond(struct session_info *s_info)
{
    int read_status = s_read(s_info); /* Blocking */
    if(read_status == -1) {
        t_print("Read failed or interrupted!\n");
        if(s_info->client_id != -1) {
            serial_display_connections[s_info->client_id] = '0';
        }
        return -1;
    };

    int parse_status = parse_input(s_info);

    if(parse_status == -1) {
        s_write(s_info, ERROR_ILLEGAL_MESSAGE_SIZE, 
            sizeof(ERROR_ILLEGAL_MESSAGE_SIZE));
    }
    if(parse_status == 0) {
        s_write(s_info, ERROR_ILLEGAL_COMMAND, 
            sizeof(ERROR_ILLEGAL_COMMAND));
    }
    if(parse_status == 1) {
        if(s_info->cm.code == CODE_DISCONNECT) {
            t_print("Client %d requested DISCONNECT.\n", s_info->client_id);
            s_write(s_info, PROTOCOL_OK, sizeof(PROTOCOL_OK));
            serial_display_connections[s_info->client_id] = '0';
            return -1;
        }
        if(s_info->cm.code == CODE_IDENTIFY) {
            t_print("IDENTIFY received\n");
            int id = 0;

            if(sscanf(s_info->cm.parameter, "%d", &id) == -1) {;
                s_write(s_info, ERROR_ILLEGAL_COMMAND, sizeof(ERROR_ILLEGAL_COMMAND));
                return 0;
            }

            if(serial_display_connections[id] == '1') {
                s_info->client_id = -1;
                s_write(s_info, "ID in use!\n", 11);
                return 0;
            }

            s_info->client_id = id;
            t_print("ID set to: %d\n", s_info->client_id);
            serial_display_connections[s_info->client_id] = '1';
            s_write(s_info, PROTOCOL_OK, sizeof(PROTOCOL_OK));
            return 0;
        }

        if(s_info->client_id < 0) {
            s_write(s_info, ERROR_NO_ID, sizeof(ERROR_NO_ID));
            return 0;
        }
    }
    return 0;
}

/* The session_info struct allocated on stack only */
void setup_session(int session_fd)
{
    /* Initializing structure */
    struct session_info *s_info = malloc(sizeof(struct session_info));
    s_info->heartbeat_timeout.tv_sec = CLIENT_TIMEOUT + 1000; //remove 1000 when testing is done!
    s_info->heartbeat_timeout.tv_usec = 0;
    s_info->client_id = -1;
    s_info->session_fd = session_fd;
    s_info->iobuffer = calloc (SESSION_INFO_IO_BUFFER_SIZE, sizeof(void*));
    if(s_info->iobuffer == NULL) {
        die(21, "Memory allocation failed!");
    }

    /* Setting socket timeout to default value */
    /* This doesn't always work for some reason, race condition? :/ */
    if (setsockopt (s_info->session_fd, SOL_SOCKET, 
        SO_RCVTIMEO, (char *)&s_info->heartbeat_timeout, sizeof(struct timeval)) < 0) {
        die(36,"setsockopt failed\n");
    }

    /*
    * Entering child process main loop
    * Breaks (disconnects the client) if
    * respond < 0
    */

    while(!done) {
        if(respond(s_info) < 0) {
            break;
        }
    }
    /* Freeing resources */
    free(s_info->iobuffer);
    free(s_info);
}

/*
* Main loop for the server.
* Forks everytime a client connects
* and calls setup_session()
*/
void start_server(int port_number, char *serial_display_path)
{
    /* Initializing variables */
    //char *serial_display_path = "/dev/ttyACM0";
    int server_sockfd;
    struct sockaddr_in serv_addr;

    //struct client_table_entry client_list; 

    INIT_LIST_HEAD(&client_list.list);

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
        die(80,"ERROR on binding");
    }

    /* Marking the connection for listening*/
    listen(server_sockfd,SOMAXCONN);

    /* Forking out proc for serial com */
    if(serial_display_path != NULL) {
        /* Shared memory used for "IPC"*/
        serial_display_connections = mmap(NULL, 
            sizeof SERVER_MAX_CONNECTIONS*(sizeof(char)), PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        serial_display_message = mmap(NULL, 
            sizeof DISPLAY_SIZE*(sizeof(char)), PROT_READ | PROT_WRITE,
             MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        pid_t pid=fork();
        if (pid==-1) {
            die(94, "failed to create child process (errno=%d)",errno);
        } else if (pid==0) {
            t_print("Serial COM started (PID: %d)\n", getpid());
            open_serial(serial_display_path, serial_display_connections);
            t_print("Serial COM closed (PID: %d)\n", getpid());
            _exit(0);
        }

        int loop_counter;
        for(loop_counter = 0; loop_counter < DISPLAY_SIZE; loop_counter++) {
            serial_display_message[loop_counter] = '0';
        }
    } else {
        serial_display_connections = malloc(SERVER_MAX_CONNECTIONS*(sizeof(char)));
        t_print("No serial display defined.\n");
    }

    /* Initializing connection table */
    int loop_counter;
    for(loop_counter = 0; loop_counter < SERVER_MAX_CONNECTIONS; loop_counter++) {
        serial_display_connections[loop_counter] = '0';
    }

    int session_fd = 0;
    while (!done) {
        show_list();
        session_fd = accept(server_sockfd,0,0);
        if (session_fd==-1) {
            if (errno==EINTR) continue;
            die(90,"failed to accept connection (errno=%d)",errno);
        }
        pid_t pid=fork();
        if (pid==-1) {
            die(94, "failed to create child process (errno=%d)",errno);
        } else if (pid==0) {
            close(server_sockfd);
            setup_session(session_fd);
            t_print("PROC %d: Closing up session\n", getpid());
            close(session_fd);
            _exit(0);
        } else {
            t_print("%d: Connection accepted\n", getpid());
            char ip[INET_ADDRSTRLEN];
            get_ip_str(session_fd, ip);
            append_to_list(&client_list,pid,ip);
            close(session_fd);
        }
    }
    if(serial_display_path == NULL) {
        free(serial_display_connections);
    }
    close(server_sockfd);
    t_print("%d: Server STOPPED!\n", getpid());
}

int usage(char *argv[])
{
    char description[] = {"Required argument:\n\t -p <PORT NUMBER>\n\nOptional:\n\t -s <PATH TO SERIAL DISPLAY>\n"};
    printf("Usage: %s [ARGS]\n\n", argv[0]);
    printf("Sensor_server: Server part of GPS Jamming/Spoofing system\n\n");
    printf("%s\n", description);
    return 0;
}

int main(int argc, char *argv[])
{
    char *serial_display_path = NULL;
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

    if(argc == 5) {
        if(argv[3][1] == 's') {
            serial_display_path = argv[4];
        }
    }

    t_print("%d: Sensor server starting...\n");
    start_server(atoi(port_number), serial_display_path);
    exit(0);
}